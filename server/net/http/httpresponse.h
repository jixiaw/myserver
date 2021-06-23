#ifndef SERVER_NET_HTTP_HTTPRESPONSE_H
#define SERVER_NET_HTTP_HTTPRESPONSE_H

#include <string>
namespace server {
namespace net{

class Buffer;

/*
http响应
响应行:     HTTP/1.1 statusCode_(200) statusMessage_(OK)
响应头:     key: value
空行
响应体
*/


class HttpResponse
{
public:
    enum HttpStatusCode {
        kUnKnown,
        k200Ok = 200,
        k301MovePermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };
    explicit HttpResponse(bool close)
    : statusCode_(kUnKnown),
      closeConnection_(false)
    {
    }
    ~HttpResponse() {};

    void setStatusCode(HttpStatusCode code) {
        statusCode_ = code;
    }
    HttpStatusCode getStatusCode() const { return statusCode_; }

    void setStatusMessage(const std::string& message) {
        statusMessage_ = message;
    }
    const std::string& getStatusMessage() const { return statusMessage_; }
    
    void setCloseConnection(bool on) {
        closeConnection_ = on;
    }
    bool getCloseConnection() const { return closeConnection_; }

    void addHeader(const std::string& key, const std::string& value) {
        headerMap_[key] = value;
    }
    // 内容类型，一般需要频繁设置，因此独立出来
    void setContentType(const std::string& contentType) {
        addHeader("Content-Type", contentType);
    }
    
    void setBody(const std::string& body) {
        body_ = body;
    }

    void appendToBuffer(Buffer* buffer) const {
        char buf[32];
        snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
        buffer->append(buf);
        buffer->append(statusMessage_);
        buffer->append("\r\n");
        if (closeConnection_) {
            buffer->append("Connection: close\r\n");
        } else {
            // Keep-Alive 才设置Content-Length
            snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
            buffer->append(buf);
            buffer->append("Connection: Keep-Alive\r\n");
        }
        for (auto& header: headerMap_) {
            buffer->append(header.first);
            buffer->append(": ");
            buffer->append(header.second);
            buffer->append("\r\n");
        }
        buffer->append("\r\n");
        buffer->append(body_);
    }


private:
    HttpStatusCode statusCode_;
    bool closeConnection_;
    std::string statusMessage_;
    std::map<std::string, std::string> headerMap_;
    std::string body_;
};
}
}

#endif