#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include <time.h>
#include <string>

class TimeUtil
{
public:
    TimeUtil();
    ~TimeUtil();

public:
    static time_t GetCurrentMs();
    static time_t GetCurrentSecond();
    static std::string GetCurrentStringSecond();
public:
    static std::string GetFormatTime(const std::string& format="%Y-%m-%d %H:%M:%S");
};

#endif