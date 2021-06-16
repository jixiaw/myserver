#ifndef SERVER_BASE_LOGGING_H
#define SERVER_BASE_LOGGING_H
#include "logstream.h"
#include "base/timestamp.h"
#include <string>
namespace server {

class Logger 
{
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };
    Logger(LogLevel level) 
    : tempLevel_(level)
    {
        TimeStamp t = TimeStamp::now();
        logStream_ << t.toFormatString() << " ";
        logStream_ << getLevelString(level);
    }
    ~Logger() {
        logStream_.print();
    }
    static LogLevel getLogLevel();
    static void setLogLevel(LogLevel level);


    static std::string getLevelString(LogLevel level)
    {
        switch (level)
        {
        case TRACE:
            return "TRACE ";
            break;
        case DEBUG:
            return "DEBUG ";
            break;
        case INFO:
            return "INFO  ";
            break;
        case WARN:
            return "WARN  ";
            break;
        case ERROR:
            return "ERROR ";
            break;
        case FATAL:
            return "FATAL ";
            break;
        default:
            break;
        }
        return " ";
    }

    LogStream& getStream() { return logStream_; }
    
private:
    LogLevel tempLevel_;
    server::LogStream logStream_;

};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::getLogLevel()
{
    return g_logLevel;
}
inline void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}
// void Logger::setLogLevel(Logger::LogLevel level) {
//     g_logLevel = level;
// } 


}

#define LOG_TRACE if (server::Logger::getLogLevel() <= server::Logger::TRACE) \
    server::Logger(server::Logger::TRACE).getStream()
#define LOG_DEBUG if (server::Logger::getLogLevel() <= server::Logger::DEBUG) \
    server::Logger(server::Logger::DEBUG).getStream()
#define LOG_INFO if (server::Logger::getLogLevel() <= server::Logger::INFO) \
    server::Logger(server::Logger::INFO).getStream()
#define LOG_WARN if (server::Logger::getLogLevel() <= server::Logger::WARN) \
    server::Logger(server::Logger::WARN).getStream()
#define LOG_ERROR if (server::Logger::getLogLevel() <= server::Logger::ERROR) \
    server::Logger(server::Logger::ERROR).getStream()
#define LOG_FATAL if (server::Logger::getLogLevel() <= server::Logger::FATAL) \
    server::Logger(server::Logger::FATAL).getStream()

#endif