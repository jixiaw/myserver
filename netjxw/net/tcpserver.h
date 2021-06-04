#ifndef SERVER_NET_TCPSERVER_H
#define SERVER_NET_TCPSERVER_H

#include <functional>
#include <memory>
#include <map>
#include <string>

#include "net/common.h"
#include "base/noncopyable.h"
#include "net/inetaddress.h"
#include "net/tcpconnection.h"

namespace server {
namespace net {

class EventLoop;
class Channel;
class Acceptor;
class TcpConnection;
class EventLoopThreadPool;

// 用户持有，一个TCP服务，包含Acceptor用来接受连接，保存当前建立的连接
class TcpServer : noncopyable
{
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name);
    ~TcpServer();

    void start();  // 开启监听
    void setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = cb;};
    void setMessageCallback(const MessageCallback& cb) {messageCallback_ = cb;};

    void setNumThread(int numThread);

    EventLoop* getLoop() const { return loop_; }
    const std::string& getName() const { return name_; }
    const std::string& getIpPort() const { return ipPort_; }

private:
    // Acceptor accept 连接时回调这个函数建立连接
    void newConnection(int sockfd, const InetAddress& peerAddr);
    // TcpConnection 关闭连接时回调这个函数删除连接
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
private:
    // int sockfd_;
    EventLoop* loop_;
    InetAddress localAddr_;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptorPtr;
    ConnectionCallback connectionCallback_;  // 连接建立和断开时会调用，TcpConnection::connectEstablish和connectDestory
    MessageCallback messageCallback_;  // TcpConnection::handleRead调用
    // CloseCallback closeCallback_;  // 断开连接时调用， TcpConnection::handleClose里
    bool start_;
    int nextConnId_;
    ConnectionMap connectionMap_;
    // 新建连接后从线程池中取出一个event线程来管理
    std::unique_ptr<EventLoopThreadPool> threadPool_;
};

}
}

#endif