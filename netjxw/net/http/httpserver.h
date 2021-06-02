#ifndef SERVER_NET_HTTP_HTTPSERVER_H
#define SERVER_NET_HTTP_HTTPSERVER_H

#include "base/noncopyable.h"
#include "net/tcpserver.h"
#include <string>
namespace server {
namespace net{

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable 
{
public:
    typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpCallback;
    HttpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name);
    ~HttpServer();

    void start();
    void setThreadNum(int numThread) { 
        server_.setNumThread(numThread);
    }

    EventLoop* getLoop() const { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& cb) {
        httpCallback_ = cb;
    }

private:
    // 连接到达时回调函数
    void onConnection(const TcpConnectionPtr& conn);
    // 数据到达时回调函数，用于分析http包
    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer);
    // 整个http包分析完毕后调用，回调httpCallback,之后发送数据给客户端
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest& req);

private:
    TcpServer server_;
    // 用户实现，根据httprequest生成相应的httpresponse
    HttpCallback httpCallback_;
};
}
}

#endif