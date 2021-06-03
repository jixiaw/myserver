#ifndef SERVER_BASE_THREADPOOL_H
#define SERVER_BASE_THREADPOOL_H
#include <thread>
#include <condition_variable>
#include <vector>
#include <list>
#include <memory>
#include <functional>
#include "logging.h"
#include "noncopyable.h"

namespace server 
{
// 用于计算的线程池
class ThreadPool : noncopyable
{
public:
    typedef std::function<void()> Task;
    ThreadPool(int numThread, const std::string& name);
    ~ThreadPool();

public:
    void start();
    void run(const Task& task);
    void stop();

public:
    const std::string& getName() const { return name_; }
    int getThreadNum() const { return numThread_; }
    void setThreadNum(int numThread) { numThread_ = numThread; } 

private:
    void pushTask(const Task& task);
    Task popTask();
    void runInThread();

private:
    int numThread_;
    const std::string name_;
    volatile bool isRun_;
    std::vector<std::unique_ptr<std::thread>> threadVec_;  // 线程
    std::list<Task> taskQueue_;     // 任务队列 fix: 设置最大任务队列长度
    std::mutex mutex_;
    std::condition_variable cond_;
};
}

#endif
