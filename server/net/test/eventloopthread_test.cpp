#include "net/eventloopthread.h"
#include <thread>
#include <unistd.h>
using namespace server::net;

void fun1()
{
    printf("test thread: %d\n", getpid());
}


int main()
{
    EventLoopThread loopthread;
    EventLoop* loop = loopthread.startLoop();
    loop->runInLoop(fun1);
    return 0;
}