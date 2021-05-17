#include "tcpserver.h"
#include "acceptor.h"

using namespace server::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name)
: loop_(loop),
  localAddr_(listenAddr),
  acceptorPtr(new Acceptor(loop, listenAddr)),
  start_(false),
  nextConnId_(0),
  name_(name)
{
    acceptorPtr->setNewConnectCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    
}

void TcpServer::start()
{
    if (!start_){
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptorPtr.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peeraddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    std::cout<<"TcpServer::newConnection [" << name_ <<"] new connection [" <<connName
            << "] from " <<peeraddr.toString() <<std::endl;
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr_, peeraddr));
    connectionMap_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->connectEstablish();
}
