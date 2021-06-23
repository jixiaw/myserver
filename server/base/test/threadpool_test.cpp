#include "base/threadpool.h"
#include <stdio.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
using namespace server;
using namespace std;
ThreadPool* g_pool;
pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

class T {
public:
    void task3(int a) {
        printf("%d: T::test3 %d\n", gettid(), a);
    }
};

T a;

void task2(int x) {
    printf("%d: test2 %d\n", gettid(), x);
    g_pool->run(std::bind(&T::task3, &a, 2));
}

void task1() {
    printf("%d: test1\n", gettid());
    g_pool->run(std::bind(&task2, 1));
}



int main()
{
    ThreadPool pool(4, "threadpool");
    g_pool = &pool;
    pool.start();
    pool.run(task1);
    for (int i = 0; i < 10; ++i) {
        pool.run(std::bind(&task2, i));
    }
    sleep(3);
    pool.stop();
    return 0;
}