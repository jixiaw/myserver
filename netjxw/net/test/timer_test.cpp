#include "net/eventloop.h"
#include <stdio.h>
using namespace server::net;
using namespace std;


EventLoop* g_loop;


void print()
{
    printf("test timer\n");
}
void stop()
{
    printf("stop\n");
    g_loop->quit();
}

int main()
{
    EventLoop loop;
    g_loop = &loop;
    loop.runEvery(1.0, print);
    loop.runAfter(4.5, stop);
    loop.loop();
    return 0;
}