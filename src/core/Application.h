#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QString>
#include <QSettings>

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application();
    
    // 获取应用程序单例
    static Application* instance();
    
    // 初始化应用程序
    bool initialize();
    
    // 获取配置
    QString getConfigValue(const QString& key, const QString& defaultValue = QString()) const;
    void setConfigValue(const QString& key, const QString& value);
    
    // 获取当前用户
    QString getCurrentUser() const { return m_currentUser; }
    void setCurrentUser(const QString& user) { m_currentUser = user; }
    
    // 获取当前任务ID
    QString getCurrentTaskId() const { return m_currentTaskId; }
    void setCurrentTaskId(const QString& taskId) { m_currentTaskId = taskId; }
    
    // 应用程序路径
    QString getAppDataPath() const;
    QString getDatabasePath() const;
    QString getStoragePath() const;

signals:
    void userChanged(const QString& user);
    void taskChanged(const QString& taskId);

private:
    void setupPaths();
    void loadSettings();
    
private:
    QSettings* m_settings;
    QString m_currentUser;
    QString m_currentTaskId;
    QString m_appDataPath;
    QString m_databasePath;
    QString m_storagePath;
};

#endif // APPLICATION_H
