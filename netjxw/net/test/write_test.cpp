#include "net/eventloop.h"
#include "net/tcpserver.h"
#include "net/tcpconnection.h"
#include "net/inetaddress.h"
#include "net/buffer.h"
#include "base/logging.h"

using namespace std;
using namespace server::net;
using namespace server;

void onCompleteWrite(const TcpConnectionPtr& conn)
{
    printf("onCompleteWrite\n");
    conn->startRead();

}

void onHighWater(const TcpConnectionPtr& conn, size_t mark)
{
    printf("onHighWater: %ld\n", mark);
    conn->stopRead();
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buffer)
{
    string msg(buffer->retrieveAllString());

    cout<<conn->getName()<<" recv "<<msg.size()<<"bytes. size of buffer: "<<buffer->size()<<endl;
    conn->send(msg);
    // conn->shutdown();
}

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()){
        printf("onConnection: new connection [%s] from %s\n",
            conn->getName().c_str(),
            conn->getPeerAddr().toString().c_str());
        conn->setTcpNoDelay(true);
        conn->setKeepAlive(true);
        conn->setWriteCompleteCallback(onCompleteWrite);
        conn->setHighWaterMarkCallback(onHighWater, 512*1024);
    } else {
        printf("onConnection: dis connection [%s] from %s\n",
            conn->getName().c_str(),
            conn->getPeerAddr().toString().c_str());
    }
}


int main(int argc, char* argv[])
{
    Logger::setLogLevel(Logger::TRACE);
    EventLoop loop;
    int port = 1234;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    printf("listen in %d\n", port);
    TcpServer server_(&loop, InetAddress(port), "test write");
    server_.setConnectionCallback(onConnection);
    server_.setMessageCallback(onMessage);
    server_.start();
    loop.loop();
    return 0;
}
