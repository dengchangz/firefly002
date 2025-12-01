#include "network/ZmqClient.h"
#include "core/Logger.h"
#include <QUuid>
#include <QDateTime>
#include <QThread>

// ==================== ZmqClient Implementation ====================

ZmqClient::ZmqClient(QObject *parent)
    : QObject(parent)
    , m_context(std::make_unique<zmq::context_t>(1))
    , m_receiveThread(nullptr)
    , m_connected(false)
{
}

ZmqClient::~ZmqClient()
{
    disconnect();
}

bool ZmqClient::connectToServer(const QString& reqEndpoint, const QString& subEndpoint)
{
    try {
        m_reqEndpoint = reqEndpoint;
        m_subEndpoint = subEndpoint;
        
        Logger::instance()->info("Connecting to ZeroMQ server...");
        Logger::instance()->info("REQ endpoint: " + reqEndpoint);
        Logger::instance()->info("SUB endpoint: " + subEndpoint);
        
        // 创建REQ socket（用于请求-响应）
        m_reqSocket = std::make_unique<zmq::socket_t>(*m_context, zmq::socket_type::req);
        m_reqSocket->connect(reqEndpoint.toStdString());
        
        // 设置接收超时
        int timeout = 30000; // 30秒
        m_reqSocket->set(zmq::sockopt::rcvtimeo, timeout);
        m_reqSocket->set(zmq::sockopt::sndtimeo, timeout);
        
        // 创建订阅线程
        m_receiveThread = new ZmqReceiveThread(subEndpoint, this);
        connect(m_receiveThread, &ZmqReceiveThread::messageReceived,
                this, &ZmqClient::onNotificationMessage);
        m_receiveThread->start();
        
        m_connected = true;
        emit connected();
        
        Logger::instance()->info("ZeroMQ client connected successfully");
        return true;
        
    } catch (const zmq::error_t& e) {
        QString error = QString("ZeroMQ connect error: %1").arg(e.what());
        Logger::instance()->error(error);
        emit errorOccurred(error);
        m_connected = false;
        return false;
    }
}

void ZmqClient::disconnect()
{
    if (!m_connected) {
        return;
    }
    
    Logger::instance()->info("Disconnecting from ZeroMQ server...");
    
    // 停止接收线程
    if (m_receiveThread) {
        m_receiveThread->stop();
        m_receiveThread->wait(3000);
        delete m_receiveThread;
        m_receiveThread = nullptr;
    }
    
    // 关闭REQ socket
    if (m_reqSocket) {
        m_reqSocket->close();
        m_reqSocket.reset();
    }
    
    m_connected = false;
    emit disconnected();
    
    Logger::instance()->info("ZeroMQ client disconnected");
}

QJsonObject ZmqClient::request(const QString& action, const QJsonObject& params, int timeout)
{
    if (!m_connected) {
        return QJsonObject{
            {"status", "error"},
            {"message", "Not connected to server"}
        };
    }
    
    try {
        // 1. 构建请求
        QJsonObject request = buildRequest(action, params);
        QJsonDocument doc(request);
        QString jsonStr = doc.toJson(QJsonDocument::Compact);
        
        Logger::instance()->debug("Sending request: " + action);
        
        // 2. 发送请求
        zmq::message_t requestMsg(jsonStr.toStdString());
        auto sendResult = m_reqSocket->send(requestMsg, zmq::send_flags::none);
        
        if (!sendResult.has_value()) {
            Logger::instance()->error("Failed to send request");
            return QJsonObject{{"status", "error"}, {"message", "Failed to send request"}};
        }
        
        // 3. 接收响应
        zmq::message_t replyMsg;
        
        // 临时设置超时
        m_reqSocket->set(zmq::sockopt::rcvtimeo, timeout);
        
        auto recvResult = m_reqSocket->recv(replyMsg, zmq::recv_flags::none);
        
        if (!recvResult.has_value()) {
            Logger::instance()->warning("Request timeout: " + action);
            return QJsonObject{{"status", "error"}, {"message", "Request timeout"}};
        }
        
        // 4. 解析响应
        std::string replyStr = replyMsg.to_string();
        QJsonObject response = parseResponse(replyStr);
        
        Logger::instance()->debug("Received response: " + response["status"].toString());
        
        return response;
        
    } catch (const zmq::error_t& e) {
        QString error = QString("Request error: %1").arg(e.what());
        Logger::instance()->error(error);
        emit errorOccurred(error);
        return QJsonObject{{"status", "error"}, {"message", error}};
    }
}

