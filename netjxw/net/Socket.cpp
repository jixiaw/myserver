#include "Socket.h"
#include "base/logging.h"
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <netinet/tcp.h>
using namespace server::net;

Socket::~Socket()
{
    close(sockfd_);
}

int Socket::accept(InetAddress* addr)
{
    struct sockaddr_in sockaddr;
    bzero(&sockaddr, sizeof sockaddr);
    socklen_t addrlen = sizeof(sockaddr);
    int connfd = ::accept(sockfd_, (struct sockaddr*)&sockaddr, &addrlen);
    if (connfd >= 0) {
        addr->setSockAddr(sockaddr);
    }
    return connfd;
}

void Socket::listen()
{
    int ret;
    ret = ::listen(sockfd_, 5);
    if (0 != ret) {
        LOG_ERROR << "Socket::listen() error: " << ret;
    }
    LOG_INFO << "Socket::listen() start listenning";
}

void Socket::bind(const InetAddress& addr)
{
    int ret = ::bind(sockfd_, (struct sockaddr *)addr.getSockAddr(), 
                    sizeof(struct sockaddr));
    if (ret < 0)
    {
        LOG_ERROR << "Socket::bind() Error bind socket: "<< ret;
    }
}

int Socket::createNonblockingSocket() {
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (fd <= 0) {
        LOG_ERROR << "Socket::createNonblockingSocket() Create nonblock socket Error.";
    }
    LOG_INFO << "Socket::createNonblockingSocket() create socket: " << fd;
    return fd;
}

void Socket::shutdownWrite()
{
    int rst;
    if ((rst = ::shutdown(sockfd_, SHUT_WR)) < 0) {
        LOG_ERROR << "Socket::shutdownWrite error";
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof opt);
}

void Socket::setKeepAlive(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof opt);
}