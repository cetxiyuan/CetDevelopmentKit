# CetDevelopmentKit 维护者指南

## 一、代码风格规范

### 命名规则

| 元素 | 规则 | 示例 |
|------|------|------|
| 类名 | 大驼峰 (PascalCase) | `MainWindow`, `EC200UConfigure`, `ADBConfigure` |
| 接口类 | `Cet` 前缀 + 大驼峰 | `CetLicenseInterface`, `CetUpdateInterface` |
| 成员变量 | `m_` 前缀 + 小驼峰 | `m_cmdProcess`, `m_settings`, `m_toolPluginCtx` |
| 局部变量 | 小驼峰 | `toolPath`, `oldPacket`, `exitCode` |
| 宏定义 | 全大写 + 下划线 | `VER_MAJOR`, `PRODUCT_NAME`, `CETDKIT_SETTINGS_PREFIX` |
| 文件名 | 全小写 + 下划线分隔 | `mainwindow.cpp`, `ec200uconfigure.h`, `cettoolplugincontext.h` |
| UI 文件 | 与类名一致 | `mainwindow.ui`, `ec200uconfigure.ui` |
| 信号/槽 | Qt 自动命名规则 | `on_processHandleButton_clicked()` |
| 枚举值 | 全大写 + 下划线 | `CONFIG_EC200U_DPATH`, `ACTIVATE_OK` |

### 缩进与格式

- 使用 **4 个空格** 缩进（不使用 Tab）
- 左大括号不换行（K&R 风格）
- 条件/循环建议使用大括号；现有代码中单行 `if` 也存在省略大括号的写法（如 `if (!toolPath.isEmpty()) ui->toolPathLineEdit->setText(toolPath);`）

```cpp
// 正确
if (condition) {
    doSomething();
} else {
    doOther();
}

// 正确（接口纯虚方法）
virtual void initialize(const QString &productId) = 0;
```

### 注释风格

- 使用 `/* */` 块注释或 `//` 行注释（项目混用，新增代码建议使用 `//`）
- 关键逻辑、复杂状态机、定时器意图需要注释
- 宏定义必须有注释说明用途

```cpp
/* 是否为正式版 */
#define IS_RELEASE_VERSION      ( 1 )

// 工具包种类
ui->processComboBox->addItem("EC200U 差分包制作", CONFIG_EC200U_DPATH);
```

### 头文件组织

```cpp
#ifndef HEADER_NAME_H
#define HEADER_NAME_H

// 1. Qt 头文件
#include <QWidget>
#include <QSettings>

// 2. 项目内头文件
#include "ec200uconfigure.h"

// 3. 前向声明
namespace Ui { class MyClass; }
class QProcess;

// 4. 类定义
class MyClass : public QWidget
{
    Q_OBJECT
    // ...
};

#endif // HEADER_NAME_H
```

---

## 二、版本控制工作流

### 分支策略

- **master**: 主分支，保持稳定可发布状态
- **feature/xxx**: 功能开发分支，从 master 拉出，完成后合并回 master
- **hotfix/xxx**: 紧急修复分支

### Commit 消息格式

采用简洁的中文描述，格式：

```
<类型>: <简要描述>

类型包括：
- 发布正式版 Vx.y.z     (版本发布)
- 发布补丁包 Vx.y.z.nnnn (补丁包发布)
- Fixed: <描述>         (Bug 修复)
- Updated: <描述>        (更新/改进)
- Added: <描述>          (新增功能)
- Modified: <描述>       (修改调整)
```

示例：

```
发布正式版 V2.5.0(采样许可3.0)
Updated: 支持临时授权、停用/拒绝状态，拒绝后停止重试
Fixed: 修复许可周期超过 24小时 从 10min 开始重置
Modified: 许可证激活定时器重构为可复用 QTimer
```

### 版本号更新

发布前必须同步更新以下文件中的版本号：

