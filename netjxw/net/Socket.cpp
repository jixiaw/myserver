#include "Socket.h"
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
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
    std::cout<<"addrlen: "<<addrlen<<std::endl;
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
        std::cout<<"Socket listen: ."<<ret<<std::endl;
    }
    std::cout<<"start listenning"<<std::endl;
}

void Socket::bind(const InetAddress& addr)
{
    int ret = ::bind(sockfd_, (struct sockaddr *)addr.getSockAddr(), 
                    sizeof(struct sockaddr));
    if (ret < 0)
    {
        std::cout<<"Error bind socket: "<<ret<<std::endl;
    }
}

int Socket::createNonblockingSocket() {
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (fd <= 0) {
        std::cout<<"Create nonblock socket Error."<<std::endl;
    }
    std::cout<< "socket: "<<fd<<std::endl;
    return fd;

}