# 资金分析系统

基于 Qt 6 C++ + ZeroMQ + Python + DuckDB 的资金分析桌面工具

## 项目结构

```
firefly002/
├── src/                    # Qt C++ 前端源代码
├── backend/                # Python 后端服务
├── resources/              # 资源文件
├── database/               # 数据库文件
├── storage/                # 文件存储
└── CMakeLists.txt         # 构建配置
```

## 技术栈

- **前端**: Qt 6.0+ C++ (Ribbon界面, 多窗口)
- **后端**: Python 3.9+
- **通信**: ZeroMQ (REQ-REP + PUB-SUB)
- **数据库**: DuckDB
- **可视化**: Cytoscape.js + WebGL

## 环境要求

### 前端 (Qt C++)
- Qt 6.6+
- CMake 3.16+
- C++17 编译器 (MSVC 2019+, GCC 9+, Clang 10+)
- ZeroMQ (cppzmq)

### 后端 (Python)
- Python 3.9+
- 依赖包见 `backend/requirements.txt`

## 快速开始

### 1. 安装依赖

#### Windows (vcpkg)
```bash
# 安装 ZeroMQ
vcpkg install cppzmq:x64-windows

# 安装 Python 依赖
cd backend
python -m venv venv
venv\Scripts\activate
pip install -r requirements.txt
```

#### Linux
```bash
# 安装 ZeroMQ
sudo apt-get install libzmq3-dev libcppzmq-dev

# 安装 Python 依赖
cd backend
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

### 2. 构建前端

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 3. 运行

#### 先启动后端服务
```bash
cd backend
python main.py
```

#### 再启动前端程序
```bash
# Windows
.\build\bin\Release\FundAnalysis.exe

# Linux
./build/bin/FundAnalysis
```

## 开发指南

### 前端开发
- 主窗口: `src/ui/MainWindow.cpp`
- Ribbon界面: `src/ui/ribbon/RibbonBar.cpp`
- ZeroMQ通信: `src/network/ZmqClient.cpp`

### 后端开发
- 服务入口: `backend/main.py`
- 添加新的处理器到 `register_handler()`

## License

Copyright © 2024 FundAnalysis Team
