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
    // int connfd = ::accept(sockfd_, (struct sockaddr*)&sockaddr, &addrlen);
    int connfd = ::accept4(sockfd_, (struct sockaddr*)&sockaddr, &addrlen, SOCK_CLOEXEC | SOCK_NONBLOCK);
    if (connfd >= 0) {
        addr->setSockAddr(sockaddr);
    }
    return connfd;
}

void Socket::listen()
{
    int ret;
    ret = ::listen(sockfd_, 1024);
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

struct sockaddr_in Socket::getLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, (struct sockaddr*)(&localaddr), &addrlen) < 0) {
        LOG_ERROR << "Socket::getLocalAddr";
    }
    return localaddr;
}

struct sockaddr_in Socket::getPeerAddr(int sockfd)
{
    struct sockaddr_in peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, (struct sockaddr*)(&peeraddr), &addrlen) < 0) {
        LOG_ERROR << "Socket::getPeerAddr";
    }
    return peeraddr;
}

int Socket::getSocketError(int sockfd) 
{
    int opt;
    socklen_t optlen = static_cast<socklen_t>(sizeof opt);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &opt, &optlen) < 0) {
        return errno;
    } else {
        return opt;
    }
}

void Socket::setReuseAddr(bool on)
{
    int opt = on ? 1: 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
}
void Socket::setReusePort(bool on)
{
    int opt = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
    if (ret < 0 && on) {
        LOG_ERROR << "Socket::setReuseAddr() SO_REUSEPORT failed.";
    }
}

