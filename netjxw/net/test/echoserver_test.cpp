#include "net/eventloop.h"
#include "net/inetaddress.h"
#include "net/tcpserver.h"
#include "net/buffer.h"
#include <iostream>
#include <string>
using namespace server::net;
using namespace std;
class EchoServer{
public:
    EchoServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr, "echo server")
    {
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer)
    {
        string msg(buffer->retrieveAllString());
        cout<<conn->getName()<<" recv "<<msg.size()<<"bytes."<<endl;
        conn->send(msg);
    }
    void start()
    {
        server_.start();
    }


private:
    EventLoop* loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress listenaddr(1234);
    EchoServer server(&loop, listenaddr);
    server.start();
    loop.loop();
    return 0;
}