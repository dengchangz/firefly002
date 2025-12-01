#include "core/Application.h"
#include "core/Logger.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_settings(nullptr)
{
    setOrganizationName("FundAnalysis");
    setOrganizationDomain("fundanalysis.com");
    setApplicationName("FundAnalysis");
    setApplicationVersion("1.0.0");
}

Application::~Application()
{
    if (m_settings) {
        delete m_settings;
    }
}

Application* Application::instance()
{
    return qobject_cast<Application*>(QApplication::instance());
}

bool Application::initialize()
{
    Logger::instance()->info("Initializing application...");
    
    // 设置路径
    setupPaths();
    
    // 加载设置
    loadSettings();
    
    // 创建必要的目录
    QDir().mkpath(m_appDataPath);
    QDir().mkpath(m_databasePath);
    QDir().mkpath(m_storagePath);
    QDir().mkpath(m_storagePath + "/original_files");
    QDir().mkpath(m_storagePath + "/exports");
    QDir().mkpath(m_storagePath + "/backups");
    
    Logger::instance()->info("Application initialized successfully");
    return true;
}

void Application::setupPaths()
{
    // 应用数据路径
    m_appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    // 数据库路径
    m_databasePath = m_appDataPath + "/database";
    
    // 存储路径
    m_storagePath = m_appDataPath + "/storage";
    
    Logger::instance()->info("App data path: " + m_appDataPath);
    Logger::instance()->info("Database path: " + m_databasePath);
    Logger::instance()->info("Storage path: " + m_storagePath);
}

void Application::loadSettings()
{
    m_settings = new QSettings(m_appDataPath + "/config.ini", QSettings::IniFormat);
    
    // 加载默认设置
    if (!m_settings->contains("backend/host")) {
        m_settings->setValue("backend/host", "localhost");
    }
    if (!m_settings->contains("backend/req_port")) {
        m_settings->setValue("backend/req_port", 5555);
    }
    if (!m_settings->contains("backend/pub_port")) {
        m_settings->setValue("backend/pub_port", 5556);
    }
}

QString Application::getConfigValue(const QString& key, const QString& defaultValue) const
{
    if (m_settings) {
        return m_settings->value(key, defaultValue).toString();
    }
    return defaultValue;
}

void Application::setConfigValue(const QString& key, const QString& value)
{
    if (m_settings) {
        m_settings->setValue(key, value);
        m_settings->sync();
    }
}

QString Application::getAppDataPath() const
{
    return m_appDataPath;
}

QString Application::getDatabasePath() const
{
    return m_databasePath;
}

QString Application::getStoragePath() const
{
    return m_storagePath;
}
