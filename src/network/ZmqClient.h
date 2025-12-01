#ifndef ZMQCLIENT_H
#define ZMQCLIENT_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QString>
#include <QThread>
#include <zmq.hpp>
#include <memory>

class ZmqReceiveThread;

class ZmqClient : public QObject
{
    Q_OBJECT

public:
    explicit ZmqClient(QObject *parent = nullptr);
    ~ZmqClient();
    
    // 连接到服务器
    bool connectToServer(const QString& reqEndpoint, const QString& subEndpoint);
    void disconnect();
    
    bool isConnected() const { return m_connected; }
    
    // 同步请求（REQ-REP模式）
    QJsonObject request(const QString& action, const QJsonObject& params, int timeout = 30000);
    
    // 订阅主题
    void subscribe(const QString& topic = "");
    
signals:
    // 收到通知信号
    void notificationReceived(const QString& type, const QJsonObject& data);
    
    // 连接状态变化
    void connected();
    void disconnected();
    
    // 错误信号
    void errorOccurred(const QString& error);

private slots:
    void onNotificationMessage(const QString& message);

private:
    QString generateMessageId();
    QJsonObject buildRequest(const QString& action, const QJsonObject& params);
    QJsonObject parseResponse(const std::string& response);
    
private:
    std::unique_ptr<zmq::context_t> m_context;
    std::unique_ptr<zmq::socket_t> m_reqSocket;    // REQ socket
    
    ZmqReceiveThread* m_receiveThread;
    
    QString m_reqEndpoint;
    QString m_subEndpoint;
    bool m_connected;
};

// ZeroMQ 接收线程
class ZmqReceiveThread : public QThread
{
    Q_OBJECT

public:
    explicit ZmqReceiveThread(const QString& endpoint, QObject *parent = nullptr);
    ~ZmqReceiveThread();
    
    void stop();
    void subscribe(const QString& topic);

signals:
    void messageReceived(const QString& message);

protected:
    void run() override;

private:
    QString m_endpoint;
    std::unique_ptr<zmq::context_t> m_context;
    std::unique_ptr<zmq::socket_t> m_subSocket;
    bool m_running;
    QStringList m_topics;
};

#endif // ZMQCLIENT_H
