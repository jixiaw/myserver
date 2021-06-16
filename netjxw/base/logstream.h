#ifndef SERVER_BASE_LOGSTREAM_H
#define SERVER_BASE_LOGSTREAM_H
#include <string>
#include <iostream>
namespace server {
class LogStream
{
public:
    LogStream() {}
    LogStream(const std::string& str): data(str) {}

    LogStream& operator<<(const char* x) {
        data.append(x);
        return *this;
    }

    LogStream& operator<<(const std::string& x) {
        data += x;
        return *this;
    }

    LogStream& operator<<(bool x) {
        if (x) data += "true";
        else data += "false";
        return *this;
    }

    LogStream& operator<<(int x) {
        data.append(std::to_string(x));
        return *this;
    }

    LogStream& operator<<(uint x) {
        data.append(std::to_string(x));
        return *this;
    }

    LogStream& operator<<(int64_t x) {
        data.append(std::to_string(x));
        return *this;
    }

    LogStream& operator<<(uint64_t x) {
        data.append(std::to_string(x));
        return *this;
    }

    LogStream& operator<<(int16_t x) {
        data.append(std::to_string(x));
        return *this;
    }
    
    LogStream& operator<<(uint16_t x) {
        data.append(std::to_string(x));
        return *this;
    }
    
    LogStream& operator<<(double x) {
        data.append(std::to_string(x));
        return *this;
    }

    const std::string& getData() const { return data; }

    void print() {
        std::cout<<data<<std::endl;
        data.clear();
    }

private:
    std::string data;
};

}
#endif