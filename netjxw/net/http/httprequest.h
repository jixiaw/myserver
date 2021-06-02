#ifndef SERVER_NET_HTTP_HTTPREQUEST_H
#define SERVER_NET_HTTP_HTTPREQUEST_H

#include <string>
#include <map>
#include <assert.h>

namespace server {
namespace net{

/*
http Request
请求行:     method_ path_?query_ version_
请求头:     key: value
空行
请求体:     body_
*/


class HttpRequest
{
public:
    enum Method {
        kInvalid, kGet, kPost, kHead, kPut, kDelete 
    };
    enum Version {
        kUnKnown, kHttp10, kHttp11
    };
    
    HttpRequest()
    : method_(kInvalid),
      version_(kUnKnown)
    {
    }

    ~HttpRequest() {}
    void setVersion(Version v) { version_ = v; }
    void setMethod(Method m) { method_ = m; }

    Version getVersion() const { return version_; }
    Method getMethod() const { return method_; }
    bool setMethod(const char* start, const char* end) {
        assert(method_ == kInvalid);
        std::string m(start, end);
        if (m == "GET") {
            method_ = kGet;
        } else if (m == "POST") {
            method_ = kPost;
        } else if (m == "HEAD") {
            method_ = kHead;
        } else if (m == "PUT") {
            method_ = kPut;
        } else if (m == "DELETE") {
            method_ = kDelete;
        } else {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }
    const char* getMethodString() const {
        const char* result = "UNKNOWN";
        switch (method_)
        {
        case kGet:
            result = "GET";
            break;
        case kPost:
            result = "POST";
            break;
        case kHead:
            result = "HEAD";
            break;
        case kPut:
            result = "PUT";
            break;
        case kDelete:
            result = "DELETE";
            break;
        default:
            break;
        }
        return result;
    }

    void setPath(const char* start, const char* end) {
        path_.assign(start, end);
    }
    const std::string& getPath() const { return path_; }

    void setQuery(const char* start, const char* end) {
        query_.assign(start, end);
    }
    const std::string& getQuery() const { return query_; }

    void setBody(const char* start, const char* end) {
        body_.assign(start, end);
    }
    const std::string& getBody() const { return body_; }

    // colon: 冒号
    void addHeader(const char* start, const char* colon, const char* end) {
        std::string key(start, colon);
        ++colon;
        while (colon < end && isspace(*colon)) { ++colon; }
        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size()-1])) {
            value.resize(value.size()-1);
        }
        headerMap_[key] = value;
    }
    
    std::string getHeader(const std::string& key) const {
        auto it = headerMap_.find(key);
        if (it != headerMap_.end()) {
            return it->second;
        }
        return "";
    }

    std::map<std::string, std::string> getHeaderMap() const {
        return headerMap_;
    }

    void swap(HttpRequest& thr) {
        std::swap(method_, thr.method_);
        std::swap(version_, thr.version_);
        path_.swap(thr.path_);
        query_.swap(thr.query_);
        headerMap_.swap(thr.headerMap_);
    }

private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    std::string body_;
    std::map<std::string, std::string> headerMap_;
};
}
}

#endif