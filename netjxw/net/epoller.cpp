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
    LOG_DEBUG << "fd total count " << channelMap_.size();
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), 
                static_cast<int>(events_.size()), timeoutMs);
    if (numEvents > 0) {
        LOG_INFO << "Epoller::poll() " << numEvents << " events happend";
        fillActiveChannels(numEvents, activeChannels);
        // expand events size
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(2 * numEvents);
            LOG_DEBUG << "Epoller::poll() resize eventsVec: " << events_.size();
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
    int state = channel->index();
    // 新加入的或者已经删除的
    if (state == kNew || state == kDeleted) {
        if (state == kNew) {  // 新加入的在map里找不到
            assert(channelMap_.find(fd) == channelMap_.end());
            channelMap_[fd] = channel;
        } else {        // 删掉的在map里找的到，但是内核epoll里没有
            assert(channelMap_.find(fd) != channelMap_.end());
            assert(channelMap_[fd] == channel);
        }
        channel->setIndex(kAdded);
        struct epoll_event ep;
        ep.events = channel->events();
        ep.data.ptr = channel;
        ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ep);
    } else {
        auto it = channelMap_.find(fd);
        assert(it != channelMap_.end());
        assert(it->second == channel);
        assert(state == kAdded);
        struct epoll_event ep;
        ep.events = channel->events();
        ep.data.ptr = channel;
        // 没有事件，从epoll中删除，但还在map里
        if (channel->isNoneEvent()) {
            ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ep);
            channel->setIndex(kDeleted);
        } else {
            ::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ep);
        }
    }
    LOG_DEBUG << "Epoller::updateChannel() fd: " << fd <<" state: " 
            << state << " -> " <<channel->index();
}

void Epoller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    int fd = channel->fd();
    int state = channel->index();
    assert(state == kDeleted || state == kAdded);
    channel->setIndex(kDeleted);
    auto it = channelMap_.find(fd);
    assert(it != channelMap_.end());
    assert(it->second == channel);
    channelMap_.erase(it);
    if (state == kAdded) {
        struct epoll_event ep;
        ep.events = channel->events();
        ep.data.ptr = channel;
        ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ep);
    }
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
