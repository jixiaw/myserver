#ifndef SERVER_NET_TCPSERVER_H
#define SERVER_NET_TCPSERVER_H

#include <functional>
#include <memory>
#include <map>
#include <string>

#include "base/noncopyable.h"
#include "net/inetaddress.h"
#include "net/tcpconnection.h"

namespace server {
namespace net {

class EventLoop;
class Channel;
class Acceptor;
class TcpConnection;

// 用户持有，一个TCP服务，包含Acceptor用来接受连接，保存当前建立的连接
class TcpServer : noncopyable
{
public:
    typedef std::function<void()> ConnectionCallback;
    typedef std::function<void()> MessageCallback;
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name);
    ~TcpServer();

    void start();  // 开启监听
    void setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = cb;};
    void setMessageCallback(const MessageCallback& cb) {messageCallback_ = cb;};

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
private:
    // int sockfd_;
    InetAddress localAddr_;
    EventLoop* loop_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptorPtr;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    bool start_;
    int nextConnId_;
    ConnectionMap connectionMap_;
};

}
}

#endif