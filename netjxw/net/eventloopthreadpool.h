#ifndef SERVER_NET_EVENTLOOP_THREADPOOL_H
#define SERVER_NET_EVENTLOOP_THREADPOOL_H

#include <memory>
#include <vector>
#include <string>
#include "base/noncopyable.h"

namespace server {
namespace net {
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable 
{
public:
    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name="EventLoopThreadPool");
    ~EventLoopThreadPool();

    void setNumThreads(int numThreads);
    void start();
    EventLoop* getLoop();
    std::string getName() { return name_; }

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool start_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};
}
}
#endif