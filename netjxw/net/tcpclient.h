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
class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;
// not thread safe
class TcpClient : noncopyable
{
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    EventLoop* getLoop() const { return loop_; }
    const std::string& getName() const { return name_; }
    TcpConnectionPtr getConnection() const { return connection_; }

    void setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = cb;};
    void setMessageCallback(const MessageCallback& cb) {messageCallback_ = cb;};
    void setCloseCallback(const CloseCallback& cb) {closeCallback_ = cb;}

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

private:
    EventLoop* loop_;
    ConnectorPtr connector_;
    const std::string name_;
    ConnectionCallback connectionCallback_;  // 连接建立和断开时会调用，TcpConnection::connectEstablish和connectDestory
    MessageCallback messageCallback_;  // TcpConnection::handleRead调用
    CloseCallback closeCallback_;  // 断开连接时调用， TcpConnection::handleClose里
    bool connect_;
    TcpConnectionPtr connection_;
    int nextConnId;
};

}
}
#endif