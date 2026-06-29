# CetDevelopmentKit API 参考

## 一、版本信息

**当前版本**: V2.4.0

### 版本号

| 宏 | 值 | 说明 |
|----|-----|------|
| `VER_MAJOR` | 2 | 主版本号 |
| `VER_MINOR` | 4 | 次版本号 |
| `VER_MICRO` | 0 | 小版本号 |
| `IS_RELEASE_VERSION` | 1 | 是否为正式版（1=正式版，0=补丁版） |
| `APP_VERSION` | `"2.4.0"` | 应用版本字符串 |
| `TIP_VERSION` | `"(CXYQK5152.CDPK240.G631)"` | 版本提示字符串 |
| `PATCH_PACKET` | `"5251"` | 补丁包号 |

### 产品信息

| 宏 | 值 | 说明 |
|----|-----|------|
| `APP_NAME` | `"CetDevelopmentKit"` | 应用名称 |
| `PRODUCT_NAME` | `"CetDevelopmentKit240"` | 产品 ID（= APP_NAME + 主版本号 + 次版本号 + "0"） |
| `COMPANY_NAME` | `"CetXiyuan"` | 公司名称 |
| `PRODUCT_ICON` | `"favorite.ico"` | 应用图标文件 |
| `ORGANIZATION_DOMAIN` | `"https://www.cetxiyuan.com/"` | 公司域名 |

### 版本工具宏

| 宏 | 说明 |
|----|------|
| `VERSION_CHECK(major, minor, micro)` | 将版本号编码为整数：`(major<<16)|(minor<<8)|(micro)` |
| `VERSION_STRING(major, minor, micro)` | 将版本号编码为字符串：`"major.minor.micro"` |
| `DIRECT_STRING(major, minor, micro)` | 将版本号直接拼接：`"majorminormicro"` |

### 文件版本信息

| 宏 | 说明 |
|----|------|
| `FILE_VERSION` | Windows 文件版本（`2,4,0`） |
| `FILE_VERSION_STR` | 文件版本字符串 |
| `FILE_DESCRIPTION` | 文件描述（"CetDevelopmentKit based on Qt 5.15.2 (MinGW, 32 bit)"） |
| `LEGAL_COPYRIGHT` | 版权信息 |
| `ORIGINAL_NAME` | 原始文件名（"CetDevelopmentKit.exe"） |

### 版本编码说明

以 `RELEASE_DATE` 宏 `(CXYQK5152.CDPK240.G631)` 为例：

| 段 | 含义 |
|---|---|
| `CXYQK5152` | CetXiyuan Qt Kernel 5.15.2（Qt 版本） |
| `CDPK240` | CetDevelopmentKit 第 240 次构建 |
| `G631` | G=2026年（A=2020年起），6=6月，31编码为 31−6=25 即 25日 |

---

## 二、核心类

### 2.1 MainWindow

**头文件**: `mainwindow/mainwindow.h`

**用途**: 应用程序主窗口，负责插件加载、功能模块管理、进程生命周期控制和许可证激活调度。

**继承关系**: `QMainWindow`

#### 公共枚举

```cpp
enum ConfigType {
    CONFIG_EC200U_DPATH = 0,   // EC200U 差分包制作
    CONFIG_ADB_ZIPEXPORT,      // ADB 设备数据打包导出
};
Q_ENUM(ConfigType)             // 注册到 Qt 元对象系统，支持 currentData().value<ConfigType>()
```

#### 构造函数

```cpp
MainWindow(QWidget *parent = nullptr);
```

初始化顺序：
1. 加载 `CetLogManagerPlugin.dll` 插件
2. 创建 `QSettings("Configs/AppMaster.ini", QSettings::IniFormat)`
3. 创建 `QProcess`
4. 设置 UTF-8 编码
5. 初始化 UI，创建 `EC200UConfigure` 和 `ADBConfigure` 子模块
6. 连接所有信号/槽
7. 调用 `initFeaturesPlugin()` 加载其余插件并启动激活时序

#### 析构函数

```cpp
~MainWindow();
```

