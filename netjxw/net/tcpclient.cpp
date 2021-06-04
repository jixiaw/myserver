#include "tcpclient.h"
#include "connector.h"
#include "eventloop.h"
#include "base/logging.h"
#include "Socket.h"
using namespace server::net;


TcpClient::TcpClient(EventLoop* loop, 
            const InetAddress& serverAddr, 
            const std::string& name)
: loop_(loop),
  connector_(new Connector(loop, serverAddr)),
  name_(name),
  connect_(true),
  connectionCallback_(defaultConnectionCallback),
  messageCallback_(defaultMessageCallback),
  nextConnId_(0)
{
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}
TcpClient::~TcpClient()
{
    
}

void TcpClient::connect()
{
    LOG_INFO << "TcpClient::connect [" << name_ << "] - connecting to "
            << connector_->getServerAddr().toString();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;
    connection_->shutdown();
}

void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(Socket::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toString().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    InetAddress localAddr(Socket::getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(
        loop_, connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    // unsafe
    connection_ = conn;

    conn->connectEstablish();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());
    connection_.reset();
    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    
}
