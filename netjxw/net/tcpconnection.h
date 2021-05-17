#ifndef SERVER_NET_TCPCONNECTION_H
#define SERVER_NET_TCPCONNECTION_H

#include <functional>
#include <memory>
#include <map>
#include <string>

#include "base/noncopyable.h"
#include "net/inetaddress.h"

namespace server {
namespace net {

class EventLoop;
class Channel;
class Acceptor;

class TcpConnection{
public:
    typedef std::function<void()> ConnectionCallback;
    typedef std::function<void()> MessageCallback;
    TcpConnection(EventLoop* loop, std::string& name, int sockfd, 
                    const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    void connectEstablish();

    void setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = cb;};
    void setMessageCallback(const MessageCallback& cb) {messageCallback_ = cb;};
private:
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}
}

#endif