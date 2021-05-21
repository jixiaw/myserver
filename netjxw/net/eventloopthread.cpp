#include "eventloopthread.h"

using namespace server::net;

EventLoopThread::EventLoopThread()
: loop_(NULL),
  thread_(std::bind(&EventLoopThread::threadFunc, this))
{
}

EventLoopThread::~EventLoopThread()
{
    if (loop_ != NULL)
    {
        loop_->quit();
        thread_.join();
    }
}


EventLoop* EventLoopThread::startLoop()
{
    {
        std::unique_lock<std::mutex> look_(mutex_);
        while(loop_ == NULL) {
            condition_.wait(look_);
        }
    }

    return loop_;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        std::unique_lock<std::mutex> look_(mutex_);
        loop_ = &loop;
        condition_.notify_one();
    }
    loop.loop();
}