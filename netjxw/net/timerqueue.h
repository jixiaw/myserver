#ifndef SERVER_NET_TIMERQUEUE_H
#define SERVER_NET_TIMERQUEUE_H

#include "base/noncopyable.h"
#include "base/timestamp.h"
#include "net/channel.h"
#include <memory>
#include <map>
#include <vector>
#include <functional>

namespace server {
namespace net{

class EventLoop;
class Timer;

typedef std::function<void ()> TimerCallback;
typedef std::shared_ptr<Timer> Timerptr;
class TimerQueue : noncopyable 
{
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();
    
    void addTimer(const TimerCallback& cb, TimeStamp when, double interval);
    void addTimerInLoop(const Timerptr& timer);
    void handleRead();
    // todo
    void cancel();

    int createTimerfd();
    void resetTimerfd(int timerfd, TimeStamp expiration);
    struct timespec howMuchTimeFromNow(TimeStamp when);
    void readTimerfd(int timerfd, TimeStamp now);

    // 插入定时器
    bool insert(const Timerptr& timer); 
    // 获取已经到期的定时器
    std::vector<Timerptr> getExpired(TimeStamp now);
    // 重置已经到期的定时器
    void reset(const std::vector<Timerptr>& expired, TimeStamp now);
    
private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerChannel_;
    std::multimap<TimeStamp, Timerptr> timerMap;

};


}
}

#endif