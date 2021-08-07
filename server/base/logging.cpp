#include "logging.h"

namespace server {
Logger::LogLevel g_logLevel = Logger::INFO;

// void Logger::setLogLevel(Logger::LogLevel level)
// {
//     g_logLevel = level;
// }
// Logger::LogLevel Logger::getLogLevel()
// {
//     return g_logLevel;
// }
void defaultOutput(const char* msg, int len) {
  fwrite(msg, 1, len, stdout);
}
Logger::OutputFunc g_output = defaultOutput;
void Logger::setOutput(OutputFunc func) {
  g_output = func;
}

Logger::~Logger() {
    // logStream_.print();
    logStream_ << '\n';
    g_output(logStream_.c_str(), logStream_.size());
}

}

