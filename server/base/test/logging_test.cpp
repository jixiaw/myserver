#include "base/logging.h"
#include "base/logstream.h"
#include "base/async_logging.h"
using namespace server;

void outputFunc(const char* msg, int len) {
    AsyncLogging::getLogInstance()->append(msg, len);
}

int main()
{
    Logger::setOutput(outputFunc);
    LOG_INFO << "test log" <<" 123456789";
    LOG_ERROR << "ERROR in log" <<" code : 1";
    LOG_INFO << 123;
    LOG_INFO << 12.23;
    char buf[12];
    snprintf(buf, sizeof buf, "test %d", 12);
    LOG_INFO << buf;
    return 0;
}