#include "net/eventloop.h"
#include "net/inetaddress.h"
#include "net/tcpserver.h"
#include "net/buffer.h"
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <iostream>
#include <string>
using namespace server::net;
using namespace std;
class EchoServer{
public:
    EchoServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr, "echo server"),
      sleepSeconds_(5)
    {
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer)
    {
        string msg(buffer->retrieveAllString());
        cout<<conn->getName()<<" recv "<<msg.size()<<"bytes."<<endl;
        conn->send(msg);
    }

    void onConnection(const TcpConnectionPtr& conn)
    {
        if (conn->connected()){
            printf("onConnection: new connection [%s] from %s\n",
                conn->getName().c_str(),
                conn->getPeerAddr().toString().c_str());
            // if (sleepSeconds_ > 0) {
            //     ::sleep(sleepSeconds_);
            // }
            // conn->send("hello\n");
            // conn->send("echo server\n");
            // conn->shutdown();
            conn->setTcpNoDelay(true);
            conn->setKeepAlive(true);
        }
    }

    void start()
    {
        server_.start();
    }


private:
    EventLoop* loop_;
    TcpServer server_;
    int sleepSeconds_;
};

int main()
{
    // ::signal(SIGPIPE, SIG_IGN);
    pid_t pid = ::getpid();
    cout<<"pid: "<<pid<<endl;
    EventLoop loop;
    InetAddress listenaddr(1234);
    EchoServer server(&loop, listenaddr);
    server.start();
    loop.loop();
    return 0;
}