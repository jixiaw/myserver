#include "eventloop.h"
#include <poll.h>
#include "poller.h"
#include "channel.h"
using namespace server::net;

__thread EventLoop* t_loopInThisThread = NULL;
const int kPollTimeMs = 10000;
EventLoop::EventLoop()
    : looping_(false),
      threadId_(std::this_thread::get_id()),
      quit_(false),
      poller_(new Poller(this))
{
    if (t_loopInThisThread) {
        std::cout<<"EventLoop already exists. "<<std::endl;
    }
    else {
        std::cout<<"EventLoop created in thread "<< threadId_<< std::endl;
        t_loopInThisThread = this;
    }
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
    }
    std::cout<<"EventLoop stop" <<std::endl;
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}