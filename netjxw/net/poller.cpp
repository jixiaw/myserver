#include "poller.h"
#include "channel.h"
#include <iostream>

using namespace server::net;


Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop)
{
}

Poller::~Poller()
{
}

int Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    if (numEvents > 0) {
        std::cout<<numEvents<<" events happended"<<std::endl;
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        std::cout<<" nothing happended"<<std::endl;
    } else {
        std::cout<<"error pollor::poll()"<<std::endl;
    }
    return numEvents;
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels)
{
    for (PollFdList::iterator it = pollfds_.begin(); 
        it != pollfds_.end() && numEvents > 0; ++it) {
        if (it->revents > 0) {
            --numEvents;
            ChannelMap::iterator ch = channels_.find(it->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd() == it->fd);
            channel->setRevents(it->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    if (channel->index() < 0) {  // 不存在这个channel
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1; 
        channel->setIndex(idx);
        channels_[pfd.fd] = channel;
    } else {
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {  // 忽略这个fd
            pfd.fd = -1;
        }
    }
}

void Poller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    int idx = channel->index();
    // assert(channels_.find(channel->fd()) != channels_.end());
    auto it = channels_.find(channel->fd());
    assert(it != channels_.end());
    assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));
    channels_.erase(it);
    if (idx < static_cast<int>(pollfds_.size())-1) {
        std::swap(pollfds_[idx], pollfds_.back());
        auto now = channels_.find(pollfds_[idx].fd);
        assert(now != channels_.end());
        now->second->setIndex(idx);
    }
    pollfds_.pop_back();
    
}