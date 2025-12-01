@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

echo ========================================
echo   资金分析系统 - 启动脚本
echo ========================================
echo.

REM 检查 Python
where python >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [错误] 未找到 Python
    pause
    exit /b 1
)

REM 检查虚拟环境
if not exist "backend\venv" (
    echo [提示] 首次运行，正在创建Python虚拟环境...
    cd backend
    python -m venv venv
    call venv\Scripts\activate
    echo [提示] 安装Python依赖...
    pip install -r requirements.txt
    cd ..
    echo.
)

REM 启动后端服务
echo [启动] Python 后端服务...
start "资金分析系统 - 后端" cmd /k "cd backend && venv\Scripts\activate && python main.py"

REM 等待后端启动
timeout /t 3 /nobreak >nul

REM 启动前端程序
echo [启动] Qt 前端程序...
if exist "build\bin\Release\FundAnalysis.exe" (
    start "" "build\bin\Release\FundAnalysis.exe"
) else if exist "build\bin\Debug\FundAnalysis.exe" (
    start "" "build\bin\Debug\FundAnalysis.exe"
) else (
    echo [错误] 未找到可执行文件，请先运行 build.bat 构建项目
    pause
    exit /b 1
)

echo.
echo [完成] 系统已启动
echo.
echo 前端程序和后端服务已在独立窗口中运行
echo 关闭后端窗口将停止后端服务
echo.
pause