1. **`CetDevelopmentKit/version.h`**: `VER_MAJOR`, `VER_MINOR`, `VER_MICRO`, `RELEASE_DATE`, `IS_RELEASE_VERSION`
2. **`CetDevelopmentKit-Setup/CetDevelopmentKit.iss`**: `MyAppVersion`
3. **`CHANGELOG.md`**: 添加新版本的变更记录

### 提交前检查清单

- [ ] 编译通过（Release 模式无警告）
- [ ] 版本号已同步更新
- [ ] CHANGELOG.md 已更新
- [ ] 不提交二进制文件（.exe、.dll）到版本库
- [ ] 不提交构建产物（build/ 目录应在 .gitignore 中）

---

## 三、测试方法

### 手动测试

#### EC200U 差分包制作测试

1. 准备测试数据：
   - 旧版固件包（`.pac` 文件）
   - 新版固件包（`.pac` 文件）
   - fotacreate2 工具（含 `setting/fota8910.xml` 配置文件）
2. 启动 CetDevelopmentKit
3. 选择 "EC200U 差分包制作"
4. 配置工具路径、原固件包、新固件包、输出目录
5. 点击 "执行"
6. 验证输出目录中生成的 `.pac` 差分包

#### ADB 数据打包导出测试

1. 准备 Android 设备（已开启 USB 调试）
2. 连接设备到电脑，确认 `adb devices` 可识别
3. 启动 CetDevelopmentKit
4. 选择 "ADB 设备数据打包导出"
5. 配置 ADB 路径、设备用户名、密码、打包路径、输出目录
6. 点击 "执行"
7. 验证输出目录中生成的 `.tar.gz` 文件

### 单元测试

当前项目未包含自动化单元测试。如需添加：

1. 在 `.pro` 文件中添加 `QT += testlib`
2. 创建 `tests/` 目录
3. 使用 `QTest` 框架编写测试用例

```cpp
#include <QTest>
#include "ec200uconfigure.h"

class TestEC200UConfigure : public QObject
{
    Q_OBJECT
private slots:
    void testStartCommand_data();
    void testStartCommand();
};
```

### 回归测试

发布前建议验证以下场景：

| 场景 | 预期结果 |
|------|---------|
| 首次启动（无配置文件） | 正常启动，30 秒后触发激活 |
| 已激活状态启动 | 正常进入主界面，中央控件可用 |
| 未激活状态启动 | 中央控件禁用，显示到期提示 |
| 切换功能模块 | 配置面板切换，日志区清空 |
| 配置文件持久化 | 关闭重开后配置恢复 |
| DLL 完整性校验 | CetProductWizard.dll 及各 Cet 插件 DLL 哈希匹配，校验通过 |

---

## 四、调试技巧

### qDebug() 输出

项目已启用 `QT_MESSAGELOGCONTEXT` 宏，qDebug() 输出包含文件名、行号和函数名。

```cpp
qDebug() << "command" << command;
// 输出: [mainwindow.cpp:239] command "..."
// （具体格式取决于 Qt 版本和平台）
```

### 关键日志输出点

| 位置 | 日志内容 | 用途 |
|------|---------|------|
| `mainwindow.cpp` | `on_processComboBox_currentIndexChanged type <type>` | 模块切换跟踪 |
| `mainwindow.cpp` | `processIndex <index>` | 模块索引保存跟踪 |
| `mainwindow.cpp` | `exitCode <code> exitStatus <status>` | 进程退出状态 |
| `mainwindow.cpp` | `command <command>` | 执行的命令 |
| `adbconfigure.cpp` | `fileName <name>` | ADB 工具路径选择 |

### 调试流程

1. 在 Qt Creator 中设置断点（F9）
2. 按 F5 启动调试
3. 查看变量值：右键 → "Add Expression Evaluator"
4. 查看调用堆栈：窗口 → "视图" → "堆栈"

### 常用调试场景

**场景 1: 验证命令字符串**

