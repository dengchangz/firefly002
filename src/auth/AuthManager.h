#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

class ZmqClient;

class AuthManager : public QObject
{
    Q_OBJECT

public:
    static AuthManager* instance();
    
    // 认证方法
    bool login(const QString& username, const QString& password);
    void logout();
    bool isAuthenticated() const;
    
    // 用户信息
    QString getCurrentUser() const;
    QString getCurrentRole() const;
    QJsonObject getUserInfo() const;
    
    // 权限检查
    bool hasPermission(const QString& permission) const;
    bool hasRole(const QString& role) const;
    
    // 会话管理
    QString getSessionToken() const;
    bool isSessionValid() const;
    void refreshSession();

signals:
    void loginSuccess(const QString& username);
    void loginFailed(const QString& reason);
    void logoutSuccess();
    void sessionExpired();

private:
    explicit AuthManager(QObject *parent = nullptr);
    ~AuthManager();
    
    QString hashPassword(const QString& password) const;
    bool verifyPassword(const QString& password, const QString& hash) const;
    void clearSession();

private:
    static AuthManager* s_instance;
    
    bool m_authenticated;
    QString m_currentUser;
    QString m_currentRole;
    QString m_sessionToken;
    QJsonObject m_userInfo;
    QStringList m_permissions;
    
    ZmqClient* m_zmqClient;
};

#endif // AUTHMANAGER_H
