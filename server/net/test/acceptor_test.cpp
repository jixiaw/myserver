#include "net/acceptor.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
using namespace server::net;



int fd = Socket::createNonblockingSocket();
Socket sk(fd);
EventLoop loop;

InetAddress addr1("10.15.198.199", 9091);
InetAddress addr2("10.15.198.199", 9981);
Channel ch(&loop, fd);
Acceptor acceptor(&loop, addr1);
void fun1(int sockfd, const InetAddress& addr)
{
    std::cout<<"newconnection: "<<addr.toString()<<std::endl;
    ::write(sockfd, "hello\n", 6);
    ::close(sockfd);
    // acceptor.~Acceptor();
    // loop.quit();
}
void fun2()
{
    InetAddress clientaddr;
    int connfd = sk.accept(&clientaddr);
    std::cout<<"connect ok: "<<connfd<<std::endl;
    ::write(connfd, "hello\n", 6);
    ::close(connfd);
    ch.disableAll();
    ch.remove();
}

int main()
{
    std::cout<<"main: "<<fd<<std::endl;

    // sk.bind(addr1);
    // sk.listen();
    // InetAddress clientaddr;
    // while(1)
    // {
    //     int connfd = sk.accept(&clientaddr);
    //     std::cout<<connfd<<std::endl;
    //     sleep(1);
    //     if (connfd >= 0) break;
    // }

    // Channel ch(&loop, fd);
    // ch.setReadCallBack(fun2);
    // ch.enableReading();
    // loop.loop();

    acceptor.setNewConnectCallback(fun1);
    acceptor.listen();
    
    // Socket sk(Socket::createNonblockingSocket());
    // InetAddress add2("10.15.198.199", 9981);
    sk.bind(addr2);
    sk.listen();
    ch.setReadCallBack(fun2);
    ch.enableReading();    

    loop.loop();
    return 0;
}