#ifndef SERVER_NET_BUFFER_H
#define SERVER_NET_BUFFER_H

#include <functional>
#include <string>
#include <vector>
namespace server {
namespace net {

class Buffer{
public:
    static const size_t kInitialSize = 1024;
    static const size_t kPrepend = 8;
    Buffer(size_t initialSize=kInitialSize)
    : buffer_(kPrepend + initialSize),
      readIdx_(kPrepend),
      writeIdx_(kPrepend) 
    {
    }
    ssize_t readFd(int fd);

    size_t readableBytes() const { return writeIdx_ - readIdx_; }
    size_t writableBytes() const { return buffer_.size() - writeIdx_; }
    size_t prependableBytes() const { return readIdx_; }

    const char* begin() const { return &*(buffer_.begin()); }
    char* begin() { return &*(buffer_.begin()); }

    const char* peek() const { return begin() + readIdx_; }
    char* peek() { return begin() + readIdx_; }
    
    char* beginWritable() { return begin() + writeIdx_; }
    const  char* beginWritable() const { return begin() + writeIdx_; }

    void append(const char* str, size_t len);
    void makeSpace(size_t len);

    std::string retrieveAll();
    std::string retrieve(size_t len);

private:
    std::vector<char> buffer_;
    size_t readIdx_;    // 可读位置
    size_t writeIdx_;   // 可写位置
};

}
}
#endif