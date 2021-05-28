#include "net/tcpserver.h"
#include "net/eventloop.h"
#include "net/inetaddress.h"
#include <string>
#include <unistd.h>
using namespace server::net;
using namespace std;

class EchoServer
{
public:
    EchoServer(EventLoop* loop, const InetAddress& listenAddr, int numThread=4)
    :loop_(loop),
     server_(loop, listenAddr, "echo server thread pool"),
     numThread_(numThread)
    {
        server_.setNumThread(numThread);
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer)
    {
        string msg(buffer->retrieveAllString());
        // cout<<conn->getName()<<" recv "<<msg.size()<<"bytes."<<endl;
        conn->send(msg);
    }
    void onConnection(const TcpConnectionPtr& conn)
    {
        if (conn->connected()){
            // printf("onConnection: new connection [%s] from %s\n",
            //     conn->getName().c_str(),
            //     conn->getPeerAddr().toString().c_str());
            // if (sleepSeconds_ > 0) {
            //     ::sleep(sleepSeconds_);
            // }
            // conn->send("hello\n");
            // conn->send("echo server\n");
            // conn->shutdown();
            conn->setTcpNoDelay(true);
        }
    }
    void start() {
        server_.start();
    }
private:
    EventLoop* loop_;
    TcpServer server_;
    int numThread_;

};

int main(int argc, char* argv[])
{
    cout<<"pid = "<<getpid()<<endl;
    if (argc > 1) {
        EventLoop loop;
        InetAddress listenaddr(stoi(argv[1]));
        int numThread = 0;
        if (argc > 2) {
            numThread = stoi(argv[2]);
        }
        EchoServer server(&loop, listenaddr, numThread);
        cout<<"listening in "<<listenaddr.toString()<<endl;
        server.start();
        loop.loop();
    }
    else {
        EventLoop loop;
        InetAddress listenaddr(1234);
        EchoServer server(&loop, listenaddr);
        cout<<"listening in "<<listenaddr.toString()<<endl;
        server.start();
        loop.loop();
    }
    return 0;
}