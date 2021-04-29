#include "net/eventloop.h"
#include "net/channel.h"
#include <string.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <sys/timerfd.h>
using namespace server::net;

void threadFunc()
{
    printf("pid = %d, tid = %lld\n", getpid(), std::this_thread::get_id());
    EventLoop loop;
    loop.loop();
}

EventLoop* g_loop;
void threadFunc2()
{
    g_loop->loop();
}

void timeout() 
{
    printf("Timeout\n");
    g_loop->quit();
}

void writestd()
{
    char temp[100];
    read(0, temp, 99);
    write(1, "test std write\n", 15);
}
int main()
{
    printf("pid = %d, tid = %lld\n", getpid(), std::this_thread::get_id());
    EventLoop loop;
    g_loop = &loop;
    #if 0
    {    
        std::thread th(threadFunc2);
        th.join();
        loop.loop();
    }
    #endif
    int timefd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timefd);
    channel.setReadCallBack(timeout);
    channel.enableReading();
    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 20;
    ::timerfd_settime(timefd, 0, &howlong, NULL);

    Channel ch(&loop, 0);
    ch.setReadCallBack(writestd);
    ch.enableReading();
    loop.loop();
    ::close(timefd);

    return 0;
}