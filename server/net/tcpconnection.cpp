#include "tcpconnection.h"
#include "Socket.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include "base/logging.h"
#include <memory>
#include <unistd.h>

using namespace server::net;

void server::net::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        printf("defaultConnectionCallback(): new connection [%s] from %s\n",
                conn->getName().c_str(),
                conn->getPeerAddr().toString().c_str());
    } else {
        printf("defaultConnectionCallback(): connection [%s] is down.\n",
        conn->getName().c_str());
    }
}

void server::net::defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer)
{
    printf("defaultMessageCallback(): received %zd bytes from connection [%s]\n",
            buffer->readableBytes(), conn->getName().c_str());
    printf("defaultMessageCallback(): [%s]\n", buffer->retrieveAllString().c_str());
}

TcpConnection::TcpConnection(EventLoop* loop, std::string& name, int sockfd, 
                    const InetAddress& localAddr, const InetAddress& peerAddr,
                    bool epollET)
: loop_(loop),
  name_(name),
  state_(kConnecting),
  reading_(true),
  epollET_(epollET),
  socket_(new Socket(sockfd)),
  channel_(new Channel(loop, sockfd)),
  localAddr_(localAddr),
  peerAddr_(peerAddr),
  highWaterMark_(64*1024*1024),
  pContext_(NULL)
{
    channel_->setReadCallBack(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallBack(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallBack(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallBack(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConection deconstruct";
}

void TcpConnection::connectEstablish()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    if (epollET_) {
        channel_->enableETReading();
    } else {
        channel_->enableReading();
    }
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    // setState(kDisconnected);
    // channel_->disableAll();
    // connectionCallback_(shared_from_this());
    channel_->remove();
}

void TcpConnection::handleRead()
{
    // char buf[4096];
    // ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
    ssize_t n;
    bool flag = false;
    if (channel_->isETMode()) {
        n = inputBuffer_.readLoop(channel_->fd(), flag);
    } else {
        n = inputBuffer_.readFd(channel_->fd());
    }
    LOG_DEBUG << "TcpConnection::handleRead() read " << n <<"bytes.";
    if (n > 0){
        messageCallback_(shared_from_this(), &inputBuffer_);
        if (flag) {
            // close wait状态
            loop_->queueInLoop(std::bind(&TcpConnection::handleClose, this));
        }
    } else if (n == 0) {
        handleClose();
    } else {
        LOG_ERROR << "ERROR in TcpConnection::handleRead().";
        handleError();
    }

}
void TcpConnection::handleWrite()
{
    LOG_TRACE << "TcpConnection::handleWrite()";
    loop_->assertInLoopThread();
    // 有些channel可能已经关闭了
    if (channel_->isWriting()){
        ssize_t n;
        if (channel_->isETMode()) {
            n = outputBuffer_.writeLoop(channel_->fd());
        } else {
            n = outputBuffer_.writeFd(channel_->fd());
        }
        LOG_DEBUG << "TcpConnection::handleWrite() write size " << n;
        if (n > 0) {
            // outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                LOG_DEBUG << "TcpConnection::handleWrite() disablewrite.";
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            } else {
                LOG_DEBUG << "TcpConnection::handleWrite() remaining size " << outputBuffer_.readableBytes();
            }
        }
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    LOG_TRACE << "TcpConnection::handleClose().";
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr thr(shared_from_this());
    connectionCallback_(thr);
    closeCallback_(thr);  // 回调让TcpServer删除这个连接
}

void TcpConnection::handleError()
{
    int err = Socket::getSocketError(channel_->fd());
    char buf[512];
    LOG_ERROR << "TcpConnection::handleError [" << name_ 
              << "] - socket error = " << err <<" "<< strerror_r(err, buf, sizeof buf);
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
                std::bind((void (TcpConnection::*)(const std::string&))&TcpConnection::sendInLoop, 
                           this, message));
        }
    }
}

void TcpConnection::send(const char* message, size_t len)
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message, len);
        } else {
            // 学习一下绑定重载函数
            loop_->runInLoop(
                std::bind((void (TcpConnection::*)(const char*, size_t))&TcpConnection::sendInLoop, 
                           this, message, len));
        }
    }
}

void TcpConnection::send(Buffer* buffer)
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buffer->peek(), buffer->readableBytes());
            buffer->retrieveAll();
        } else {
            loop_->runInLoop(
                std::bind((void (TcpConnection::*)(const std::string&))&TcpConnection::sendInLoop,
                           this, buffer->retrieveAllString()));
        }
    }
}

void TcpConnection::sendInLoop(const char* message, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // 如果缓冲区没有等待写的就直接写入
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message, len);
        if (nwrote >= 0) {
            if (static_cast<size_t>(nwrote) < len) {
                LOG_INFO << "TcpConnection::sendInLoop() There is more data to write.";
            } else if (writeCompleteCallback_) {
                LOG_INFO << "TcpConnection::sendInLoop() all data wrote: " << nwrote;
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            LOG_ERROR << "Error["<<errno<<"] write in TcpConnection::sendInLoop.";
        }
    }
    // 没有全部写进去，监听写事件
    if (static_cast<size_t>(nwrote) < len) {
        outputBuffer_.append(message + nwrote, len-nwrote);
        size_t len = outputBuffer_.readableBytes();
        if (highWaterMarkCallback_ && len >= highWaterMark_) {
            loop_->queueInLoop(
                std::bind(highWaterMarkCallback_, shared_from_this(), len));
        }
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    } else {
        LOG_DEBUG << "TcpConnection::sendInLoop() send all completely.";
    }
    LOG_DEBUG << "TcpConnection::sendInLoop() buffer: "<<outputBuffer_.readableBytes();
}

void TcpConnection::sendInLoop(const std::string& message)
{
    loop_->assertInLoopThread();
    sendInLoop(message.c_str(), message.size());
}

void TcpConnection::setTcpNoDelay(bool on) 
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::setKeepAlive(bool on)
{
    socket_->setKeepAlive(on);
}

void TcpConnection::startRead() 
{
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading()) {
        LOG_DEBUG << "TcpConnection::startReadInLoop()";
        reading_ = true;
        channel_->enableReading();
    }
}

void TcpConnection::stopRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading()) {
        LOG_DEBUG << "TcpConnection::stopReadInLoop()";
        reading_ = false;
        channel_->disableReading();
    }
}