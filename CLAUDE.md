# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

CetDevelopmentKit 是一个基于 Qt 5.15.2 (MinGW 32-bit) 的 Windows 桌面开发集成工具包，当前版本 V2.3.1，归属 CetXiyuan 公司。主要功能：
- **EC200U 差分包制作**：生成 EC200U 模块的 OTA 差分升级包
- **ADB 设备数据打包导出**：通过 ADB 连接 Android 设备并打包导出数据

## 构建方式

支持两种构建方式（均有对应的 build 目录）：

**QMake（推荐）：**
```
# 在 Qt Creator 中打开 CetDevelopmentKit/CetDevelopmentKit.pro
# 选择 Desktop Qt 5.15.2 MinGW 32bit - Release 构建套件
```

**CMake：**
```
# 在 Qt Creator 中打开 CetDevelopmentKit/CMakeLists.zip 解压后的 CMakeLists.txt
```

构建输出目录：
- QMake: `CetDevelopmentKit/build/build-QMake-CetDevelopmentKit-Desktop_Qt_5_15_2_MinGW_32bit-Release/`
- CMake: `CetDevelopmentKit/build/build-CMake-CetDevelopmentKit-Desktop_Qt_5_15_2_MinGW_32bit-Release/`

## 打包发布

1. 将编译产物复制到 `CetDevelopmentKit-Release/` 目录（已包含所有 Qt 运行时 DLL）
2. 使用 Inno Setup 打开 `CetDevelopmentKit-Setup/CetDevelopmentKit.iss` 编译生成安装包
3. 安装包输出到 `CetDevelopmentKit-Setup/Output/`，默认安装路径为 `D:\Program Files (x86)\CetXiyuan\CetDevelopmentKit`

## 版本管理

版本号统一在 `CetDevelopmentKit/version.h` 中维护：
- `VER_MAJOR` / `VER_MINOR` / `VER_MICRO`：三段式版本号
- `RELEASE_DATE`：采用内部编码格式，如 `(CXYQK5152.CDPK230.G512)`
- `IS_RELEASE_VERSION`：1 为正式版，0 为补丁版（补丁号由 `PATCH_PACKET` 指定）
- 发布时同步更新 `CetDevelopmentKit-Setup/CetDevelopmentKit.iss` 中的 `MyAppVersion`

## 架构说明

### 插件系统

应用运行时从以下路径加载功能插件（`.dll`）：
1. 优先从 `D:/Program Files (x86)/CetXiyuan/CetToolDLLs/plugins/` 加载
2. 若不存在则从 `./plugins/` 加载（Release 目录下的 `plugins/` 子目录）

插件通过纯虚接口类定义契约，位于 `CetDevelopmentKit/interfaces/`：

| 接口文件 | 插件 DLL | 用途 |
|---|---|---|
| `cetlicenseinterface.h` | `CetLicensePlugin.dll` | 授权激活（绑定 `PRODUCT_NAME` 产品固钥） |
| `cetupdateinterface.h` | `CetUpdatePlugin.dll` | 在线更新检测 |
| `cetlogmanagerinterface.h` | `CetLogManagerPlugin.dll` | 日志管理窗口 |
| `cetprogressinterface.h` | `CetProgressPlugin.dll` | 进度显示 |

静态库 `CetProductWizard.dll` 位于 `CetDevelopmentKit/libs/`，链接方式见 `.pro` 文件末行。

### 许可证激活逻辑

首次启动 10 秒后自动触发激活，之后按 30→60→120 分钟指数退避重试。未激活时主界面 `centralWidget` 被禁用。激活以 `PRODUCT_NAME`（= `APP_NAME` + 版本数字，如 `CetDevelopmentKit230`）作为产品 ID。

### 功能模块结构

每个功能模块（`modules/ec200u/`、`modules/adb/`）是独立的 `QWidget` 子类，通过 `.pri` 文件引入，共享主窗口的 `QProcess` 和 `QPlainTextEdit` 实例。模块对外暴露两个接口：
- `startCommand()`：返回要执行的命令行字符串
- `recvHandle(data)`：处理进程标准输出（用于解析进度等）

配置持久化使用 `QSettings`，INI 文件路径为 `Configs/AppMaster.ini`。
