#ifndef SERVER_NET_EPOLLER_H
#define SERVER_NET_EPOLLER_H

#include "base/noncopyable.h"
#include <functional>
#include "inetaddress.h"
namespace server {
namespace net {
class EventLoop;
class InetAddress;
class Connector 
{
public:
    typedef std::function<void(int fd)> NewConnectionCallback;
    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void start();
    void restart();
    void stop();
    const InetAddress& getServerAddr() const { return serverAddr_; }
    void setNewConnectionCallback(const NewConnectionCallback& cb) 
    { connCallback_ = cb; }

private:
    void startInLoop();
    void stopInLoop();
    int removeChannel();
    void resetChannel();
    void connecting(int sockfd);
    void handleWrite();
    void handleRead();
    void handleError();

private:    
    EventLoop* loop_;
    NewConnectionCallback connCallback_;
    InetAddress serverAddr_;
    bool connect_;
    std::unique_ptr<Channel> channel_;
};


}
}

#endif