#include "acceptor.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

using namespace server::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
: loop_(loop),
  acceptSocket_(Socket::createNonblockingSocket()),
  acceptChannel_(loop, acceptSocket_.getFd()),
  listenning_(false)
{
    acceptSocket_.bind(listenAddr);
    acceptChannel_.setReadCallBack(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    // std::cout<<"deconstruct acceptor"<<std::endl;
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress clientaddr;
    int connfd = acceptSocket_.accept(&clientaddr);
    if (connfd >= 0) {
        if (newConnectCallback_) {
            newConnectCallback_(connfd, clientaddr);
        } else {
            ::close(connfd);
        }
    }
}