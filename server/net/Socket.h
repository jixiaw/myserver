#ifndef SERVER_NET_SOCKET_H
#define SERVER_NET_SOCKET_H

#include "base/noncopyable.h"
#include "net/inetaddress.h"

namespace server {
namespace net {

class EventLoop;
class Channel;

class Socket : noncopyable
{
public:
    explicit Socket(int sockfd)
    : sockfd_(sockfd)
    { }
    ~Socket();

    int getFd() const { return sockfd_; } 
    void bind(const InetAddress& addr);
    void listen();
    int accept(InetAddress* addr);
    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);

    static int createNonblockingSocket();
    static struct sockaddr_in getLocalAddr(int sockfd);
    static struct sockaddr_in getPeerAddr(int sockfd);
    static int getSocketError(int sockfd);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
private:
    int sockfd_;
};

}
}

#endif