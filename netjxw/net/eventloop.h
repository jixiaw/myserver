#ifndef SERVER_NET_EVENTLOOP_H
#define SERVER_NET_EVENTLOOP_H
#include "base/noncopyable.h"
#include <thread>
#include <iostream>
#include <assert.h>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include "base/timestamp.h"
#include "net/common.h"
using namespace server;

namespace server
{
namespace net
{

class Channel;
class Poller;
class TimerQueue;
class Epoller;

class EventLoop: server::noncopyable {
public:
    typedef std::function<void()> Functor;
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
    void removeChannel(Channel* channel);

    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);
    void wakeup();

    void runAt(const TimeStamp& time, const TimerCallback& cb);
    void runAfter(double delay, const TimerCallback& cb);
    void runEvery(double interval, const TimerCallback& cb);

private:
    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();
    int createEventfd();

    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    const std::thread::id threadId_;
    bool quit_;
    bool callingPendingFunctors_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    // std::unique_ptr<Epoller> poller_;
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;
    mutable std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
    std::unique_ptr<TimerQueue> timerQueue_;

};



}
}

#endif