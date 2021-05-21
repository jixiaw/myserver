#ifndef SERVER_BASE_TIMESTAMP_H
#define SERVER_BASE_TIMESTAMP_H

#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <string>

namespace server{
namespace base{
class TimeStamp{
public:
    TimeStamp(): microSecond_(0) {};
    explicit TimeStamp(int64_t microSecond): microSecond_(microSecond) {}

    // bool operator< (const TimeStamp& thr) {
    //     return this->microSecond_ < thr.microSecond_;
    // }
    // bool operator> (const TimeStamp& thr) {
    //     return this->microSecond_ > thr.microSecond_;
    // }
    // bool operator== (const TimeStamp& thr) {
    //     return this->microSecond_ == thr.microSecond_;
    // }

    static TimeStamp now();

    int64_t getSecond() const {return microSecond_ / microSecondPerSecond;}
    int64_t getMicroSecond() const {return microSecond_;}

    std::string toFormatString(bool showMicrosecond=true);
    std::string toString();

    static const int64_t microSecondPerSecond = 1000 * 1000;
private:
    int64_t microSecond_;

};

inline TimeStamp addTime(const TimeStamp& timestamp, double delay)
{
    int64_t delta = static_cast<int64_t>(delay * TimeStamp::microSecondPerSecond);
    return TimeStamp(timestamp.getMicroSecond() + delta);
}

inline bool operator<(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.getMicroSecond() < rhs.getMicroSecond();
}
inline bool operator==(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.getMicroSecond() == rhs.getMicroSecond();
}
}
}
#endif

