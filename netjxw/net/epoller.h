#ifndef SERVER_NET_EPOLLER_H
#define SERVER_NET_EPOLLER_H

#include <vector>
#include <map>
#include "eventloop.h"
#include <sys/epoll.h>

struct pollfd;

namespace server {
namespace net {

class EventLoop;
class Channel;

class Epoller
{
public:
    typedef std::vector<Channel*> ChannelList;

    Epoller(EventLoop* loop);
    ~Epoller();
    // EventLoop调用poll, 返回待操作的 activeChannels
    int poll(int timeoutMs, ChannelList* activeChannels);

    // EventLoop调用，用来插入或更新 channel
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInLoopThread() {ownerLoop_->assertInLoopThread();}

private:
    //　遍历 pollfds_, 找出所有活动的fd, 添加进activeChannels
    void fillActiveChannels(int numEvents, ChannelList* activeChannels);
    typedef std::vector<struct epoll_event> EpollFdList;
    // typedef std::map<int, Channel*> ChannelMap;
    EventLoop* ownerLoop_;
    int epollfd_;
    EpollFdList events_;    
    // ChannelMap channels_;   //　fd 到channel的映射
};

}
}

#endif