清理 `m_cmdProcess` 和 UI 资源。

#### 私有方法

```cpp
QObject *loadPlugin(const QString &dllName);
```

从插件目录加载指定 DLL 插件。

**参数**:
- `dllName` — 插件 DLL 文件名（如 `"CetLicensePlugin.dll"`）

**返回值**: 插件实例的 `QObject*` 指针，加载失败返回 `nullptr`。

**加载路径优先级**:
1. `D:/Program Files (x86)/CetXiyuan/CetToolDLLs/plugins/`
2. `./plugins/`（相对于可执行文件目录）

#### 私有方法

```cpp
void initFeaturesPlugin();
```

初始化所有功能插件，包括：
- 配置日志管理菜单项
- 加载许可证插件并设置激活时序（30 秒首次 + 指数退避）
- 加载更新检测插件并检查更新
- 加载进度显示插件

```cpp
void doCurrentIndexChanged(int index);
```

**槽函数**，响应 `QComboBox::currentIndexChanged`。根据选中的功能模块切换显示对应的配置面板，并保存选择到 `QSettings`。

#### 私有槽

| 槽 | 签名 | 说明 |
|----|------|------|
| `onProcessFinished` | `(int exitCode, QProcess::ExitStatus exitStatus)` | 进程结束处理：正常退出置进度条 100%，异常退出记录日志 |
| `on_processHandleButton_clicked` | `()` | 执行按钮：关闭当前进程 → 获取命令 → 启动新进程 |
| `on_processConfigureButton_clicked` | `()` | 配置按钮：折叠/展开配置面板 |

#### 关键 Lambda 连接

**进程标准输出处理**（在构造函数中连接）:

```cpp
connect(m_cmdProcess, &QProcess::readyReadStandardOutput, this, [=]() {
    QByteArray data = m_cmdProcess->readAllStandardOutput();
    ui->plainTextEdit->appendPlainText(data);
    ConfigType type = ui->processComboBox->currentData().value<ConfigType>();
    switch (type) {
    case CONFIG_EC200U_DPATH: m_ec200uConfigure->recvHandle(data); break;
    case CONFIG_ADB_ZIPEXPORT: m_adbConfigure->recvHandle(data); break;
    }
});
```

**进程标准错误处理**（在构造函数中连接）:

```cpp
connect(m_cmdProcess, &QProcess::readyReadStandardError, this, [=]() {
    QByteArray data = m_cmdProcess->readAllStandardError();
    ui->plainTextEdit->appendPlainText(data);
});
```

**激活定时器**（在 `initFeaturesPlugin()` 中设置）:

```cpp
// 首次激活：30 秒后
QTimer::singleShot(30 * 1000, this, [=]() {
    emit activeAction->triggered(true);
});

// 重试激活：指数退避
QTimer::singleShot(auto_timing_msec, this, [=]() {
    auto_timing_active = true;
    auto_timing_msec = auto_timing_msec * 2;  // 30min → 60min → 120min ...
    emit activeAction->triggered(true);
});
```

#### 关键成员变量

| 变量 | 类型 | 说明 |
|------|------|------|
| `ui` | `Ui::MainWindow*` | UI 自动生成类 |
| `m_cetLogManagerInterface` | `CetLogManagerInterface*` | 日志管理插件 |
| `m_settings` | `QSettings* const` | 全局配置（INI 文件） |
| `m_cmdProcess` | `QProcess* const` | 共享进程实例 |
| `m_ec200uConfigure` | `EC200UConfigure*` | EC200U 功能模块 |
| `m_adbConfigure` | `ADBConfigure*` | ADB 功能模块 |
| `m_cetLicenseInterface` | `CetLicenseInterface*` | 许可证插件 |
| `m_cetUpdateInterface` | `CetUpdateInterface*` | 更新检测插件 |
| `m_cetProgressInterface` | `CetProgressInterface*` | 进度显示插件 |

---

### 2.2 EC200UConfigure

**头文件**: `modules/ec200u/ec200uconfigure.h`

