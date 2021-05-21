#include "eventloop.h"
#include <poll.h>
#include <sys/eventfd.h>
#include "poller.h"
#include "channel.h"
#include "net/timerqueue.h"
#include <unistd.h>
using namespace server::net;

__thread EventLoop* t_loopInThisThread = NULL;
const int kPollTimeMs = 10000;
EventLoop::EventLoop()
    : looping_(false),
      threadId_(std::this_thread::get_id()),
      quit_(false),
      callingPendingFunctors_(false),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      poller_(new Poller(this)),
      timerQueue_(new TimerQueue(this))
{
    if (t_loopInThisThread) {
        std::cout<<"EventLoop already exists. "<<std::endl;
    }
    else {
        std::cout<<"EventLoop created in thread "<< threadId_<< std::endl;
        t_loopInThisThread = this;
    }
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
        std::cout<<"Faild in eventfd"<<std::endl;
    }
    return evtfd;
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one){
        std::cout<<"EventLoop::wakeup() writes " << n <<"bytes instead of 8"<<std::endl;
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        std::cout<<"EventLoop::handleRead() reads " << n <<"bytes instead of 8"<<std::endl;
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
    std::cout<<"EventLoop stop" <<std::endl;
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
