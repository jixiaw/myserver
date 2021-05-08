#include "channel.h"
#include "eventloop.h"
#include <poll.h>

#include <iostream>


using namespace server::net;


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fdarg)
    : loop_(loop),
      fd_(fdarg),
      events_(0),
      revents_(0),
      index_(-1)
{
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
    assert(events_ == kNoneEvent);
    loop_->removeChannel(this);
}

void Channel::handleEvent()
{
    if (revents_ & POLLNVAL) {  // 非法请求
        std::cout<<"channel::handle event() POLLNVAL"<<std::endl;
        loop_->quit();
    }
    if (revents_ & (POLLERR | POLLNVAL)) {  // 错误
        if (errorCallBack_) errorCallBack_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) { // 可读
        if (readCallBack_) readCallBack_();
    }
    if (revents_ & POLLOUT) {   // 可写
        if (writeCallBack_) writeCallBack_();
    }
}