**用途**: 配置 EC200U 模块的 OTA 差分升级包制作参数，验证路径有效性，生成 `fotacreate2` 命令。

**继承关系**: `QWidget`

#### 构造函数

```cpp
explicit EC200UConfigure(QProcess *process,
    QPlainTextEdit *plainTextEdit, QWidget *parent = nullptr);
```

**参数**:
- `process` — 共享的 QProcess 实例（仅持有引用，不负责生命周期）
- `plainTextEdit` — 日志输出控件
- `parent` — 父控件

初始化时从 `Configs/EC200UConfigure.ini` 恢复上次配置。

#### 公共方法

```cpp
QString startCommand();
```

验证并构造命令行。

**验证内容**:
- `toolPath` 路径是否存在
- `oldPacket` 路径是否存在
- `newPacket` 路径是否存在
- `outputName` 为空时自动填充 `"output.pac"`

**返回值**: 构造的命令行字符串，格式为：
```
<toolPath> fotacreate2 --pac <oldPacket>,<newPacket>,<settingDir>\setting\fota8910.xml <outputDir>\<outputName> -d v
```
验证失败返回空字符串并弹出警告。

```cpp
void recvHandle(const QByteArray &data);
```

接收进程输出数据。当前实现直接发送 `progress(100)` 信号。

**参数**:
- `data` — 进程标准输出数据（当前未使用）

#### 信号

| 信号 | 签名 | 触发时机 |
|------|------|---------|
| `progress` | `void progress(int value)` | `recvHandle()` 被调用时，发送值 100 |

#### 私有槽

| 槽 | 触发时机 |
|----|---------|
| `on_toolPathButton_clicked()` | 点击工具路径浏览按钮 → 打开文件对话框 → 保存到 QSettings |
| `on_oldPacketButton_clicked()` | 点击原固件包浏览按钮 → 打开 `.pac` 文件对话框 → 保存到 QSettings |
| `on_newPacketButton_clicked()` | 点击新固件包浏览按钮 → 打开 `.pac` 文件对话框 → 保存到 QSettings |
| `on_outputDirButton_clicked()` | 点击输出目录浏览按钮 → 打开目录对话框 → 保存到 QSettings |

#### 关键成员变量

| 变量 | 类型 | 说明 |
|------|------|------|
| `ui` | `Ui::EC200UConfigure*` | UI 自动生成类 |
| `m_settings` | `QSettings* const` | 配置持久化（`Configs/EC200UConfigure.ini`） |
| `m_process` | `QProcess* const` | 进程引用（由 MainWindow 管理） |
| `m_plainTextEdit` | `QPlainTextEdit* const` | 日志输出控件引用 |

---

### 2.3 ADBConfigure

**头文件**: `modules/adb/adbconfigure.h`

**用途**: 通过 ADB Shell 交互式会话实现设备数据打包并拉取到本地，使用状态机驱动 Shell 交互。

**继承关系**: `QWidget`

#### 构造函数

```cpp
explicit ADBConfigure(QProcess *process,
    QPlainTextEdit *plainTextEdit, QWidget *parent = nullptr);
```

**参数**:
- `process` — 共享的 QProcess 实例
- `plainTextEdit` — 日志输出控件
- `parent` — 父控件

初始化时从 `Configs/ADBConfigure.ini` 恢复上次配置。

#### 公共方法

```cpp
QString startCommand();
```

验证 ADB 工具路径有效性，初始化 `m_step = 0`，返回 `"<adbPath>" shell` 命令。

**返回值**: ADB shell 命令字符串（路径带引号）。验证失败返回空字符串。

```cpp
void recvHandle(const QByteArray &data);
```

状态机驱动的 Shell 交互处理。

**状态机流程**:

| 条件 | 动作 | m_step | progress |
|------|------|--------|----------|
| 数据包含 `"ogin"` | 发送用户名 | — | — |
| 数据包含 `"assword"` | 发送密码 | — | 20 |
| 数据包含 `"#"` 且 step=0 | 发送 `rm /tmp/cetmp*.tar.gz` | 1 | 45 |
| 数据包含 `"#"` 且 step=1 | 发送 `tar -zcf /tmp/cetmp_xxx.tar.gz path/*` | 2 | 65 |
| 数据包含 `"#"` 且 step=2 | 关闭 Shell，1 秒后执行 `adb pull` | -1 | 85 |