在 `on_processHandleButton_clicked()` 中设置断点，检查 `command`、`program`、`arguments` 变量。

**场景 2: 跟踪 ADB 状态机**

在 `ADBConfigure::recvHandle()` 的每个 `if` 分支设置断点，观察 `data` 内容和 `m_step` 变化。

**场景 3: 验证插件加载**

在 `CetToolPluginContext::loadPlugin()` 中设置断点，检查 `pluginsDir` 路径和 `plugin` 返回值。

### 插件调试技巧

**确认插件是否被找到：**

```cpp
QDir pluginsDir("./plugins");
qDebug() << pluginsDir.entryList(QDir::Files); // 列出目录中所有文件
```

**确认插件加载失败原因：**

```cpp
QPluginLoader loader(pluginsDir.absoluteFilePath("CetMyPlugin.dll"));
if (!loader.instance()) {
    qWarning() << loader.errorString();
}
```

常见失败原因：
- Qt 版本或编译器不匹配（错误信息含 `Cannot load library`）
- IID 不匹配（`instance()` 返回 nullptr 但无错误信息）
- 插件依赖的 DLL 缺失（可用 Dependency Walker 检查）

### 虚拟串口工具（用于 ADB 调试）

- [com0com](https://com0com.sourceforge.net/)：虚拟串口对
- 使用 TCP/IP 转串口工具模拟 ADB 连接

### 日志输出位置

- 应用日志通过 `qDebug()` 输出到 Qt Creator 的 "应用程序输出" 面板
- 日志插件 (`CetLogManagerPlugin.dll`) 可提供独立的日志管理窗口
- 进程输出直接显示在主界面的 `QPlainTextEdit` 中

---

## 五、发布流程

### 1. 更新版本号

编辑 `CetDevelopmentKit/version.h`:

```cpp
#define VER_MAJOR               2
#define VER_MINOR               6
#define VER_MICRO               0
#define RELEASE_DATE            "(CXYQK5152.CDPK260.G832)"
#define IS_RELEASE_VERSION      ( 1 )
```

### 2. 更新安装脚本

编辑 `CetDevelopmentKit-Setup/CetDevelopmentKit.iss`:

```iss
#define MyAppVersion "2.6.0"
```

### 3. 更新 CHANGELOG.md

```markdown
## [2.6.0] - 2026-08-24

### 新增
- xxx

### 修改
- xxx
```

### 4. 编译 Release 版本

在 Qt Creator 中选择 Release 模式，构建项目。

### 5. 准备 Release 目录

```shell
# 复制编译产物到 CetDevelopmentKit-Release/
cp build/.../release/CetDevelopmentKit.exe CetDevelopmentKit-Release/

# 使用 windeployqt 收集 Qt 依赖
cd CetDevelopmentKit-Release
windeployqt --release CetDevelopmentKit.exe

# 确保插件目录存在
mkdir plugins
# 复制最新的插件 DLL 到 plugins/
```

### 6. 编译安装包

在 Inno Setup 中打开 `CetDevelopmentKit.iss`，点击 Build → Compile。

### 7. 提交并打标签

```shell
git add CetDevelopmentKit/version.h CetDevelopmentKit-Setup/CetDevelopmentKit.iss CHANGELOG.md
git commit -m "发布正式版 V2.6.0"
git tag -a v2.6.0 -m "Release V2.6.0"
git push origin master --tags
```

### 8. 验证安装包

- 在干净的测试环境安装
- 验证所有功能正常
- 验证插件加载和完整性校验正常

---

## 六、扩展新功能模块

### 步骤概览

1. 在 `modules/` 下新建子目录，创建继承 `QWidget` 的配置类和 `.ui` 文件
2. 实现 `startCommand()` 返回完整命令行，实现 `recvHandle()` 解析输出并 `emit progress()`
3. 创建 `.pri` 文件，并在 `modules/modules.pri` 中 `include` 引入
4. 在 `MainWindow::ConfigType` 枚举中添加新类型
5. 在 `MainWindow` 构造函数中实例化并加入 `configureGroupBox`，在 `processComboBox` 中添加条目
6. 在 `doCurrentIndexChanged()` 和 `on_processHandleButton_clicked()` 的 `switch` 中处理新类型

### 详细步骤

#### 步骤 1：创建模块目录和文件

```
modules/yourmodule/
├── yourmodule.pri
├── yourmoduleconfigure.h
├── yourmoduleconfigure.cpp
└── yourmoduleconfigure.ui
```

#### 步骤 2：实现 QWidget 子类

```cpp
// yourmoduleconfigure.h
class YourModuleConfigure : public QWidget
{
    Q_OBJECT
public:
    explicit YourModuleConfigure(QProcess *process,
        QPlainTextEdit *plainTextEdit, QWidget *parent = nullptr);
    QString startCommand();
    void recvHandle(const QByteArray &data);
signals:
    void progress(int value);
    // ...
};
```

#### 步骤 3：创建 .pri 文件

```qmake
# yourmodule.pri
HEADERS += \
    $$PWD/yourmoduleconfigure.h

SOURCES += \
    $$PWD/yourmoduleconfigure.cpp

FORMS += \
    $$PWD/yourmoduleconfigure.ui
```

#### 步骤 4：注册到构建系统

在 `modules/modules.pri` 中添加：

```qmake
include ($$PWD/yourmodule/yourmodule.pri)
```

#### 步骤 5：注册到 MainWindow

在 `mainwindow.h` 的 `ConfigType` 枚举中添加新类型：

```cpp
enum ConfigType {
    CONFIG_EC200U_DPATH = 0,
    CONFIG_ADB_ZIPEXPORT,
    CONFIG_YOUR_MODULE,     // 新增
};
```

在 `mainwindow.cpp` 的构造函数中初始化，在 `doCurrentIndexChanged()` 和 `on_processHandleButton_clicked()` 的 switch 中添加对应分支。

---

## 七、插件开发指南

### 7.1 插件系统概述

CetDevelopmentKit 使用 Qt 插件机制（`QPluginLoader`）在运行时动态加载功能插件。插件管理由 `CetToolPluginContext` 统一封装，其源码位于 `CetToolDLLs` 仓库（通过 `INCLUDEPATH += $$PWD/../../CetToolDLLs` 引入）。

**插件加载路径（按优先级）：**

```
1. D:/Program Files (x86)/CetXiyuan/CetToolDLLs/plugins/   ← 开发环境（共享多个工具的插件）
2. ./plugins/                                                 ← 发布包随附目录
```

### 7.2 现有接口清单

| 接口头文件 | IID | 对应插件 DLL |
|---|---|---|
| `interfaces/cetlicenseinterface.h` | `org.qter.plugin.CetLicenseInterface` | `CetLicensePlugin.dll` |
| `interfaces/cetupdateinterface.h` | `org.qter.plugin.CetUpdateInterface` | `CetUpdatePlugin.dll` |
| `interfaces/cetlogmanagerinterface.h` | `org.qter.plugin.CetLogManagerInterface` | `CetLogManagerPlugin.dll` |
| `interfaces/cetprogressinterface.h` | `org.qter.plugin.CetProgressInterface` | `CetProgressPlugin.dll` |

### 7.3 开发新插件

#### 步骤 1：在主工程中定义接口

在 `CetDevelopmentKit/interfaces/` 下新建接口头文件，例如 `cetmyplugininterface.h`：

```cpp
#ifndef CETMYPLUGININTERFACE_H
#define CETMYPLUGININTERFACE_H

#include <QObject>

class CetMyPluginInterface
{
public:
    virtual ~CetMyPluginInterface() { }

    virtual void doSomething() = 0;   // 根据实际需求定义纯虚方法
};

// IID 格式统一为 org.qter.plugin.<接口类名>
Q_DECLARE_INTERFACE(CetMyPluginInterface, "org.qter.plugin.CetMyPluginInterface")

#endif // CETMYPLUGININTERFACE_H
```

将新文件加入 `interfaces/interfaces.pri`：

```qmake
HEADERS += \
    $$PWD/cetmyplugininterface.h
    # ... 已有接口 ...
```

#### 步骤 2：创建插件工程

在 `CetToolDLLs/CetToolPlugins/` 下新建插件子目录。`.pro` 文件关键配置：

```qmake
QT       += core
TEMPLATE  = lib
CONFIG   += plugin
CONFIG   += c++11

# 插件 DLL 名称
TARGET = CetMyPlugin

# 引入主工程的接口头文件（也可在插件目录内保留一份接口头文件副本）
INCLUDEPATH += ../../../CetDevelopmentKit/CetDevelopmentKit/interfaces
```

> 实际约定：`CetUpdatePlugin` / `CetLogManagerPlugin` / `CetProgressPlugin` 等在每个插件目录内各自保留一份接口头文件副本；`CetLicensePlugin` 则从 `CetToolLicense` 仓库（`$$PWD/../../../CetToolLicense/CetToolLicense`）引入许可相关头文件。

#### 步骤 3：实现插件类

```cpp
// cetmyplugin.h
#include <QObject>
#include "cetmyplugininterface.h"

class CetMyPlugin : public QObject, public CetMyPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qter.plugin.CetMyPluginInterface")
    Q_INTERFACES(CetMyPluginInterface)

public:
    explicit CetMyPlugin(QObject *parent = nullptr);

    void doSomething() override;
};
```

```cpp
// cetmyplugin.cpp
#include "cetmyplugin.h"

CetMyPlugin::CetMyPlugin(QObject *parent) : QObject(parent) { }

void CetMyPlugin::doSomething()
{
    // 实现逻辑
}
```

> **关键点**：插件类必须同时继承 `QObject` 和接口类，且 `Q_PLUGIN_METADATA` 中的 IID 字符串必须与接口头文件中 `Q_DECLARE_INTERFACE` 的 IID **完全一致**。

#### 步骤 4：在主程序中加载使用

由于 V2.5.0 起插件通过 `CetToolPluginContext` 管理，有两种方式：

**方式一（推荐）：扩展 CetToolPluginContext**

在 `CetToolDLLs/CetToolLibs/CetProductWizard/cettoolplugincontext.cpp` 的 `CetToolPluginContext` 实现中添加新接口的加载逻辑。

**方式二：在 MainWindow 中直接调用**

```cpp
// 通过 CetToolPluginContext 的 loadPlugin 方法
QObject *plugin = m_toolPluginCtx->loadPlugin("CetMyPlugin.dll");
CetMyPluginInterface *myPlugin = qobject_cast<CetMyPluginInterface*>(plugin);
if (myPlugin) {
    myPlugin->doSomething();
}
```

#### 步骤 5：部署

将编译产出的 `CetMyPlugin.dll` 复制到以下任一路径：

- 开发环境：`D:/Program Files (x86)/CetXiyuan/CetToolDLLs/plugins/`
- 发布包：`CetDevelopmentKit-Release/plugins/`

### 7.4 注意事项

| 事项 | 说明 |
|---|---|
| 编译器一致性 | 插件必须与主程序使用**相同的编译器和 Qt 版本**（当前：Qt 5.15.2 MinGW 32-bit）编译，否则 `QPluginLoader` 加载会静默失败 |
| Debug/Release 匹配 | 主程序 Release 版本只能加载 Release 编译的插件，Debug 同理 |
| IID 字符串 | `Q_PLUGIN_METADATA` 和 `Q_DECLARE_INTERFACE` 中的 IID 必须完全一致，大小写敏感 |
| 插件父对象 | `loadPlugin()` 对 QWidget 类插件将其 parent 设为主窗口（`parentWidget()`），其余插件则设为 `CetToolPluginContext` 自身，插件无需自行管理生命周期 |
| 完整性校验 | 如需对新 DLL 添加完整性校验，更新 `cettooldlls_hashes.h` 中的哈希值 |
| CetToolPluginContext 实现 | 该类的 `.cpp` 实现不在主仓库中，位于 `CetToolDLLs` 仓库，通过 INCLUDEPATH 引入编译 |

---

## 八、常见修改场景示例

### 场景一：增加新的配置项

以 EC200U 模块为例，增加一个"输出文件名"配置项（已存在，演示流程）：

1. **修改 UI 文件** (`ec200uconfigure.ui`): 在 Qt Designer 中添加新的 `QLineEdit` 控件
2. **修改头文件** (`ec200uconfigure.h`): 添加对应的槽函数声明
3. **修改实现文件** (`ec200uconfigure.cpp`):
   - 在构造函数中恢复配置值
   - 在 `startCommand()` 中使用新配置
   - 在槽函数中保存配置到 QSettings

```cpp
// 在构造函数中恢复配置
QString outputName = m_settings->value(QLatin1String("outputName"), "output.pac").toString();
ui->outputNameLineEdit->setText(outputName);

// 在 startCommand() 中使用
QString outputName = ui->outputNameLineEdit->text();
if (outputName.isEmpty()) {
    outputName.append("output.pac");
}

// 保存配置的槽函数
void EC200UConfigure::on_outputNameLineEdit_textChanged(const QString &arg1) {
    m_settings->beginGroup(QLatin1String(EC200U_SETTINGS_PREFIX));
    m_settings->setValue(QLatin1String("outputName"), arg1);
    m_settings->endGroup();
}
```

> 注意：源码中 EC200U 模块的 `outputName` **有意不做持久化**（每次任务文件名不同，见《架构设计》4.3），上面的"恢复配置/保存槽"仅为演示"新增一个配置项"的通用流程，并非该模块的实际实现。

### 场景二：修改接收缓存大小

ADB 接收处理通过 `QProcess::readyReadStandardOutput` 信号驱动，数据由 `readAllStandardOutput()` 读取。如需调整缓存行为：

**修改 `mainwindow.cpp` 中的 lambda**:

```cpp
connect(m_cmdProcess, &QProcess::readyReadStandardOutput, this, [=]() {
    // 方式一：限制单次读取大小
    QByteArray data = m_cmdProcess->read(4096);  // 每次最多读 4096 字节

    // 方式二：使用 readAllStandardOutput() 读取全部可用数据
    QByteArray data = m_cmdProcess->readAllStandardOutput();

    ui->plainTextEdit->appendPlainText(data);
    // ... 路由到模块
});
```

**修改 `ADBConfigure::recvHandle()` 中的数据匹配逻辑**:

如果数据量大导致单次接收不完整，需要改为累积缓存方式：

```cpp
void ADBConfigure::recvHandle(const QByteArray &data) {
    static QByteArray buffer;
    buffer.append(data);

    // 按行分割处理
    while (buffer.contains('\n')) {
        int idx = buffer.indexOf('\n');
        QByteArray line = buffer.left(idx).trimmed();
        buffer.remove(0, idx + 1);
        processLine(line);
    }
}
```

### 场景三：修改许可证激活时间间隔

修改 `CetToolPluginContext` 实现中的定时器设置（代码位于 `CetToolDLLs` 仓库）：

```cpp
// 首次激活延迟（默认 30 秒）
QTimer::singleShot(30 * 1000, this, [=]() { ... });

// 重试间隔初值（默认 10 分钟）
static int auto_timing_msec = 10 * 60 * 1000;

// 指数退避倍数（默认 2 倍），超过 24 小时重置回 10 分钟
auto_timing_msec = auto_timing_msec * 2;
if (auto_timing_msec > 24 * 60 * 60 * 1000)
    auto_timing_msec = 10 * 60 * 1000;
```

### 场景四：修改 QSettings 配置文件路径

修改各模块构造函数中的 `QSettings` 实例化：

```cpp
// 当前路径（相对于可执行文件目录）
m_settings(new QSettings("Configs/AppMaster.ini", QSettings::IniFormat))

// 如需改为绝对路径
m_settings(new QSettings("D:/MyApp/Configs/AppMaster.ini", QSettings::IniFormat))

// 如需改为系统标准路径
m_settings(new QSettings(
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/AppMaster.ini",
    QSettings::IniFormat))
```

### 场景五：控制进程执行策略（关闭 vs 禁止重复执行）

`on_processHandleButton_clicked()` 中使用 `#if 1` 控制行为：

```cpp
void MainWindow::on_processHandleButton_clicked()
{
#if 1
    m_cmdProcess->close();          // 当前行为：总是先关闭再启动
#else
    if (m_cmdProcess->isOpen()) {   // 旧行为：若正在运行则拒绝新任务
        QMessageBox::warning(...);
        return;
    }
#endif
    // ... 后续启动逻辑
```

- `#if 1`（当前）：点击"处理"总是关闭当前进程并启动新任务，适合需要快速重试的场景
- `#if 0`（旧行为）：若进程正在运行则弹出警告并拒绝，避免意外中断

修改 `#if 1` 为 `#if 0` 即可切换回旧的保护模式。

### 场景六：添加新的插件接口

1. **创建接口头文件**:

```cpp
// interfaces/cetyourinterface.h
class CetYourInterface
{
public:
    virtual ~CetYourInterface() {}
    virtual void yourMethod() = 0;
};
Q_DECLARE_INTERFACE(CetYourInterface, "org.qter.plugin.CetYourInterface")
```

2. **在 interfaces.pri 中注册**:

```qmake
HEADERS += $$PWD/cetyourinterface.h
```

3. **通过 CetToolPluginContext 加载**:

```cpp
// 在 CetToolPluginContext 中添加加载逻辑，或直接调用
QObject *plugin = m_toolPluginCtx->loadPlugin("CetYourPlugin.dll");
CetYourInterface *yourPlugin = qobject_cast<CetYourInterface*>(plugin);
```

---

## 九、项目文件结构速查

```
CetDevelopmentKit/
├── CetDevelopmentKit/              # 主程序源码
│   ├── CetDevelopmentKit.pro       # QMake 项目文件
│   ├── CMakeLists.txt              # CMake 构建文件
│   ├── main.cpp                    # 入口
│   ├── version.h                   # 版本定义
│   ├── mainwindow/                 # 主窗口
│   │   ├── mainwindow.h / .cpp
│   │   └── cettoolplugincontext.h  # 插件上下文（实现在 CetToolDLLs）
│   ├── modules/ec200u/             # EC200U 差分包模块
│   ├── modules/adb/                # ADB 打包导出模块
│   ├── interfaces/                 # 插件接口定义
│   └── libs/                       # 链接库
├── CetDevelopmentKit-Release/      # 发布目录（含 Qt 运行时）
├── CetDevelopmentKit-Setup/        # 安装包制作
│   ├── CetDevelopmentKit.iss       # Inno Setup 脚本
│   └── Output/                     # 安装包输出
├── CetDevelopmentKit-Patch/        # 补丁包目录
├── docs/                           # 文档
├── CHANGELOG.md                    # 变更记录
├── CLAUDE.md                       # Claude Code 指引
└── README.md                       # 项目说明
```

> **注意**: `CetToolPluginContext` 的 `.cpp` 实现位于 `CetToolDLLs` 仓库（`F:\sharefolder\cetqtlearn\CetToolDLLs\`），通过 `CetDevelopmentKit.pro` 中的 `INCLUDEPATH += $$PWD/../../CetToolDLLs` 引入。
