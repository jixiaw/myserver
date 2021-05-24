#include "eventloopthreadpool.h"
#include "eventloop.h"
#include "eventloopthread.h"
#include "base/logging.h"
#include <string>

using namespace server::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
: baseLoop_(baseLoop),
  name_(name),
  numThreads_(0),
  next_(0),
  start_(false)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{

}

void EventLoopThreadPool::setNumThreads(int numThreads)
{
    numThreads_ = numThreads;
}

void EventLoopThreadPool::start()
{
    assert(!start_);
    baseLoop_->assertInLoopThread();
    start_ = true;
    for (int i = 0; i < numThreads_; ++i) {
        EventLoopThread* t = new EventLoopThread(name_ + ": thread #" + std::to_string(i));
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
        LOG_DEBUG << "EventLoopThreadPool::start() creates [" << t->getName() << "].";
    }
}

EventLoop* EventLoopThreadPool::getLoop()
{
    baseLoop_->assertInLoopThread();
    assert(start_);
    EventLoop* loop = baseLoop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        LOG_DEBUG << "EventLoopThreadPool::getLoop() gets [" << threads_[next_]->getName() << "].";
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}