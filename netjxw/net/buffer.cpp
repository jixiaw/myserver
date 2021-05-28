#include "buffer.h"
#include "base/logging.h"
#include <unistd.h>
#include <sys/uio.h>

using namespace server::net;
using namespace server::base;

ssize_t Buffer::readFd(int fd)
{
    char buf[65536];
    // ::read(fd, buf, sizeof buf);
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writeIdx_;
    vec[0].iov_len = writable;
    vec[1].iov_base = buf;
    vec[1].iov_len = sizeof buf;
    ssize_t n = ::readv(fd, vec, 2);
    if (n < 0) {
        LOG_ERROR << "Buffer::readFd().";
    } else if (static_cast<size_t>(n) <= writable) {
        writeIdx_ += n;
    } else {
        append(buf, n);
    }
    return n;
}

void Buffer::append(const char* str, size_t len)
{
    if (len > writableBytes()) {
        makeSpace(len);
    }
    std::copy(str, str+len, beginWritable());
    writeIdx_ += len;
}

void Buffer::makeSpace(size_t len) 
{
    if (writableBytes() + prependableBytes() < len + kPrepend) {
        buffer_.resize(writeIdx_ + len);
    } else {
        size_t readable = readableBytes();
        std::copy(begin()+readIdx_, begin()+writeIdx_, begin()+kPrepend);
        readIdx_ = kPrepend;
        writeIdx_ = readIdx_ + readable;
    }
}

void Buffer::retrieve(size_t len)
{
    readIdx_ += len;
    if (readIdx_ >= writeIdx_) {
        retrieveAll();
    }
}

void Buffer::retrieveAll()
{
    readIdx_ = kPrepend;
    writeIdx_ = kPrepend;
}

std::string Buffer::retrieveString(size_t len)
{
    if (len > readableBytes()) {
        LOG_WARN << "Buffer::retrieveString len[" << len
                 << "] more than readableBytes[" << readableBytes() <<"].";
        len = readableBytes();
    }
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

std::string Buffer::retrieveAllString()
{
    return retrieveString(readableBytes());
}
