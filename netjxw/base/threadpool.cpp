#include "threadpool.h"
#include <assert.h>
using namespace server;

ThreadPool::ThreadPool(int numThread, const std::string& name)
: numThread_(numThread),
  name_(name),
  isRun_(false)
{

}

ThreadPool::~ThreadPool()
{
    if (isRun_) {
        stop();
    }
}

void ThreadPool::start()
{
    if (isRun_) {
        LOG_ERROR << "ThreadPool::start() : ThreadPool ["<<name_<<"] already started.";
        return;
    }
    assert(numThread_ > 0);
    isRun_ = true;
    threadVec_.reserve(numThread_);
    for (int i = 0; i < numThread_; ++i) {
        threadVec_.emplace_back(new std::thread(
            std::bind(&ThreadPool::runInThread, this)));
    }
}

void ThreadPool::stop()
{
    if (isRun_) {
        std::unique_lock<std::mutex> lock(mutex_);
        isRun_ = false;
        cond_.notify_all();
    }
    for (auto& thread : threadVec_) {
        thread->join();
    }
}

void ThreadPool::run(const Task& task)
{
    if (!isRun_) {
        LOG_WARN << "ThreadPool::run :  ThreadPool [" << name_ <<"] is not running.";
        task();
    } else {
        std::unique_lock<std::mutex> lock(mutex_);
        taskQueue_.push_back(task);
        cond_.notify_one();
    }
}

ThreadPool::Task ThreadPool::popTask() {
    Task task;
    std::unique_lock<std::mutex> lock(mutex_);
    while (taskQueue_.empty() && isRun_) {
        cond_.wait(lock);
    }
    if (!taskQueue_.empty()) {
        task = taskQueue_.front();
        taskQueue_.pop_front();
    }
    return task;
}

void ThreadPool::runInThread()
{
    while(isRun_) {
        Task task = popTask();
        if (task) {
            task();
        }
    }
}