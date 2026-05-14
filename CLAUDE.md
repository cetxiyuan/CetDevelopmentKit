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

## HTML 文档要求

### 1. 样式统一
- **页面主体**：白色（`#ffffff`）或浅灰色背景（`#fafafa`），深色文字（`#1e2a3e` 或 `#3b3f46`），禁止深色或黑色背景。
- **代码块**：使用深色背景（推荐 `#1e1e1e`、`#282c34` 或 `#2d2d2d`），搭配语法高亮（关键字、字符串、注释、函数名等使用对比鲜明的亮色）。
- **表格、引用、列表**：保持浅色系，使用柔和的边框和底色（如 `#f6f8fa` 背景，`#e2e8f0` 边框）。
- 页面最大宽度 `1000px`，居中显示，移动端自适应；代码块支持横向滚动（`overflow-x: auto`）。

### 2. 语法高亮
- 必须支持至少以下语言：C、C++、Python、Bash、JavaScript、Markdown、JSON、YAML。
- 推荐使用 **highlight.js** 或 **Prism.js**，并加载对应语言插件。若使用纯 CSS 方案，需覆盖常见 token 类型。
- **重要**：highlight.js 的深色主题 CSS（如 `atom-one-dark`）会覆盖所有 `code` 元素的背景色，导致行内代码也变成深色背景。必须对行内代码加 `!important` 强制覆盖：
  ```css
  #doc-content code:not(pre code) {
    background: #f0f4f8 !important;
    color: #c7254e !important;
  }
  ```
- **重要**：highlight.js 无法识别的语言（如 `.pro`、`.iss` 等）不会着色，代码块文字会继承深色主题的暗色，在深色背景上看不清。必须为 `pre code` 设置默认浅色文字：
  ```css
  #doc-content pre code {
    color: #abb2bf;  /* 默认浅灰，hljs 着色后会覆盖此值 */
  }
  ```

### 3. 一键复制全文
- 每个 HTML 文档顶部或右上角有一个 **[Copy]** 按钮。
- 点击按钮复制**整个文档的正文纯文本**（不含按钮、页脚、YAML Frontmatter 等界面元素，但保留段落结构、列表符号、代码块内的缩进与换行）。
- 复制成功后显示短暂提示（如"✅ 已复制全文"），提示浮层需位于页面底部中央，2 秒后自动消失。

### 4. 代码块独立复制
- 每个 `<pre><code>` 代码块右上角增加一个小型"📋 复制"按钮。
- 点击仅复制该代码块内的文本内容，不影响其他内容。
- 如果无法为每个代码块生成独立按钮，则至少保证全文复制时代码块格式完整（不会丢失缩进或变成连续文本）。

### 5. 修复与优化
- 自动修复常见 Markdown 格式错误：
  - 标题 `#标题` → `# 标题`
  - 列表 `-条目` → `- 条目`
  - 代码块标记未闭合时补全
- 忽略 YAML Frontmatter（`--- ... ---` 块），不显示在正文中；但将其中的 `title` 字段值用于 HTML `<title>` 标签。
- 相对路径资源（如图片）需转换为有效路径，或将图片复制到输出目录并更新引用。
- 每页底部添加一行：`📄 文档转换 · 笔名 CetXiyuan`。

### 6. 输出格式与编码
- 使用 `UTF-8` 编码，文档类型为 `<!DOCTYPE html>`。
- **单个 `.md` 文件**：直接输出完整的 HTML 代码。
- **多个 `.md` 文件**：输出一个独立的 **Python 脚本**（依赖 `markdown` 和 `pygments` 或 `highlight.js` 的本地库），该脚本读取所有 `.md` 文件，按上述要求生成对应的 `.html` 文件。
- html文档统一保存到 `docs/html/` 目录下。
- 若 Markdown 中包含本地图片，脚本须将图片复制到 `docs/html/assets/` 并更新引用路径（保留相对目录结构）。
