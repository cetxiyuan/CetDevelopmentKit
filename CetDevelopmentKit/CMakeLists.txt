# 要求 CMake 版本至少为 3.16
cmake_minimum_required(VERSION 3.16)

# 定义项目名称，项目版本为 0.1，并指定该项目是 C++ 项目
project(CetDevelopmentKit VERSION 0.1 LANGUAGES CXX)

# 自动处理 Qt 的 UI、MOC 和资源文件
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)	# 确保 CMake 只使用 C++17 或更高版本

# 寻找 Qt5 库，要求至少包含 Widgets 组件。如果找不到，CMake 会报错。
find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

# 项目源文件
file(GLOB SOURCES 
	*.cpp
	*.h
	*.ui
	interfaces/*.cpp
	interfaces/*.h
	interfaces/*.ui
	mainwindow/*.cpp
	mainwindow/*.h
	mainwindow/*.ui
	modules/*/*.cpp
	modules/*/*.h
	modules/*/*.ui
	app.rc
)

# 包含子目录下的源文件
include_directories(.)
include_directories(interfaces)
include_directories(mainwindow)
include_directories(modules/adb)
include_directories(modules/ec200u)

# 创建可执行文件，PROJECT_NAME 对应 CetSScom。包括了源文件、头文件、UI 文件、资源文件等
add_executable(${PROJECT_NAME}
	${SOURCES}
)

# 定义 QT_DEPRECATED_WARNINGS 宏来启用弃用警告
target_compile_definitions(${PROJECT_NAME} PRIVATE 
	QT_DEPRECATED_WARNINGS
)

# 定义 QT_MESSAGELOGCONTEXT 宏来启用消息日志上下文信息
target_compile_definitions(${PROJECT_NAME} PRIVATE 
	QT_MESSAGELOGCONTEXT
	BUILD_PRODUCTWIZARD_LIB
)

# 将 Qt5 的 Widgets、SerialPort 和 Network 库链接到项目的可执行文件
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt5::Widgets
    Qt5::Network
)

# 在 Windows 上链接额外的库
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
    	-L${CMAKE_SOURCE_DIR}/libs/ -lCetProductWizard
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}								# 动态添加自定义的选项
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE	 						# 将目标设置为 Windows 可执行文件
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .							# 将应用程序 bundle 安装到当前目录
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}		# 将库文件安装到指定的库目录
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}		# 将可执行文件安装到指定的可执行文件目录
)
