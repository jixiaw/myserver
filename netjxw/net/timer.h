#ifndef SERVER_NET_TIMER_H
#define SERVER_NET_TIMER_H

#include "base/noncopyable.h"
#include "base/timestamp.h"
#include <functional>
using namespace server::base;

namespace server {
namespace net{


typedef std::function<void()> TimerCallback;
class Timer 
{
public:
    Timer(const TimerCallback& cb, TimeStamp when, double interval)
        : timerCallback_(cb),
          expiration_(when),
          interval_(interval),
          repeat_(interval_ > 0.0)
    {
    }

    void run() const { timerCallback_(); }
    TimeStamp expiration() const { return expiration_; }
    bool isRepeat() const {return repeat_;}
    void restart(TimeStamp now);

    friend class TimerQueue;
private:
    TimerCallback timerCallback_;
    TimeStamp expiration_;
    double interval_;
    bool repeat_;
};


}
}

#endif