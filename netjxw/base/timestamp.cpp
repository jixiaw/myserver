#include "timestamp.h"


using namespace server;

TimeStamp TimeStamp::now()
{
    struct timeval tmp;
    gettimeofday(&tmp, NULL);
    int64_t seconds = tmp.tv_sec;
    return TimeStamp(seconds * microSecondPerSecond + tmp.tv_usec);
}

std::string TimeStamp::toFormatString(bool showMicrosecond) 
{
    char buf[64];
    time_t seconds = static_cast<time_t>(microSecond_ / microSecondPerSecond);
    struct tm tm_time;
    // gmtime_r 返回的是UTC时间
    // localtime_r 返回的是本地时间UTC+8
    localtime_r(&seconds, &tm_time);
    if (showMicrosecond) { 
        int microseconds = static_cast<int> (microSecond_ % microSecondPerSecond);
        snprintf(buf, sizeof buf, "%4d%02d%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year+1900, tm_time.tm_mon+1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                microseconds);
    } else {
        snprintf(buf, sizeof buf, "%4d%02d%02d %02d:%02d:%02d",
                tm_time.tm_year+1900, tm_time.tm_mon+1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

std::string TimeStamp::toString()
{
    char buf[32];
    int64_t seconds = microSecond_ / microSecondPerSecond;
    int64_t microseconds = microSecond_ % microSecondPerSecond;
    snprintf(buf, sizeof buf, "%ld.%06ld", seconds, microseconds);
    return buf;
}

