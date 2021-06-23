# 网络库
用C++在Linux平台开发的TCP网络库，是非阻塞IO的Reactor网络库，整体框架为one loop per thread + thread pool, 主要参考muduo网络库，比muduo简单。在此基础上添加了epoll的边缘触发，完善http报文头解析等。

## 使用
C++11即可编译，无需装任何库
```
cd myserver
mkdir build && cd build
cmake ..
make
```

## examples

### echo server
echo 服务器，可以用作pingpong测试服务器

### http server
静态http服务器

### file http downloader
文件下载http服务器，参考 python 的 http.server


## 遇到的问题
1. 对于边缘触发所有文件描述法不用说肯定要设置成非阻塞，水平触发也最好都设置成非阻塞，虽然epoll得到的肯定有事件发生，但是可能哪里代码写错了突然阻塞了，然后查了半天发现是这个问题。
2. 错误回调函数很重要，socket在对方发送rst包后会有一个POLLHUP事件，这时可以调用错误回调函数关闭即可。
3. 压测，10000并发时客户端出现大量104错误：Connection reset by peer。有一个原因是listen的backlog设置太小，增大这个值即可。同时可以使用while循环accept连接。
4. 压测，epoll 边缘触发，结束时出现大量连接未断开，即服务端出现大量close_wait。原因是循环读取数据读到了0字节（对方断开连接），而没有处理。解决方法：添加读到0字节的处理，这就又出来一个问题了，这时需要处理之前收到的数据吗？目前的实现里先处理再close，但是其实可以直接close，因为一般half-close connection很少用到，muduo也不支持对端断开后还向对端发送数据，可见 https://github.com/chenshuo/muduo/issues/366
