#include "inetaddress.h"
#include <string.h>
#include "arpa/inet.h"
#include <iostream>
using namespace server::net;

InetAddress::InetAddress(uint16_t port)
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

InetAddress::InetAddress(const struct sockaddr_in& addr)
: addr_(addr)
{
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) 
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
        std::cout<<"error from stringip to ip"<<std::endl;
    }
}

uint16_t InetAddress::port() const
{
    return ntohs(addr_.sin_port); 
}

const struct sockaddr_in* InetAddress::getSockAddr() const {
    return &addr_;
}

void InetAddress::setSockAddr(const struct sockaddr_in& addr)
{
    addr_ = addr;
}

std::string InetAddress::toIp() const 
{
    char buf[32];
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, 32);
    return buf;
}

std::string InetAddress::toString() const 
{
    std::string ip = toIp();
    return ip + ":" + std::to_string(port());
}
