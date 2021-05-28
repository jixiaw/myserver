#include "connector.h"
#include "eventloop.h"
#include "channel.h"
#include "Socket.h"
#include "base/logging.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
using namespace server::net;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
: loop_(loop),
  serverAddr_(serverAddr),
  connect_(false)
{
    
}

Connector::~Connector()
{
    assert(!channel_);
}

void Connector::start() 
{
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    int sockfd = Socket::createNonblockingSocket();
    const struct sockaddr_in* addr = serverAddr_.getSockAddr();
    int ret = ::connect(sockfd, 
        // static_cast<const struct sockaddr*>((const void*)serverAddr_.getSockAddr()), 
        (struct sockaddr*)addr,
        static_cast<socklen_t>(sizeof *serverAddr_.getSockAddr()));
    // sockfd是非阻塞的，需要进一步错误处理
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno) {
        case 0:
        case EINPROGRESS:   // 连接请求还没有完成
        case EINTR:         // 系统调用由于捕获中断而中止
        case EISCONN:       // 已经连接到socket
            connecting(sockfd); // 继续连接，对sockfd添加可写事件，如果可写则连接成功
            break;
        default:
            ::close(sockfd);
            LOG_ERROR << "Connector::startInLoop() connect error: "<< savedErrno; 
    }
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    connect_ = true;
    startInLoop();
}

void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    int sockfd = removeChannel();
    ::close(sockfd);
}

void Connector::handleError()
{
    LOG_ERROR << "Connector::handleError()";
}

void Connector::handleWrite()
{
    int sockfd = removeChannel();
    // fix: add error handle
    if (connect_) {
        connCallback_(sockfd);
    } else {
        ::close(sockfd);
    }
}

int Connector::removeChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    // 在channel::handleEvent里面不能直接析构channel
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}

void Connector::connecting(int sockfd)
{
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallBack(std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallBack(std::bind(&Connector::handleError, this));
    channel_->enableWriting();
}