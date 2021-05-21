#include "base/logging.h"
#include "base/logstream.h"

using namespace server::base;

int main()
{
    LOG_INFO << "test log" <<" 123456789";
    LOG_ERROR << "ERROR in log" <<" code : 1";
    return 0;
}