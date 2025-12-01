#include "core/Application.h"
#include "core/Logger.h"
#include "ui/MainWindow.h"
#include <QMessageBox>
#include <QDir>

int main(int argc, char *argv[])
{
    // 创建应用程序
    Application app(argc, argv);
    
    // 设置日志文件
    QString logPath = app.getAppDataPath() + "/logs";
    QDir().mkpath(logPath);
    QString logFile = logPath + "/fundanalysis.log";
    Logger::instance()->setLogFile(logFile);
    
    Logger::instance()->info("========================================");
    Logger::instance()->info("资金分析系统启动");
    Logger::instance()->info("Version: 1.0.0");
    Logger::instance()->info("========================================");
    
    // 初始化应用程序
    if (!app.initialize()) {
        QMessageBox::critical(nullptr, "错误", "应用程序初始化失败!");
        return -1;
    }
    
    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.show();
    
    Logger::instance()->info("Main window shown");
    
    // 运行应用程序
    int ret = app.exec();
    
    Logger::instance()->info("Application exiting with code: " + QString::number(ret));
    Logger::instance()->info("========================================");
    
    return ret;
}
