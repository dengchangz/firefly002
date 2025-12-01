#include "core/Logger.h"
#include <QDebug>
#include <QDir>

Logger* Logger::s_instance = nullptr;

Logger::Logger()
    : m_logLevel(INFO)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
{
}

Logger::~Logger()
{
    if (m_logStream) {
        delete m_logStream;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

Logger* Logger::instance()
{
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

void Logger::setLogFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    // 关闭旧文件
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
    
    // 创建日志目录
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // 打开新文件
    m_logFile = new QFile(filePath);
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream = new QTextStream(m_logFile);
        m_logStream->setEncoding(QStringConverter::Utf8);
    } else {
        qWarning() << "Failed to open log file:" << filePath;
        delete m_logFile;
        m_logFile = nullptr;
    }
}

void Logger::debug(const QString& message)
{
    log(DEBUG, message);
}

void Logger::info(const QString& message)
{
    log(INFO, message);
}

void Logger::warning(const QString& message)
{
    log(WARNING, message);
}

void Logger::error(const QString& message)
{
    log(ERROR, message);
}

void Logger::critical(const QString& message)
{
    log(CRITICAL, message);
}

void Logger::log(LogLevel level, const QString& message)
{
    if (level < m_logLevel) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString formattedMessage = formatMessage(level, message);
    
    // 输出到控制台
    switch (level) {
        case DEBUG:
            qDebug().noquote() << formattedMessage;
            break;
        case INFO:
            qInfo().noquote() << formattedMessage;
            break;
        case WARNING:
            qWarning().noquote() << formattedMessage;
            break;
        case ERROR:
        case CRITICAL:
            qCritical().noquote() << formattedMessage;
            break;
    }
    
    // 写入文件
    if (m_logStream) {
        (*m_logStream) << formattedMessage << "\n";
        m_logStream->flush();
    }
    
    // 发送信号
    emit logMessageGenerated(formattedMessage, level);
}

QString Logger::formatMessage(LogLevel level, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = levelToString(level);
    return QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
}

QString Logger::levelToString(LogLevel level)
{
    switch (level) {
        case DEBUG:    return "DEBUG";
        case INFO:     return "INFO ";
        case WARNING:  return "WARN ";
        case ERROR:    return "ERROR";
        case CRITICAL: return "CRIT ";
        default:       return "UNKN ";
    }
}
