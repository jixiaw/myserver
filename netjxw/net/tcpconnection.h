#ifndef SERVER_NET_TCPCONNECTION_H
#define SERVER_NET_TCPCONNECTION_H

#include <functional>
#include <memory>
#include <map>
#include <string>

#include "base/noncopyable.h"
#include "net/inetaddress.h"
#include "net/common.h"
#include "net/buffer.h"
namespace server {
namespace net {

class EventLoop;
class Channel;
class Acceptor;
class Socket;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop, std::string& name, int sockfd, 
                    const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    // 当TcpServer建议新连接时调用
    void connectEstablish();
    // TcpServer断开连接时调用
    void connectDestroyed();

    void setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = cb;};
    void setMessageCallback(const MessageCallback& cb) {messageCallback_ = cb;};
    void setCloseCallback(const CloseCallback& cb) {closeCallback_ = cb;}

    std::string getName(){ return name_; }
    const InetAddress& getPeerAddr() { return peerAddr_; }
    EventLoop* getLoop() { return loop_; }
    bool connected() {return state_ == kConnected;}

    void send(const std::string& message);
    void send(Buffer* buffer);
    void send(const char* message, size_t len);
    void shutdown();
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);

    void setContext(void* context) { pContext_ = context; }
    void* getContext() const { return pContext_; }

private:
    enum State {kConnecting = 0, kConnected, kDisconnected, kDisconnecting};

    void setState(State s) {state_ = s;}
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string& message);
    void sendInLoop(const char* message, size_t len);
    void shutdownInLoop();

    EventLoop* loop_;
    std::string name_;
    State state_;
    std::unique_ptr<Socket> socket_;  // connect socket
    std::unique_ptr<Channel> channel_;
    InetAddress localAddr_;     // server addr
    InetAddress peerAddr_;      // client addr
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    Buffer inputBuffer_;    // 接收数据的buffer
    Buffer outputBuffer_;   // 发送数据的buffer
    void* pContext_;
};

}
}

#endif