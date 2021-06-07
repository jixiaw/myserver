#include "eventloop.h"
#include <poll.h>
#include <sys/eventfd.h>
#include "poller.h"
#include "epoller.h"
#include "channel.h"
#include "net/timerqueue.h"
#include "base/logging.h"
#include <unistd.h>
#include <signal.h>
using namespace server::net;

__thread EventLoop* t_loopInThisThread = NULL;
const int kPollTimeMs = 10000;

class IgnoreSigPipe{
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
IgnoreSigPipe ignoreSig;

EventLoop::EventLoop()
    : looping_(false),
      threadId_(std::this_thread::get_id()),
      quit_(false),
      callingPendingFunctors_(false),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
    //   poller_(new Epoller(this)),
      poller_(new Poller(this)),
      timerQueue_(new TimerQueue(this))
{
    if (t_loopInThisThread) {
        LOG_FATAL << "EventLoop already exists.";
    }
    else {
        LOG_INFO << "EventLoop created in thread.";
        t_loopInThisThread = this;
    }
    LOG_DEBUG << "EventLoop::EventLoop() wakeup eventfd: " << wakeupFd_;
    wakeupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

int EventLoop::createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) 
    {
        LOG_ERROR << "Faild in eventfd.";
    }
    return evtfd;
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one){
        LOG_ERROR << "EventLoop::wakeup() writes " << n <<"bytes instead of 8";
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n <<"bytes instead of 8";
    }
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    // ::poll(NULL, 0, 5*1000);
    while(!quit_) {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handleEvent();
        }
        doPendingFunctors();
    }
    LOG_INFO << "EventLoop stop";
    looping_ = false;
}


void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

void EventLoop::abortNotInLoopThread() 
{
    LOG_FATAL << "FATAL, not in loop thread";
}

void EventLoop::runInLoop(const Functor& cb) 
{
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        std::unique_lock<std::mutex> look_(mutex_);
        pendingFunctors_.push_back(cb);
    }
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::doPendingFunctors()  // 里面的functors可能会调用queueInLoop，因此需要wakeup
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> look_(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (size_t i = 0; i < functors.size(); ++i) {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::runAt(const TimeStamp& time, const TimerCallback& cb)
{
    timerQueue_->addTimer(cb, time, 0.0);
}
void EventLoop::runAfter(double delay, const TimerCallback& cb)
{
    TimeStamp time(addTime(TimeStamp::now(), delay));
    timerQueue_->addTimer(cb, time, 0.0);
}
void EventLoop::runEvery(double interval, const TimerCallback& cb)
{
    TimeStamp time(addTime(TimeStamp::now(), interval));
    timerQueue_->addTimer(cb, time, interval);
}
