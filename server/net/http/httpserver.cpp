#include "httpserver.h"
#include "httpcontext.h"
#include "httpresponse.h"
#include "base/logging.h"
using namespace server::net;

HttpServer::HttpServer(EventLoop* loop, 
                       const InetAddress& listenAddr, 
                       const std::string& name,
                       bool epollET)
: server_(loop, listenAddr, name, epollET)
{    
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}

HttpServer::~HttpServer()
{

}


void HttpServer::start()
{
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        LOG_INFO << "HttpServer::onConnection from [" 
                 << conn->getPeerAddr().toString() << "].";
        // 每个tcp连接存一个HttpContext
        conn->setContext(new HttpContext());
    }
    else {
        HttpContext* context = (HttpContext*)conn->getContext();
        if (context) {
            LOG_DEBUG << "HttpServer::onConnection delete httpcontext";
            delete context;
        }
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer)
{
    // conn用了void*, 待修复
    HttpContext* context = (HttpContext*)conn->getContext();
    if (!context->parseRequest(buffer)) {
        LOG_ERROR << "HttpServer::onMessage, parseRequest error";
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
    // http请求包分析完毕
    if (context->isParseAll()) {
        onRequest(conn, context->getRequest());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    bool close = false;
    const std::string& connection = req.getHeader("Connection");
    if (connection == "close" || 
            (req.getVersion() == HttpRequest::kHttp10 
                && connection != "Keep-Alive")) {
        close = true;
        LOG_INFO << "HttpServer::onRequest() close on";
    }
    // 生成http响应
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.getCloseConnection()) {
        conn->shutdown();
    }
}