adb pull 命令格式：
```
<adbPath> pull /tmp/cetmp_<timestamp>.tar.gz <outputPath>
```

#### 信号

| 信号 | 签名 | 触发时机 |
|------|------|---------|
| `progress` | `void progress(int value)` | 各步骤完成时: 20（登录完成）、45（清理完成）、65（打包完成）、85/100（拉取完成） |

#### 私有槽

| 槽 | 触发时机 |
|----|---------|
| `on_exeToolButton_clicked()` | 浏览 ADB 工具路径 → 保存到 QSettings（路径自动加引号） |
| `on_usernameLineEdit_textChanged(const QString &arg1)` | 用户名输入框文本变化 → 实时保存到 QSettings |
| `on_passwordLineEdit_textChanged(const QString &arg1)` | 密码输入框文本变化 → 实时保存到 QSettings |
| `on_devicePackPathLineEdit_textChanged(const QString &arg1)` | 设备打包路径输入框文本变化 → 实时保存到 QSettings |
| `on_outputToolButton_clicked()` | 浏览输出目录 → 保存到 QSettings |

#### 关键成员变量

| 变量 | 类型 | 说明 |
|------|------|------|
| `ui` | `Ui::ADBConfigure*` | UI 自动生成类 |
| `m_settings` | `QSettings* const` | 配置持久化（`Configs/ADBConfigure.ini`） |
| `m_process` | `QProcess* const` | 进程引用（由 MainWindow 管理） |
| `m_plainTextEdit` | `QPlainTextEdit* const` | 日志输出控件引用 |
| `m_step` | `int` | 状态机步骤号（-1 = 流程结束） |

---

## 三、插件接口

### 3.1 CetLicenseInterface

**头文件**: `interfaces/cetlicenseinterface.h`

**接口标识**: `"org.qter.plugin.CetLicenseInterface"`

**用途**: 定义产品授权激活的抽象契约。

#### 枚举

```cpp
enum ActivateResult {
    ACTIVATE_CANCEL = 0,   // 激活取消
    ACTIVATE_OK,           // 激活成功
    ACTIVATE_ER            // 激活失败
};
```

#### 纯虚方法

| 方法 | 签名 | 说明 |
|------|------|------|
| `initialize` | `virtual void initialize(const QString &productId) = 0` | 通过产品 ID 初始化激活环境 |
| `activate` | `virtual int activate() = 0` | 非窗口式激活（后台自动），返回 `ActivateResult` |
| `activateWindow` | `virtual int activateWindow() = 0` | 窗口式激活（用户手动），返回 `ActivateResult` |
| `resultString` | `virtual QString resultString() = 0` | 获取激活结果描述文本 |
| `solidKey` | `virtual QString solidKey() = 0` | 获取产品固钥 |

---

### 3.2 CetUpdateInterface

**头文件**: `interfaces/cetupdateinterface.h`

**接口标识**: `"org.qter.plugin.CetUpdateInterface"`

**用途**: 定义在线更新检测的抽象契约。

#### 回调类型

```cpp
typedef void (*callback_t)(void);
```

#### 纯虚方法

| 方法 | 签名 | 说明 |
|------|------|------|
| `checkUpdate` | `virtual void checkUpdate(const QString &appName, const QString &appVersion, callback_t callback = nullptr) = 0` | 检测服务器是否存在新版本 |
| `inUpdating` | `virtual bool inUpdating(QString &newVersion) = 0` | 查询是否正在更新中，返回新版本号 |

**参数**:
- `appName` — 应用名称（如 `"CetDevelopmentKit"`）
- `appVersion` — 当前版本号（如 `"2.4.0"`）
- `callback` — 可选回调函数，更新完成后调用（如 `QCoreApplication::quit`）
- `newVersion` — 输出参数，新版本号

---

