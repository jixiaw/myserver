#ifndef SERVER_BASE_ASYNCLOGGING_H
#define SERVER_BASE_ASYNCLOGGING_H
#include <string>
#include <mutex>
#include <condition_variable>
#include <dirent.h>
#include <sys/file.h>
#include <memory>
#include <vector>
#include <thread>
#include "base/timestamp.h"
#include "base/buffer.h"
namespace server {

class AsyncLogging {
public:
  typedef std::unique_ptr<Buffer> BufferPtr;
private:
  AsyncLogging(const std::string& name, const std::string& path, 
               long maxSize=1*1024*1024*1024, size_t bufferSize=4*1024*1024)
  : name_(name), 
    path_(path), 
    maxSize_(maxSize), 
    fp_(NULL), 
    bufferSize_(bufferSize), 
    currentBuffer_(new Buffer(bufferSize)),
    threadLog_(std::bind(&AsyncLogging::threadFunc, this)) 
  {
    updataFileName();
  }
  ~AsyncLogging() {
    if (fp_) {
      ::fclose(fp_);
    }
  }

public:
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
        currentBuffer_.reset(new Buffer(bufferSize_));
      }
      currentBuffer_->append(msg, len);
      cond_.notify_one();
    }
  }

  void threadFunc() {
    BufferPtr newBuffer1(new Buffer(bufferSize_));
    BufferPtr newBuffer2(new Buffer(bufferSize_));
    std::vector<BufferPtr> writeBufferVec;
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
        fileWrite(writeBufferVec[i]->peek(), writeBufferVec[i]->readableBytes());
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
          newBuffer2.reset(new Buffer(bufferSize_));
        }
      }
      ::fflush(fp_);
      writeBufferVec.clear();
    }
  }

  void fileWrite(const char* msg, int len) {
    size_t n = ::fwrite(msg, 1, len, fp_);
    assert(n == static_cast<size_t>(len));
    long size = ftell(fp_);
    if (size > maxSize_) {
      updataFileName();
    }
    // ::fflush(fp_);
  }

  void updataFileName() {
    std::string newName = path_ + '/' + name_ + "-" + TimeStamp::now().toFormatString(false) + ".log";
    if (fp_) {
      ::fclose(fp_);
    }
    fp_ = ::fopen(newName.c_str(), "wb");
  }

  static AsyncLogging* getLogInstance() { return &log_; }

private:
  static server::AsyncLogging log_;

private:
  std::string name_;
  std::string path_;
  size_t bufferSize_;
  long maxSize_;
  FILE* fp_;
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