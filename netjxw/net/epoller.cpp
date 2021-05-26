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
  epollfd_(::epoll_create1(EPOLL_CLOEXEC))
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
        LOG_INFO << numEvents << " events happend";
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        LOG_INFO << "nothing happened";
    } else {
        LOG_ERROR << "Epoller::poll()";
    }
    return numEvents;
}


void Epoller::updateChannel(Channel* channel);
void Epoller::removeChannel(Channel* channel);
void Epoller::fillActiveChannels(int numEvents, ChannelList* activeChannels)
{
    assert(static_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setRevents(events_[i].events);
        activeChannels->push_back(channel);        
    }
}
