#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>

class Logger : public QObject
{
    Q_OBJECT

public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    static Logger* instance();
    
    void setLogLevel(LogLevel level) { m_logLevel = level; }
    void setLogFile(const QString& filePath);
    
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    void critical(const QString& message);
    
    void log(LogLevel level, const QString& message);

signals:
    void logMessageGenerated(const QString& message, int level);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    QString formatMessage(LogLevel level, const QString& message);
    QString levelToString(LogLevel level);
    
private:
    static Logger* s_instance;
    LogLevel m_logLevel;
    QFile* m_logFile;
    QTextStream* m_logStream;
    QMutex m_mutex;
};

#endif // LOGGER_H
