#include "tcpserver.h"
#include "acceptor.h"
#include "buffer.h"
using namespace server::net;

void server::net::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        printf("defaultConnectionCallback(): new connection [%s] from %s\n",
                conn->getName().c_str(),
                conn->getPeerAddr().toString().c_str());
    } else {
        printf("defaultConnectionCallback(): connection [%s] is down.\n",
        conn->getName().c_str());
    }
}

void server::net::defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer)
{
    printf("defaultMessageCallback(): received %zd bytes from connection [%s]\n",
            buffer->readableBytes(), conn->getName().c_str());
    printf("defaultMessageCallback(): [%s]\n", buffer->retrieveAll().c_str());
}

void server::net::defaultCloseCallback(const TcpConnectionPtr& conn)
{
    printf("defaultCloseCallback(): connection [%s] closed.",
            conn->getName().c_str());
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name)
: loop_(loop),
  localAddr_(listenAddr),
  acceptorPtr(new Acceptor(loop, listenAddr)),
  start_(false),
  nextConnId_(0),
  name_(name),
  connectionCallback_(server::net::defaultConnectionCallback),
  messageCallback_(server::net::defaultMessageCallback),
  closeCallback_(server::net::defaultCloseCallback)
{
    acceptorPtr->setNewConnectCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    
}

void TcpServer::start()
{
    if (!start_){
        assert(!acceptorPtr->isListenning());
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptorPtr.get()));
        start_ = true;
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
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    conn->connectEstablish();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    printf("TcpServer::removeConnection [%s] - connection [%s]\n",
            name_.c_str(), conn->getName().c_str());
    size_t n = connectionMap_.erase(conn->getName());
    assert(n == 1);
    // TcpConnection 最后调用的函数，调用完后，这个Functor析构后，这个TcpConnection才析构
    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

