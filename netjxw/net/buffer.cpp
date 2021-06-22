#include "buffer.h"
#include "base/logging.h"
#include <unistd.h>
#include <sys/uio.h>
#include <algorithm>
#include <error.h>

using namespace server::net;
using namespace server;

const char Buffer::CRLF[] = "\r\n";

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
        append(buf, n - writable);
    }
    return n;
}

ssize_t Buffer::readLoop(int fd, bool& flag)
{
    ssize_t n = readFd(fd);
    if (n <= 0) return n;
    // LOG_ERROR << "Buffer::readLoop()";
    char buf[65536];
    size_t bufsize = sizeof buf;
    while (true) {
        ssize_t m = ::read(fd, buf, bufsize);
        if (m > 0) {
            n += m;
            append(buf, m);
            if (m < bufsize) break;
        } else if (m < 0) {
            if (errno == EAGAIN) {
                LOG_DEBUG << "Buffer::readLoop() read EAGAIN : ["<<n<<"]";
                break;
            } else if (errno == EINTR) {
                LOG_DEBUG << "Buffer::readLoop() read EINTR : ["<<n<<"]";
            } else {
                LOG_ERROR << "Buffer::readLoop() read ERROR["<<errno<<"]";
                return -1;
            }
        } else {
            LOG_DEBUG << "Buffer::readLoop() read 0 byte";
            flag = true;
            // return 0;
            break;
        }
    }
    return n;
}

ssize_t Buffer::writeFd(int fd)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n > 0) {
        retrieve(n);
    }
    return n;
}

ssize_t Buffer::writeLoop(int fd)
{
    ssize_t sum = 0;
    while(readableBytes() > 0) {
        ssize_t n = ::write(fd, peek(), readableBytes());
        if (n <= 0) {
            LOG_ERROR << "Buffer::writeLoop() error: ["<<errno <<"]";
            break;
        }
        retrieve(n);
        sum += n;
    }
    return sum;
}

void Buffer::append(const char* str, size_t len)
{
    if (len > writableBytes()) {
        makeSpace(len);
    }
    std::copy(str, str+len, beginWritable());
    writeIdx_ += len;
}

void Buffer::append(const std::string& str)
{
    append(str.c_str(), str.size());
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
void Buffer::retrieveBefore(const char* end) {
    assert(peek() <= end);
    assert(end <= beginWritable());
    retrieve(end - peek());
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

const char* Buffer::findCRLF(const char* start) const {
    assert(peek() <= start);
    assert(start <= beginWritable());
    const char* pos = std::search(start, beginWritable(), CRLF, CRLF+2);
    return pos == beginWritable() ? NULL : pos;
}

const char* Buffer::findCR(const char* start) const {
    assert(peek() <= start);
    const char* end = beginWritable();
    assert(start <= end);
    const char* p = start;
    while(p < end) {
        if (*p == '\n') {
            return p;
        }
        ++p;
    }
    return NULL;
}

const char* Buffer::findCRLF() const {
    return findCRLF(peek());
}
