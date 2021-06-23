#include "base/time_util.h"
#include <sys/time.h>
#include "base/string_util.h"

TimeUtil::TimeUtil() {}

TimeUtil::~TimeUtil() {}

time_t TimeUtil::GetCurrentSecond()
{
    struct timeval tmp;
    gettimeofday(&tmp, NULL);
    return tmp.tv_sec;
}

time_t TimeUtil::GetCurrentMs()
{
    struct timeval tmp;
    gettimeofday(&tmp, NULL);
    return tmp.tv_sec * 1000L + tmp.tv_usec / 1000L;
}

std::string TimeUtil::GetCurrentStringSecond()
{
    return StringUtil::ToString((int)GetCurrentSecond());
}

std::string TimeUtil::GetFormatTime(const std::string& format)
{
    struct tm tmp;
    char buffer[80];
    time_t t = time(0);
    localtime_r(&t, &tmp);
    strftime(buffer, sizeof(buffer), format.c_str(), &tmp);
    return std::string(buffer);
}