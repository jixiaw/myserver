#include "poller.h"
#include "channel.h"
#include <iostream>
#include "base/logging.h"
using namespace server::net;
using namespace server::base;

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
        LOG_INFO << "Pollor:poll() " << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        LOG_INFO << "Pollor:poll() nothing happended";
    } else {
        LOG_ERROR << "Pollor:poll() error";
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
    LOG_DEBUG << "Poller::updateChannel fd = " << channel->fd() << " events = " << channel->events();
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
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {  // 没有要监听的事件就将fd变成负数
            pfd.fd = -channel->fd()-1;
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
        // 跟最后一个交换
        int channelEnd = pollfds_.back().fd;
        std::swap(pollfds_[idx], pollfds_.back());
        if (channelEnd < 0) {   // 最后一个没有事件的话
            channelEnd = -channelEnd - 1;
        }
        // auto now = channels_.find(pollfds_[idx].fd);
        // assert(now != channels_.end());
        // now->second->setIndex(idx);
        channels_[channelEnd]->setIndex(idx);
    }
    pollfds_.pop_back();
    
}
