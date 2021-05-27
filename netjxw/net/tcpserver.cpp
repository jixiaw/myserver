#include "tcpserver.h"
#include "acceptor.h"
#include "buffer.h"
#include "base/logging.h"
#include "eventloopthreadpool.h"
using namespace server::net;


TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name)
: loop_(loop),
  localAddr_(listenAddr),
  acceptorPtr(new Acceptor(loop, listenAddr)),
  start_(false),
  nextConnId_(0),
  name_(name),
  connectionCallback_(server::net::defaultConnectionCallback),
  messageCallback_(server::net::defaultMessageCallback),
  closeCallback_(server::net::defaultCloseCallback),
  threadPool_(new EventLoopThreadPool(loop))
{
    acceptorPtr->setNewConnectCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    
}

void TcpServer::setNumThread(int numThread)
{
    threadPool_->setNumThreads(numThread);
}

void TcpServer::start()
{
    if (!start_){
        assert(!acceptorPtr->isListenning());
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptorPtr.get()));
        threadPool_->start();
        start_ = true;
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peeraddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peeraddr.toString().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO << "TcpServer::newConnection [" << name_ <<"] new connection [" << connName
             << "] from " << peeraddr.toString();
    EventLoop* loop = threadPool_->getLoop();
    TcpConnectionPtr conn(new TcpConnection(loop, connName, sockfd, localAddr_, peeraddr));
    connectionMap_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    loop->runInLoop(std::bind(&TcpConnection::connectEstablish, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnection [" << name_ << "] - connection [" 
             << conn->getName() << "]";
    size_t n = connectionMap_.erase(conn->getName());
    assert(n == 1);
    EventLoop* loop = conn->getLoop();
    // TcpConnection 最后调用的函数，调用完后，这个Functor析构后，这个TcpConnection才析构
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

