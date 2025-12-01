@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

echo ========================================
echo   资金分析系统 - 构建脚本 (Windows)
echo ========================================
echo.

REM 检查 CMake（优先使用 Qt 自带的 CMake）
set "CMAKE_EXE="

REM 尝试从 Qt 安装目录查找 CMake
for %%D in (C:\Qt18 D:\Qt E:\Qt) do (
    if exist "%%D" (
        for /f "delims=" %%F in ('dir /b /s "%%D\*cmake.exe" 2^>nul ^| findstr "Tools\CMake"') do (
            set "CMAKE_EXE=%%F"
            goto :cmake_found
        )
    )
)

:cmake_found
if not defined CMAKE_EXE (
    REM 尝试系统 PATH 中的 CMake
    where cmake >nul 2>nul
    if !ERRORLEVEL! EQU 0 (
        set "CMAKE_EXE=cmake"
    ) else (
        echo [错误] 未找到 CMake
        echo.
        echo 请选择以下方式之一：
        echo 1. 使用 Qt 自带的 CMake（推荐）
        echo    Qt 安装时会包含 CMake，通常在 Qt\Tools\CMake_64\bin\cmake.exe
        echo.
        echo 2. 从 Qt Creator 命令行运行此脚本
        echo    "工具" -^> "外部" -^> "Qt Creator Command Prompt"
        echo.
        echo 3. 手动下载安装 CMake: https://cmake.org/download/
        echo.
        pause
        exit /b 1
    )
)

echo [检测] 使用 CMake: !CMAKE_EXE!
echo.

REM 检测 Qt 安装路径
set "QT_DIR="
for %%D in (C:\Qt18 D:\Qt E:\Qt) do (
    if exist "%%D" (
        for /f "delims=" %%F in ('dir /b /ad "%%D\6.*" 2^>nul') do (
            if exist "%%D\%%F\msvc2022_64" (
                set "QT_DIR=%%D\%%F\msvc2022_64"
                goto :qt_found
            )
            if exist "%%D\%%F\msvc2019_64" (
                set "QT_DIR=%%D\%%F\msvc2019_64"
                goto :qt_found
            )
        )
    )
)

:qt_found
if defined QT_DIR (
    echo [检测] 找到 Qt: !QT_DIR!
    set "CMAKE_PREFIX_PATH=!QT_DIR!"
) else (
    echo [警告] 未自动检测到 Qt 安装路径
    echo [提示] 如果配置失败，请手动设置 Qt 路径：
    echo         set CMAKE_PREFIX_PATH=C:\Qt\6.6.0\msvc2022_64
)
echo.

REM 创建构建目录
if not exist "build" mkdir build
cd build

echo [1/3] 配置项目...
if defined CMAKE_PREFIX_PATH (
    "!CMAKE_EXE!" .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="!CMAKE_PREFIX_PATH!" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
) else (
    "!CMAKE_EXE!" .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [错误] CMake 配置失败
    echo.
    echo 可能的原因：
    echo 1. Qt 路径未正确设置
    echo 2. 缺少 ZeroMQ 库（需要通过 vcpkg 安装）
    echo.
    echo 解决方法：
    echo 1. 安装 vcpkg: git clone https://github.com/Microsoft/vcpkg.git
    echo 2. 安装 ZeroMQ: vcpkg install cppzmq:x64-windows
    echo 3. 集成到系统: vcpkg integrate install
    echo.
    cd ..
    pause
    exit /b 1
)

echo.
echo [2/3] 编译项目 (Release)...
"!CMAKE_EXE!" --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [错误] 编译失败
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo   [3/3] 构建完成！
echo ========================================
echo.
echo 可执行文件: build\bin\Release\FundAnalysis.exe
echo.
cd ..

pause
