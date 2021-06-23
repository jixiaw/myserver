#ifndef SERVER_NET_COMMON_H
#define SERVER_NET_COMMON_H

#include <functional>
#include <memory>
namespace server 
{
namespace net
{
class Buffer;
class TcpConnection;
typedef std::function<void()> TimerCallback;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
// 连接建立后回调函数
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
// 从socket读取数据后回调函数
typedef std::function<void(const TcpConnectionPtr&, 
                            Buffer* buffer)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
// 写缓冲区清空时调用
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
// 写缓冲区到达一定容量时调用
typedef std::function<void(const TcpConnectionPtr&, 
                            size_t)> HighWaterMarkCallback;

void defaultConnectionCallback(const TcpConnectionPtr&);
void defaultMessageCallback(const TcpConnectionPtr&, 
                            Buffer* buffer);
void defaultCloseCallback(const TcpConnectionPtr&);

}
}

#endif