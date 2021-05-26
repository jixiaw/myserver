#include "connector.h"
#include "eventloop.h"
#include "Socket.h"
#include "channel.h"
#include "base/logging.h"
#include <socket.h>
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
    int ret = ::connect(sockfd, 
        (struct sockaddr*)serverAddr_.getSockAddr(), 
        sizeof serverAddr_.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
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