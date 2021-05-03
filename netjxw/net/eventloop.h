#ifndef SERVER_NET_EVENTLOOP_H
#define SERVER_NET_EVENTLOOP_H
#include "base/noncopyable.h"
#include <thread>
#include <iostream>
#include <assert.h>
#include <memory>
#include <vector>
namespace server
{
namespace net
{

class Channel;
class Poller;

class EventLoop: server::noncopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

    static EventLoop* getEventLoopOfCurrentThread();

    void updateChannel(Channel* channel);

private:
    void abortNotInLoopThread() { std::cout<<"error, not in loop thread"<<std::endl; }

    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    const std::thread::id threadId_;
    bool quit_;
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;
};



}
}

#endif