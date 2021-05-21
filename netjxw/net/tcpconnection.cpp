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
    channel_->setWriteCallBack(std::bind(&TcpConnection::handleWrite, this));
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
    loop_->assertInLoopThread();
    if (channel_->isWriting()){
        ssize_t n = ::write(channel_->fd(), 
                    outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
    }
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

void TcpConnection::shutdown()
{
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWrite(); // 关闭写端
    }
}

void TcpConnection::send(const std::string& message)
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& message)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // 如果缓冲区没有等待写的就直接写入
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if (nwrote >= 0) {
            if (static_cast<size_t>(nwrote) < message.size()) {
                printf("There is more data to write.\n");
            }
        } else {
            nwrote = 0;
            printf("Error in TcpConnection::sendInLoop.\n");
        }
    }
    if (static_cast<size_t>(nwrote) < message.size()) {
        outputBuffer_.append(message.data() + nwrote, message.size()-nwrote);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}