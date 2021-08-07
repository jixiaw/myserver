#ifndef SERVER_BASE_ASYNCLOGGING_H
#define SERVER_BASE_ASYNCLOGGING_H
#include <string>
#include <mutex>
#include <condition_variable>
#include <dirent.h>
#include <sys/file.h>
#include <base/timestamp.h>
namespace server {

class AsyncLogging {
private:
  AsyncLogging(const std::string& name, const std::string& path, long maxSize=1*1024*1024*1024)
  : name_(name), path_(path), maxSize_(maxSize), fp_(NULL) {
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
    size_t n = ::fwrite(msg, 1, len, fp_);
    long size = ftell(fp_);
    if (size > maxSize_) {
      updataFileName();
    }
    ::fflush(fp_);
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
  long maxSize_;
  FILE* fp_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

AsyncLogging AsyncLogging::log_("TcpServer", "/home/ubuntu/project/myserver/build/logs");

}


#endif