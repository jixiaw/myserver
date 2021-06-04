#include "net/tcpclient.h"
#include "net/eventloop.h"
#include "net/inetaddress.h"
#include "net/buffer.h"
#include "base/logging.h"
#include <stdio.h>

using namespace std;
using namespace server::net;
using namespace server;

EventLoop* g_loop;

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        printf("onConnection(): connected\n");
        conn->send("hello server");
    } else {
        printf("onConnection(): disconnected\n");
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buffer)
{
    string msg = buffer->retrieveAllString();
    printf("onMessage() recv: %s\n", msg.c_str());
    conn->shutdown();
    // g_loop->quit();
}

int main()
{
    Logger::setLogLevel(Logger::DEBUG);
    EventLoop loop;
    g_loop = &loop;
    InetAddress peeraddr("127.0.0.1", 1234);
    TcpClient client(&loop, peeraddr, "test tcpclient");
    client.setConnectionCallback(std::bind(&onConnection, std::placeholders::_1));
    client.setMessageCallback(std::bind(&onMessage, std::placeholders::_1, std::placeholders::_2));
    client.connect();
    loop.loop();
    return 0;
}