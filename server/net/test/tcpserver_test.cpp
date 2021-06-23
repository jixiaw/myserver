#include "net/tcpserver.h"
#include "net/inetaddress.h"
#include "net/eventloop.h"
using namespace server::net;

int main()
{
    InetAddress listenAddr(9981);
    EventLoop loop;
    TcpServer sv(&loop, listenAddr, "test");
    sv.start();
    loop.loop();
}