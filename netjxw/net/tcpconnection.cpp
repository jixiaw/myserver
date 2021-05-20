#include "tcpconnection.h"
#include "Socket.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include <memory>
#include <unistd.h>

using namespace server::net;




TcpConnection::TcpConnection(EventLoop* loop, std::string& name, int sockfd, 
                    const InetAddress& localAddr, const InetAddress& peerAddr)
: loop_(loop),
  name_(name),
  state_(kConnecting),
  socket_(new Socket(sockfd)),
  channel_(new Channel(loop, sockfd)),
  localAddr_(localAddr),
  peerAddr_(peerAddr)
{
    channel_->setReadCallBack(std::bind(&TcpConnection::handleRead, this));
}

TcpConnection::~TcpConnection()
{
    printf("TcpConection deconstruct\n");
}

void TcpConnection::connectEstablish()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnected);
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());
    loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead()
{
    char buf[4096];
    // ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
    ssize_t n = inputBuffer_.readFd(channel_->fd());
    if (n > 0){
        messageCallback_(shared_from_this(), &inputBuffer_);
    } else if (n == 0) {
        handleClose();
    } else {
        printf("ERROR in TcpConnection::handleRead().\n");
        handleError();
    }

}
void TcpConnection::handleWrite()
{

}
void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnected);
    printf("TcpConnection::handleClose.\n");
    channel_->disableAll();
    closeCallback_(shared_from_this());  // 回调让TcpServer删除这个连接
}
void TcpConnection::handleError()
{
    printf("TcpConnection::handleError.\n");
}