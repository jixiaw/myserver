#ifndef SERVER_BASE_ASYNCLOGGING_H
#define SERVER_BASE_ASYNCLOGGING_H
#include <string>
#include <mutex>
#include <condition_variable>
#include <dirent.h>
#include <sys/file.h>
#include <memory>
#include <vector>
#include <assert.h>
#include <thread>
#include "base/timestamp.h"
#include "base/logfile.h"
namespace server {

class LogBuffer {
public:
  LogBuffer(size_t size): data_(size), writeIdx_(0), readIdx_(0) {}
  ~LogBuffer() {}

public:
  void append(const char* msg, size_t len) {
    if (len > writableBytes()) {
      data_.resize(data_.size() + len);
    }
    std::copy(msg, msg + len, beginWritable());
    writeIdx_ += len;
  }
  char* begin() { return &*(data_.begin()); }
  const char* begin() const { return &*(data_.begin()); }

  char* beginWritable() { return begin() + writeIdx_; }
  const char* peek() const { return begin() + readIdx_; }

  size_t writableBytes() const { return data_.size() - writeIdx_; }
  size_t readableBytes() const { return writeIdx_ - readIdx_; }
  size_t size() const { return data_.size(); }
  void retrieveAll() { readIdx_ = writeIdx_ = 0; }

private:
  std::vector<char> data_;
  size_t writeIdx_;
  size_t readIdx_;
};


class AsyncLogging {
public:
  typedef std::unique_ptr<LogBuffer> BufferPtr;
public:
  AsyncLogging(const std::string& name, const std::string& path, 
               long maxSize=10*1024*1024, size_t bufferSize=4*1024*1024)
  : name_(name), 
    path_(path), 
    maxSize_(maxSize), 
    bufferSize_(bufferSize), 
    currentBuffer_(new LogBuffer(bufferSize)),
    running_(true),
    threadLog_(std::bind(&AsyncLogging::threadFunc, this)) {}
  ~AsyncLogging() {
    stop();
  }

public:
  void stop() {
    if (running_) {
      running_ = false;
      cond_.notify_all();
      threadLog_.join();
    }
  }
  void append(const std::string& msg) {
    append(msg.c_str(), msg.size());
  }
  void append(const char* msg, int len) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (currentBuffer_->writableBytes() >= static_cast<size_t>(len)) {
      currentBuffer_->append(msg, len);
    } else {
      bufferVec_.push_back(std::move(currentBuffer_));
      if (nextBuffer_) {
        currentBuffer_ = std::move(nextBuffer_);
      } else {
        currentBuffer_.reset(new LogBuffer(bufferSize_));
      }
      currentBuffer_->append(msg, len);
      cond_.notify_one();
    }
  }

  void threadFunc() {
    BufferPtr newBuffer1(new LogBuffer(bufferSize_));
    BufferPtr newBuffer2(new LogBuffer(bufferSize_));
    std::vector<BufferPtr> writeBufferVec;
    LogFile logfile(path_, name_, maxSize_);
    while(running_) {
      {
        std::unique_lock<std::mutex> lock(mutex_);
        if (bufferVec_.empty()) {
          cond_.wait_for(lock, std::chrono::seconds(3));
        }
        bufferVec_.push_back(std::move(currentBuffer_));
        currentBuffer_ = std::move(newBuffer1);
        if (!nextBuffer_) {
          nextBuffer_ = std::move(newBuffer2);
        }
        writeBufferVec.swap(bufferVec_);
      }
      for (int i = 0; i < writeBufferVec.size(); ++i) {
        logfile.append(writeBufferVec[i]->peek(), writeBufferVec[i]->readableBytes());
        writeBufferVec[i]->retrieveAll();
      }
      if (!newBuffer1) {
        assert(!writeBufferVec.empty());
        newBuffer1 = std::move(writeBufferVec[0]);
      }
      if (!newBuffer2) {
        if (writeBufferVec.size() >= 2) {
          newBuffer2 = std::move(writeBufferVec[1]);
        } else {
          newBuffer2.reset(new LogBuffer(bufferSize_));
        }
      }
      logfile.flush();
      writeBufferVec.clear();
    }
  }

  static AsyncLogging* getLogInstance() { return &log_; }

private:
  static server::AsyncLogging log_;

private:
  std::string name_;
  std::string path_;
  size_t bufferSize_;
  long maxSize_;
  volatile bool running_;
  std::thread threadLog_;
  std::mutex mutex_;
  std::condition_variable cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  std::vector<BufferPtr> bufferVec_;
};

AsyncLogging AsyncLogging::log_("TcpServer", "/home/ubuntu/project/myserver/build/logs");

}


#endif