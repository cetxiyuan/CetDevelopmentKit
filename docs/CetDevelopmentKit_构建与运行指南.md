# CetDevelopmentKit 构建与运行指南

## 一、环境要求

| 组件 | 版本/说明 |
|------|-----------|
| 操作系统 | Windows 7 / 10 / 11（32位或64位） |
| Qt | 5.15.2（MinGW 32-bit） |
| 编译器 | MinGW 32-bit（随 Qt 5.15.2 安装） |
| CMake | 3.16+（仅 CMake 构建方式需要） |
| Qt Creator | 推荐 4.x / 5.x（包含 Qt 5.15.2 套件） |
| Inno Setup | 6.x（仅打包发布时需要） |
| Git | 任意版本 |

---

## 二、获取源代码

```bash
git clone <repository-url> CetDevelopmentKit
cd CetDevelopmentKit
```

或直接使用已有的源码目录 `F:\sharefolder\cetqtlearn\CetDevelopmentKit`。

---

## 三、依赖安装

### Windows

#### 1. 安装 Qt 5.15.2

通过 Qt 在线安装器安装：
- 选择 `Desktop Qt 5.15.2 MinGW 32-bit` 组件
- 确保安装了 `Qt Network` 模块

#### 2. 验证安装

```shell
qmake --version
# QMake version 3.1
# Using Qt version 5.15.2 in ...

gcc --version
# 或
g++.exe --version
```

### Linux / macOS

> 当前版本仅支持 Windows 平台。以下为未来跨平台移植的参考信息。

Linux:

```shell
sudo apt install qt5-default qtbase5-dev libqt5widgets5 libqt5network5
```

macOS:

```shell
brew install qt@5
```

---

## 四、构建步骤

### 方式一：Qt Creator（推荐）

1. 启动 **Qt Creator**
2. 打开项目文件：`CetDevelopmentKit/CetDevelopmentKit/CetDevelopmentKit.pro`
3. 选择构建套件：**Desktop Qt 5.15.2 MinGW 32-bit**
4. 选择构建模式：**Release**（或 Debug）
5. 点击 **构建** → **构建项目 "CetDevelopmentKit"**（或按 `Ctrl+B`）

构建输出目录：

```
CetDevelopmentKit/build/build-QMake-CetDevelopmentKit-Desktop_Qt_5_15_2_MinGW_32bit-Release/
  └── release/
      └── CetDevelopmentKit.exe
```

### 方式二：命令行（QMake）

```shell
# 进入源码目录
cd CetDevelopmentKit\CetDevelopmentKit

# 创建构建目录
mkdir build-qmake && cd build-qmake

# 运行 qmake
qmake ..\CetDevelopmentKit.pro -spec win32-g++ "CONFIG+=release"

# 编译
mingw32-make -j4
```

### 方式三：命令行（CMake）

```shell
# 进入源码目录
cd CetDevelopmentKit\CetDevelopmentKit

# 创建构建目录
mkdir build-cmake && cd build-cmake

# 配置（请根据实际 Qt 路径调整）
cmake .. -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\mingw81_32 ^
  -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . -j4
```

构建输出目录：

```
CetDevelopmentKit/build/build-CMake-CetDevelopmentKit-Desktop_Qt_5_15_2_MinGW_32bit-Release/
  └── CetDevelopmentKit.exe
```

> **注意**: CMake 构建需要将 `CMakeLists.zip` 解压到 `CetDevelopmentKit/` 目录下，或直接使用已有的 `CMakeLists.txt`。

---

## 五、运行方式

### 调试模式运行

在 Qt Creator 中：
1. 选择 **Debug** 构建配置
2. 点击 **运行** → **开始调试**（或按 `F5`）

调试模式下会启用 `qDebug()` 输出，包括：
- 功能模块切换日志
- 命令执行日志
- 进程退出码和状态
- 激活时序日志

### 发布模式运行

**方式一**：在 Qt Creator 中直接运行（选择 Release 配置）。

**方式二**：直接运行编译产物。

```shell
cd CetDevelopmentKit-Release
CetDevelopmentKit.exe
```

**方式三**：通过安装包安装后运行。

默认安装路径：`D:\Program Files (x86)\CetXiyuan\CetDevelopmentKit\CetDevelopmentKit.exe`

### 运行目录结构

可执行文件运行时需要以下目录结构：

```
CetDevelopmentKit.exe         # 主程序
├── CetProductWizard.dll      # 产品向导链接库
├── CetCommandLineTool.exe     # 命令行工具（V2.0.0 起随附）
├── plugins/                  # 插件 DLL 目录
│   ├── CetLicensePlugin.dll
│   ├── CetUpdatePlugin.dll
│   ├── CetLogManagerPlugin.dll
│   └── CetProgressPlugin.dll
├── Configs/                  # 配置文件目录（自动创建）
│   ├── AppMaster.ini
│   ├── EC200UConfigure.ini
│   └── ADBConfigure.ini
├── Qt5Core.dll               # Qt 运行时 DLL
├── Qt5Gui.dll
├── Qt5Network.dll
├── Qt5Widgets.dll
├── Qt5Svg.dll                # Qt SVG 支持
├── libEGL.dll                # Qt ANGLE / OpenGL
├── libGLESv2.dll
├── D3Dcompiler_47.dll
├── opengl32sw.dll
├── libcrypto-1_1.dll         # OpenSSL（供 Qt Network HTTPS）
├── libssl-1_1.dll
├── libgcc_s_dw2-1.dll        # MinGW 运行时
├── libstdc++-6.dll
├── libwinpthread-1.dll
├── platforms/
│   └── qwindows.dll          # Qt Windows 平台插件
├── styles/
│   └── qwindowsvistastyle.dll
├── iconengines/
│   └── qsvgicon.dll          # SVG 图标引擎
└── imageformats/             # Qt 图像格式插件（由 windeployqt 自动添加）
```