### 3.3 CetLogManagerInterface

**头文件**: `interfaces/cetlogmanagerinterface.h`

**接口标识**: `"org.qter.plugin.CetLogManagerInterface"`

**用途**: 定义日志管理窗口的抽象契约。

#### 纯虚方法

| 方法 | 签名 | 说明 |
|------|------|------|
| `show` | `virtual void show() = 0` | 显示日志管理控制窗口 |

---

### 3.4 CetProgressInterface

**头文件**: `interfaces/cetprogressinterface.h`

**接口标识**: `"org.qter.plugin.CetProgressInterface"`

**用途**: 定义进度显示插件的抽象契约（当前为空接口，预留扩展）。

---

## 四、配置文件格式

### Configs/AppMaster.ini

```ini
[CETDKITConfig]
processIndex=<int>     ; 当前选中的功能模块索引（0=EC200U, 1=ADB）
```

### Configs/EC200UConfigure.ini

```ini
[EC200UConfig]
toolPath=<string>      ; fotacreate2 工具路径
oldPacket=<string>     ; 原固件包路径
newPacket=<string>     ; 新固件包路径
outputDir=<string>     ; 输出目录
```

### Configs/ADBConfigure.ini

```ini
[ADBConfig]
toolPath=<string>      ; ADB 可执行文件路径
username=<string>      ; 设备登录用户名
password=<string>      ; 设备登录密码
packPath=<string>      ; 设备中待打包路径
outputDir=<string>     ; 本地输出目录
```

---

## 五、信号/槽连接总览

### MainWindow 构造函数中建立的连接

| 发送者 | 信号 | 接收者 | 槽/处理 |
|--------|------|--------|---------|
| `m_cmdProcess` | `finished(int, QProcess::ExitStatus)` | `MainWindow` | `onProcessFinished` |
| `m_cmdProcess` | `readyReadStandardOutput` | `MainWindow` | lambda（显示 + 路由到模块） |
| `m_cmdProcess` | `readyReadStandardError` | `MainWindow` | lambda（显示到日志区） |
| `m_ec200uConfigure` | `progress(int)` | `ui->progressBar` | `setValue` |
| `m_adbConfigure` | `progress(int)` | `ui->progressBar` | `setValue` |
| `ui->processComboBox` | `currentIndexChanged(int)` | `MainWindow` | `doCurrentIndexChanged` |
| `ui->processHandleButton` | `clicked()` | `MainWindow` | `on_processHandleButton_clicked` |
| `ui->processConfigureButton` | `clicked()` | `MainWindow` | `on_processConfigureButton_clicked` |

---

## 六、内部宏定义

### mainwindow.cpp 中的加载宏

```cpp
#define INIT_INTERFACE(mInterface, name)    \
    mInterface(qobject_cast<typeof(mInterface)>(loadPlugin(name)))

#define LOAD_INTERFACE(mInterface, name, isRemind)    \
    mInterface = qobject_cast<typeof(mInterface)>(loadPlugin(name));    \
    if (!mInterface && isRemind)    \
        QMessageBox::warning(this, tr("警告"), tr("缺少 (%1) 库\t").arg(name))
```

**用途**:
- `INIT_INTERFACE` — 在构造函数初始化列表中使用，加载插件并赋值给成员变量
- `LOAD_INTERFACE` — 在函数体内使用，加载插件并赋值，`isRemind=true` 时缺失会弹窗警告

> **注意**：宏中的 `typeof` 是 GCC 扩展（非 ISO C++），MinGW 编译器支持。若移植到 MSVC 需替换为 `decltype`。

### 模块配置前缀宏

```cpp
#define CETDKIT_SETTINGS_PREFIX      "CETDKITConfig"
#define EC200U_SETTINGS_PREFIX      "EC200UConfig"
#define ADB_SETTINGS_PREFIX         "ADBConfig"
```

**用途**: 统一管理各模块的 QSettings 键组名称。`CETDKIT_SETTINGS_PREFIX` 在 `mainwindow.cpp` 中定义，其余在各自模块的 `.cpp` 文件中定义。