void ZmqClient::subscribe(const QString& topic)
{
    if (m_receiveThread) {
        m_receiveThread->subscribe(topic);
        Logger::instance()->info("Subscribed to topic: " + (topic.isEmpty() ? "ALL" : topic));
    }
}

void ZmqClient::onNotificationMessage(const QString& message)
{
    try {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isObject()) {
            return;
        }
        
        QJsonObject notification = doc.object();
        QString type = notification["type"].toString();
        QJsonObject data = notification["data"].toObject();
        
        Logger::instance()->debug("Received notification: " + type);
        emit notificationReceived(type, data);
        
    } catch (...) {
        Logger::instance()->error("Failed to parse notification message");
    }
}

QString ZmqClient::generateMessageId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QJsonObject ZmqClient::buildRequest(const QString& action, const QJsonObject& params)
{
    QJsonObject request;
    request["msg_id"] = generateMessageId();
    request["timestamp"] = QDateTime::currentSecsSinceEpoch();
    request["action"] = action;
    request["params"] = params;
    request["user_id"] = "current_user";  // 从Application获取
    
    return request;
}

QJsonObject ZmqClient::parseResponse(const std::string& response)
{
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response));
    return doc.object();
}

// ==================== ZmqReceiveThread Implementation ====================

ZmqReceiveThread::ZmqReceiveThread(const QString& endpoint, QObject *parent)
    : QThread(parent)
    , m_endpoint(endpoint)
    , m_context(std::make_unique<zmq::context_t>(1))
    , m_running(false)
{
}

ZmqReceiveThread::~ZmqReceiveThread()
{
    stop();
}

void ZmqReceiveThread::stop()
{
    m_running = false;
}

void ZmqReceiveThread::subscribe(const QString& topic)
{
    m_topics.append(topic);
}

void ZmqReceiveThread::run()
{
    try {
        // 创建SUB socket
        m_subSocket = std::make_unique<zmq::socket_t>(*m_context, zmq::socket_type::sub);
        m_subSocket->connect(m_endpoint.toStdString());
        
        // 订阅主题
        if (m_topics.isEmpty()) {
            m_subSocket->set(zmq::sockopt::subscribe, "");  // 订阅所有
        } else {
            for (const QString& topic : m_topics) {
                m_subSocket->set(zmq::sockopt::subscribe, topic.toStdString());
            }
        }
        
        // 设置接收超时（用于定期检查m_running）
        m_subSocket->set(zmq::sockopt::rcvtimeo, 1000);
        
        Logger::instance()->info("ZeroMQ receive thread started");
        m_running = true;
        
        while (m_running) {
            try {
                zmq::message_t msg;
                auto result = m_subSocket->recv(msg, zmq::recv_flags::none);
                
                if (result.has_value()) {
                    std::string msgStr = msg.to_string();
                    emit messageReceived(QString::fromStdString(msgStr));
                }
                
            } catch (const zmq::error_t& e) {
                if (e.num() != EAGAIN) {  // EAGAIN 是超时，正常
                    Logger::instance()->warning(QString("Receive error: %1").arg(e.what()));
                }
            }
        }
        
        m_subSocket->close();
        Logger::instance()->info("ZeroMQ receive thread stopped");
        
    } catch (const zmq::error_t& e) {
        Logger::instance()->error(QString("Receive thread error: %1").arg(e.what()));
    }
}
