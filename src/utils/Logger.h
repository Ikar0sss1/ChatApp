#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>

class Logger
{
public:
    enum LogLevel {
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    };
    
    static Logger& getInstance();
    
    void initialize();
    void log(const QString& message, LogLevel level = INFO);

private:
    Logger();
    ~Logger();
    
    QString levelToString(LogLevel level) const;
    
    QFile* m_logFile;
    QTextStream* m_logStream;
};

#endif // LOGGER_H