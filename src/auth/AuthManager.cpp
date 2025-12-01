#include "auth/AuthManager.h"
#include "network/ZmqClient.h"
#include "core/Application.h"
#include "core/Logger.h"
#include <QCryptographicHash>
#include <QUuid>
#include <QDateTime>
#include <QJsonArray>

AuthManager* AuthManager::s_instance = nullptr;

AuthManager* AuthManager::instance()
{
    if (!s_instance) {
        s_instance = new AuthManager(Application::instance());
    }
    return s_instance;
}

AuthManager::AuthManager(QObject *parent)
    : QObject(parent)
    , m_authenticated(false)
    , m_zmqClient(nullptr)
{
    // 从 Application 获取 ZmqClient
    // 注意: 在实际使用中需要确保 ZmqClient 已经初始化
}

AuthManager::~AuthManager()
{
}

bool AuthManager::login(const QString& username, const QString& password)
{
    Logger::instance()->info("Attempting login for user: " + username);
    
    // 清除之前的会话
    clearSession();
    
    // 密码加密
    QString passwordHash = hashPassword(password);
    
    // 构造认证请求
    QJsonObject params;
    params["username"] = username;
    params["password"] = passwordHash;
    
    // 发送认证请求到后端
    // 注意: 需要从 Application 或 MainWindow 获取 ZmqClient 实例
    // 这里先实现本地验证逻辑
    
    // 临时实现: 本地验证（用于测试）
    // TODO: 替换为后端验证
    bool authSuccess = false;
    
    // 默认测试账号
    if (username == "admin" && password == "admin123") {
        authSuccess = true;
        m_currentRole = "administrator";
        m_permissions << "data.import" << "data.clean" << "data.query" 
                      << "analysis.execute" << "report.generate" 
                      << "system.settings" << "user.manage";
    } else if (username == "user" && password == "user123") {
        authSuccess = true;
        m_currentRole = "user";
        m_permissions << "data.import" << "data.query" 
                      << "analysis.execute" << "report.generate";
    } else if (username == "guest" && password == "guest123") {
        authSuccess = true;
        m_currentRole = "guest";
        m_permissions << "data.query";
    }
    
    if (authSuccess) {
        m_authenticated = true;
        m_currentUser = username;
        
        // 生成会话令牌
        m_sessionToken = QUuid::createUuid().toString(QUuid::WithoutBraces);
        
        // 构造用户信息
        m_userInfo["username"] = username;
        m_userInfo["role"] = m_currentRole;
        m_userInfo["login_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        m_userInfo["session_token"] = m_sessionToken;
        
        Logger::instance()->info("Login successful for user: " + username + " (role: " + m_currentRole + ")");
        emit loginSuccess(username);
        return true;
    } else {
        Logger::instance()->warning("Login failed for user: " + username);
        emit loginFailed("用户名或密码错误");
        return false;
    }
}

void AuthManager::logout()
{
    if (!m_authenticated) {
        return;
    }
    
    Logger::instance()->info("User logout: " + m_currentUser);
    
    // 清除会话
    clearSession();
    
    emit logoutSuccess();
}

bool AuthManager::isAuthenticated() const
{
    return m_authenticated;
}

QString AuthManager::getCurrentUser() const
{
    return m_currentUser;
}

QString AuthManager::getCurrentRole() const
{
    return m_currentRole;
}

QJsonObject AuthManager::getUserInfo() const
{
    return m_userInfo;
}

bool AuthManager::hasPermission(const QString& permission) const
{
    if (!m_authenticated) {
        return false;
    }
    
    // 管理员拥有所有权限
    if (m_currentRole == "administrator") {
        return true;
    }
    
    return m_permissions.contains(permission);
}

bool AuthManager::hasRole(const QString& role) const
{
    if (!m_authenticated) {
        return false;
    }
    
    return m_currentRole == role;
}

QString AuthManager::getSessionToken() const
{
    return m_sessionToken;
}

bool AuthManager::isSessionValid() const
{
    // TODO: 实现会话有效期检查
    return m_authenticated && !m_sessionToken.isEmpty();
}

void AuthManager::refreshSession()
{
    if (!m_authenticated) {
        return;
    }
    
    // 更新会话令牌
    m_sessionToken = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_userInfo["session_token"] = m_sessionToken;
    m_userInfo["refresh_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    Logger::instance()->debug("Session refreshed for user: " + m_currentUser);
}

QString AuthManager::hashPassword(const QString& password) const
{
    // 使用 SHA-256 加密
    QByteArray hash = QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Sha256
    );
    return hash.toHex();
}

bool AuthManager::verifyPassword(const QString& password, const QString& hash) const
{
    return hashPassword(password) == hash;
}

void AuthManager::clearSession()
{
    m_authenticated = false;
    m_currentUser.clear();
    m_currentRole.clear();
    m_sessionToken.clear();
    m_userInfo = QJsonObject();
    m_permissions.clear();
}
