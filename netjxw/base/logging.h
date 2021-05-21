#ifndef SERVER_BASE_LOGGING_H
#define SERVER_BASE_LOGGING_H
#include "logstream.h"
#include <string>
namespace server {
namespace base {

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
    server::base::LogStream logStream_;

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
}

#define LOG_TRACE if (server::base::Logger::getLogLevel() <= server::base::Logger::TRACE) \
    server::base::Logger(server::base::Logger::TRACE).getStream()
#define LOG_DEBUG if (server::base::Logger::getLogLevel() <= server::base::Logger::DEBUG) \
    server::base::Logger(server::base::Logger::DEBUG).getStream()
#define LOG_INFO if (server::base::Logger::getLogLevel() <= server::base::Logger::INFO) \
    server::base::Logger(server::base::Logger::INFO).getStream()
#define LOG_WARN if (server::base::Logger::getLogLevel() <= server::base::Logger::WARN) \
    server::base::Logger(server::base::Logger::WARN).getStream()
#define LOG_ERROR if (server::base::Logger::getLogLevel() <= server::base::Logger::ERROR) \
    server::base::Logger(server::base::Logger::ERROR).getStream()
#define LOG_FATAL if (server::base::Logger::getLogLevel() <= server::base::Logger::FATAL) \
    server::base::Logger(server::base::Logger::FATAL).getStream()

#endif