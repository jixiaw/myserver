#ifndef SERVER_BASE_LOGFILE_H
#define SERVER_BASE_LOGFILE_H
#include "base/timestamp.h"
#include <string>
#include <fcntl.h>
#include <assert.h>
namespace server {
class LogFile {
public:
  LogFile(const std::string& path, 
          const std::string& name, 
          size_t maxSize=1*1024*1024*1024)
  : path_(path), 
    name_(name),
    maxSize_(maxSize),
    fileName_(),
    fp_(NULL)
  {
    rollFile();
  }
  ~LogFile() {
    if (fp_) {
      ::fclose(fp_);
    }
  }

public:
  void append(const char* msg, int len) {
    size_t n = ::fwrite(msg, 1, len, fp_);
    assert(n == static_cast<size_t>(len));
    size_t size = static_cast<size_t>(ftell(fp_));
    if (size > maxSize_) {
      rollFile();
    }
  }
  void rollFile() {
    fileName_ =  path_ + '/' + name_ + "-" + TimeStamp::now().toFormatString(true) + ".log";
    if (fp_) {
      ::fclose(fp_);
    }
    fp_ = ::fopen(fileName_.c_str(), "wb");
  }
  void flush() {
    if (fp_) {
      ::fflush(fp_);
    }
  }
private:
  std::string path_;
  std::string name_;
  std::string fileName_;
  FILE* fp_;
  size_t maxSize_;
};
} 

#endif