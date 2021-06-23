#ifndef SERVER_NET_ACCEPTOR_H
#define SERVER_NET_ACCEPTOR_H

#include "base/noncopyable.h"
#include <functional>
#include <sys/socket.h>
#include "net/inetaddress.h"
#include "net/eventloop.h"
#include "net/channel.h"
#include "net/Socket.h"
namespace server
{
namespace net
{


class Acceptor : noncopyable
{
public:
    typedef std::function<void (int sockfd, 
                        const InetAddress& addr)> NewConnectCallback;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr);
    ~Acceptor();

    bool isListenning() const { return listenning_; }
    void listen();
    void setNewConnectCallback(const NewConnectCallback& cb) { newConnectCallback_ = cb; }

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectCallback newConnectCallback_;
    bool listenning_;

};

}
}

#endif