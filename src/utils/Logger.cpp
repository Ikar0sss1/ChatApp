#include "Logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

Logger::Logger()
    : m_logFile(nullptr)
    , m_logStream(nullptr)
{
}

Logger::~Logger()
{
    delete m_logStream;
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::initialize()
{
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logDir);
    
    QString logPath = logDir + "/imchat.log";
    m_logFile = new QFile(logPath);
    
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_logStream = new QTextStream(m_logFile);
        m_logStream->setEncoding(QStringConverter::Utf8);
    }
}

void Logger::log(const QString& message, LogLevel level)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString levelStr = levelToString(level);
    QString formattedMessage = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
    
    if (m_logStream) {
        *m_logStream << formattedMessage << Qt::endl;
        m_logStream->flush();
    }
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case INFO: return "INFO";
        case WARNING: return "WARN";
        case ERROR: return "ERROR";
        default: return "UNKN";
    }
}