#include "timerqueue.h"
#include "timer.h"
#include "channel.h"
#include "eventloop.h"
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>
using namespace server::net;

TimerQueue::TimerQueue(EventLoop* loop)
: loop_(loop),
  timerfd_(createTimerfd()),
  timerChannel_(loop, timerfd_)
{
    timerChannel_.setReadCallBack(std::bind(&TimerQueue::handleRead, this));
    timerChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerChannel_.disableAll();
    timerChannel_.remove();
    ::close(timerfd_);
}

void TimerQueue::addTimer(const TimerCallback& cb, TimeStamp when, double interval)
{
    std::shared_ptr<Timer> timer(new Timer(cb, when, interval));
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
}

int TimerQueue::createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        printf("Failed in timerfd_create.\n");
    }
    return timerfd;
}
void TimerQueue::readTimerfd(int timerfd, TimeStamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    printf("TimerQueue::handleRead() %d at %s.\n", howmany, now.toString().c_str());
    if (n != sizeof howmany)
    {
        printf("TimerQueue::handleRead() reads %d bytes instead of 8\n", n);
    }
}
void TimerQueue::addTimerInLoop(const std::shared_ptr<Timer>& timer)
{
    loop_->assertInLoopThread();
    bool isearliest = insert(timer);
    if (isearliest) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}
struct timespec TimerQueue::howMuchTimeFromNow(TimeStamp when)
{
    int64_t microseconds = when.getMicroSecond()
                            - TimeStamp::now().getMicroSecond();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / TimeStamp::microSecondPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % TimeStamp::microSecondPerSecond) * 1000);
    return ts;
}

void TimerQueue::resetTimerfd(int timerfd, TimeStamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof newValue);
    memset(&oldValue, 0, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        printf("timerfd_settime()\n");
    }
}

bool TimerQueue::insert(const Timerptr& timer)
{
    loop_->assertInLoopThread();
    bool isearliest = false;
    auto it = timerMap.begin();
    if (it == timerMap.end() || timer->expiration() < it->first) {
        isearliest = true;
    }
    timerMap.insert({timer->expiration(), timer});
    printf("insert %s, number: %d\n", timer->expiration().toFormatString().c_str(), timerMap.size());
    return isearliest;
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    TimeStamp now(TimeStamp::now());
    readTimerfd(timerfd_, now);
    std::vector<Timerptr> expired = getExpired(now);
    for (Timerptr& t : expired){
        t->run();
    }
    reset(expired, now);
}
std::vector<Timerptr> TimerQueue::getExpired(TimeStamp now)
{
    std::vector<Timerptr> expired;
    auto end = timerMap.lower_bound(now);
    for (auto it = timerMap.begin(); it != end; ++it) {
        expired.push_back(it->second);
    }
    timerMap.erase(timerMap.begin(), end);
    return expired;
}
void TimerQueue::reset(const std::vector<Timerptr>& expired, TimeStamp now) 
{
    for (const Timerptr& it : expired) {
        if (it->isRepeat()) {
            it->restart(now);
            insert(it);
        }
    }
    if (!timerMap.empty())
    {
        resetTimerfd(timerfd_, timerMap.begin()->first);
    }
}
