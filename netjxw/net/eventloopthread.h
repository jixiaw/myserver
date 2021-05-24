#ifndef SERVER_NET_EVENTLOOP_THREAD_H
#define SERVER_NET_EVENTLOOP_THREAD_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include "net/eventloop.h"
#include "base/noncopyable.h"
namespace server
{
namespace net 
{

class EventLoopThread : noncopyable 
{
public:
    EventLoopThread(const std::string& name="EventLoopThread");
    ~EventLoopThread();

    EventLoop* startLoop();
    void threadFunc();

    std::string getName() { return name_; }

private:
    EventLoop* loop_;
    std::string name_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable condition_;

};


}
}

#endif