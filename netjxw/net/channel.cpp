#include "channel.h"
#include "eventloop.h"
#include "base/logging.h"
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

Channel::~Channel()
{
    assert(!eventHandling);
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
    eventHandling = true;
    LOG_DEBUG << reventsToString();
    if (revents_ & POLLNVAL) {  // 非法请求
        LOG_ERROR << "Channel::handleEvent() event POLLNVAL";
        loop_->quit();
    }
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (closeCallBack_) closeCallBack_();
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
    eventHandling = false;
}

std::string Channel::eventsToString() const
{
    return eventsToString(fd_, events_);
}

std::string Channel::reventsToString() const
{
    return eventsToString(fd_, revents_);
}

std::string Channel::eventsToString(int fd, int ev)
{
    std::string res = std::to_string(fd) + ": ";
    if (ev & POLLIN)
        res.append("IN ");
    if (ev & POLLPRI)
        res.append("PRI ");
    if (ev & POLLOUT)
        res.append("OUT ");
    if (ev & POLLHUP)
        res.append("HUP ");
    if (ev & POLLRDHUP)
        res.append("RDHUP ");
    if (ev & POLLERR)
        res.append("ERR ");
    if (ev & POLLNVAL)
        res.append("NVAL ");
    return res;
}