---

## 六、打包发布

### 发布构建流程

```
1. 修改 version.h        → 更新 VER_MAJOR/VER_MINOR/VER_MICRO、RELEASE_DATE、IS_RELEASE_VERSION
2. Qt Creator 编译        → Release 模式构建（Desktop Qt 5.15.2 MinGW 32bit）
3. 复制产物               → 将 .exe 覆盖到 CetDevelopmentKit-Release/
4. 修改 .iss 脚本         → 更新 MyAppVersion 与 version.h 保持一致
5. Inno Setup 编译        → 生成 CetDevelopmentKit-Setup-Release-V<版本>.exe
6. Git 提交               → 格式：「发布 V<版本>」
```

### 1. 准备 Release 目录

将编译产物和 Qt 运行时 DLL 复制到 `CetDevelopmentKit-Release/` 目录：

```shell
# 方法一：使用 windeployqt（自动收集 Qt 依赖）
cd CetDevelopmentKit-Release
windeployqt --release CetDevelopmentKit.exe

# 方法二：手动复制（已准备好的 Release 目录）
```

`CetDevelopmentKit-Release/` 目录中已包含所有必要的 Qt 运行时 DLL 和插件。

### 2. 使用 Inno Setup 制作安装包

1. 安装 [Inno Setup](https://jrsoftware.org/isinfo.php) (6.x)
2. 打开 `CetDevelopmentKit-Setup/CetDevelopmentKit.iss`
3. 确认版本号与 `version.h` 一致
4. 点击 **Build** → **Compile**

安装包输出到：

```
CetDevelopmentKit-Setup/Output/CetDevelopmentKit-Setup-Release-V2.5.0.exe
```

### 3. 安装包配置说明

| 配置项 | 值 |
|--------|-----|
| 应用名称 | CetDevelopmentKit |
| 发行商 | CetXiyuan |
| 默认安装路径 | `D:\Program Files (x86)\CetXiyuan\CetDevelopmentKit` |
| 压缩方式 | LZMA（固实压缩） |
| 语言 | 简体中文、英文 |
| 桌面快捷方式 | 可选（安装时勾选） |

---

## 七、常见问题

### 1. 编译错误：找不到 Qt5 模块

**错误**: `Project ERROR: Unknown module(s) in QT: widgets`

**解决方法**: 确保安装了 Qt 5.15.2 完整版，特别是 `Widgets` 和 `Network` 模块。

### 2. 编译错误：找不到 CetProductWizard 库

**错误**: `cannot find -lCetProductWizard`

**解决方法**: 确保 `CetDevelopmentKit/libs/CetProductWizard.dll` 存在。

### 3. 运行时错误：找不到 DLL

**错误**: "无法启动此程序，因为计算机中丢失 Qt5Core.dll"

**解决方法**:
- 使用 `windeployqt` 自动收集依赖
- 或从 Qt 安装目录手动复制所需 DLL 到可执行文件目录

### 4. 运行时错误：找不到插件

**错误**: 菜单项不可用，或提示"缺少插件库"

**解决方法**:
- 确保 `plugins/` 目录存在于以下两个位置之一：
  - `D:/Program Files (x86)/CetXiyuan/CetToolDLLs/plugins/`
  - `./plugins/`（可执行文件同级目录）

### 5. ADB 功能无法连接设备

**错误**: 点击执行后无响应或提示连接失败

**解决方法**:
- 确保 ADB 工具路径正确（指向 Android SDK 中的 `adb.exe`）
- 确保设备已通过 USB 连接并开启 USB 调试
- 运行 `adb devices` 确认设备已识别
- 确保设备用户名和密码正确

### 6. EC200U 差分包制作失败

**错误**: 工具路径无效或命令执行失败

**解决方法**:
- 确保 `fotacreate2` 工具路径正确
- 确保原固件包和新固件包（`.pac` 文件）路径有效
- 确保 `fota8910.xml` 配置文件存在于工具目录的 `setting/` 子目录下
- 检查输出目录是否有写入权限

### 7. QMake 和 CMake 的区别

| 特性 | QMake | CMake |
|------|-------|-------|
| 项目文件 | `CetDevelopmentKit.pro` | `CMakeLists.txt` |
| C++ 标准 | C++11 | C++17 |
| 编译宏 | `QT_DEPRECATED_WARNINGS`, `QT_MESSAGELOGCONTEXT` | 同上 + `BUILD_PRODUCTWIZARD_LIB` |
| 推荐用途 | 日常开发（与 Qt Creator 集成更好） | 跨平台构建 / CI 集成 |

### 8. 清理构建缓存

```shell
# QMake 构建
cd CetDevelopmentKit\build\build-QMake-...-Release
mingw32-make clean

# CMake 构建
cd CetDevelopmentKit\build\build-CMake-...-Release
cmake --build . --target clean

# 或直接删除整个 build 目录
rm -rf CetDevelopmentKit\build
```