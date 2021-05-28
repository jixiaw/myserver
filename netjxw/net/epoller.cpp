#include "epoller.h"
#include "base/logging.h"
#include "eventloop.h"
#include <sys/epoll.h>
#include <unistd.h>
#include "channel.h"
using namespace server::net;
using namespace std;


Epoller::Epoller(EventLoop* loop)
: ownerLoop_(loop),
  epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
  events_(kInitNumEvents)
{
    if (epollfd_ < 0) {
        LOG_ERROR << "Epoller::Epoller() epoll_create1 error.";
    }
}

Epoller::~Epoller()
{
    ::close(epollfd_);
}

int Epoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), 
                static_cast<int>(events_.size()), timeoutMs);
    if (numEvents > 0) {
        LOG_INFO << "Epoller::poll() " << numEvents << " events happend";
        fillActiveChannels(numEvents, activeChannels);
        // expand events size
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(2 * numEvents);
        }
    } else if (numEvents == 0) {
        LOG_INFO << "Epoller::poll() nothing happened";
    } else {
        LOG_ERROR << "Epoller::poll()";
    }
    return numEvents;
}


void Epoller::updateChannel(Channel* channel)
{
    int fd = channel->fd();
    auto it = channelMap_.find(fd);
    if (it != channelMap_.end()) {
        struct epoll_event ep;
        ep.events = channel->events();
        ep.data.ptr = channel;
        ::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ep);
        assert(it->second == channel);
    } else {
        struct epoll_event ep;
        ep.events = channel->events();
        ep.data.ptr = channel;
        ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ep);
        channelMap_[fd] = channel;
    }
}

void Epoller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    int fd = channel->fd();
    auto it = channelMap_.find(fd);
    assert(it != channelMap_.end());
    assert(it->second == channel);
    channelMap_.erase(it);
    struct epoll_event ep;
    ep.events = channel->events();
    ep.data.ptr = channel;
    ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ep);
}

void Epoller::fillActiveChannels(int numEvents, ChannelList* activeChannels)
{
    assert(static_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        // int fd = channel->fd();
        // auto it = channelMap_.find(fd);
        // assert(it != channelMap_.end());
        // assert(it->second == channel);
        channel->setRevents(events_[i].events);
        activeChannels->push_back(channel);        
    }
}
