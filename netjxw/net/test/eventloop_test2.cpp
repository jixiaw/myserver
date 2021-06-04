#include "net/eventloop.h"
#include "net/channel.h"
#include <string.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/syscall.h>
using namespace server::net;

pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

int flag = 0;

void threadFunc()
{
    printf("pid = %d, tid = %d\n", getpid(), gettid());
    EventLoop loop;
    loop.loop();
}

EventLoop* g_loop;
void threadFunc2()
{
    printf("pid = %d, tid = %d\n", getpid(), gettid());
    g_loop->loop();
}

void run4()
{
    printf("run4: pid = %d, flag = %d\n", getpid(), flag);
    g_loop->quit();

}

void run3()
{
    printf("run3: pid = %d, flag = %d\n", getpid(), flag);
    g_loop->runInLoop(run4);
    flag = 3;
}

void run2()
{
    printf("run2: pid = %d, flag = %d\n", getpid(), flag);
    g_loop->queueInLoop(run3);
}

void run1()
{
    flag = 1;
    printf("run1: pid = %d, flag = %d\n", getpid(), flag);
    g_loop->runInLoop(run2);
    flag = 2;
}

void timeout() 
{
    printf("Timeout\n");
    g_loop->quit();
}

int main()
{
    printf("pid = %d, tid = %d\n", getpid(), gettid());
    EventLoop loop;
    g_loop = &loop;

    // std::thread th(threadFunc2);
    // th.join();

    int timefd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timefd);
    channel.setReadCallBack(run1);
    channel.enableReading();
    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 3;
    ::timerfd_settime(timefd, 0, &howlong, NULL);

    loop.loop();
    ::close(timefd);
    printf("main: pid = %d, flag = %d\n", getpid(), flag);
    return 0;
}