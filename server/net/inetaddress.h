#ifndef SERVER_NET_INETADDRESS_H
#define SERVER_NET_INETADDRESS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
namespace server{
namespace net{

class InetAddress 
{
public:
    explicit InetAddress(uint16_t port=0);
    InetAddress(const std::string& ip, uint16_t port);
    explicit InetAddress(const struct sockaddr_in& addr);

    const struct sockaddr_in* getSockAddr() const;

    void setSockAddr(const struct sockaddr_in& addr);

    std::string toIp() const;
    std::string toString() const;
    uint16_t port() const;    

private:
    struct sockaddr_in addr_;

};
}
}

#endif