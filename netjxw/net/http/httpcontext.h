#ifndef SERVER_NET_HTTP_HTTPCONTEXT_H
#define SERVER_NET_HTTP_HTTPCONTEXT_H
#include "net/http/httprequest.h"
#include "base/logging.h"
#include <string>
#include <algorithm>
namespace server {
namespace net{

class Buffer;

class HttpContext
{
public:
    // 解析HttpRequest状态
    enum HttpRequestParseState {
        kRequestLine,   // 请求行
        kHeaders,       // 请求头
        kBody,          // 请求体
        kParseAll,      // 已完成解析
    };
    HttpContext()
    : state_(kRequestLine) 
    {
    }
    ~HttpContext() {}


    bool isParseAll() const { return state_ == kParseAll; }
    const HttpRequest& getRequest() const { return request_; }
    void reset() {
        HttpRequest temp;
        request_.swap(temp);
        state_ = kRequestLine;
    }

    bool parseRequest(Buffer* buffer) {
        bool ok = true;
        bool hasMore = true;
        // LOG_DEBUG
        while (hasMore) {
            if (state_ == kRequestLine) {
                const char* crlf = buffer->findCRLF();
                if (crlf) {
                    ok = parseRequestLine(buffer->peek(), crlf);
                    if (ok) {
                        buffer->retrieveBefore(crlf + 2);
                        state_ = kHeaders;
                    } else {
                        hasMore = false;
                        LOG_ERROR << "HttpContext::parseRequest, parseRequestLine error";
                    }
                } else {
                    hasMore = false;
                }
            } else if (state_ == kHeaders) {
                const char* crlf = buffer->findCRLF();
                if (crlf) {
                    // key: value
                    const char* colon = std::find(buffer->peek(), crlf, ':');
                    if (colon != crlf) {
                        request_.addHeader(buffer->peek(), colon, crlf);
                        buffer->retrieveBefore(crlf + 2);
                    } else {
                        // 空行
                        if (crlf == buffer->peek()) {
                            // get没有body
                            if (request_.getMethod() == HttpRequest::kGet) {
                                state_ = kParseAll;
                                hasMore = false;
                            } else { // post
                                state_ = kBody;
                            }
                            buffer->retrieveBefore(crlf + 2);
                        } else {
                            hasMore = false;
                        }
                    }
                } else {
                    hasMore = false;
                }
            }  else if (state_ == kBody) {
                const std::string& contentLength = request_.getHeader("Content-Length");
                size_t len = static_cast<size_t>(atol(contentLength.c_str()));
                if (contentLength.empty() || len == 0) {
                    hasMore = false;
                    state_ = kParseAll;
                } else {
                    if (buffer->readableBytes() >= len) {
                        request_.setBody(buffer->peek(), buffer->peek() + len);
                        buffer->retrieve(len);
                        state_ = kParseAll;
                        hasMore = false;
                    } else {
                        hasMore = false;
                    }
                }
            }
        }
        return ok;
    }
private:
    bool parseRequestLine(const char* start, const char* end) {
        std::string s(start, end);
        LOG_DEBUG << "httpContext::parseRequestLine: " << s;
        bool ok = false;
        const char* p = start;
        const char* space = std::find(p, end, ' ');
        if (space != end && request_.setMethod(p, space)) {
            p = space + 1;
            space = std::find(p, end, ' ');
            if (space != end) {
                const char* q = std::find(p, space, '?');
                if (q != space) {
                    request_.setPath(p, q);
                    request_.setQuery(q, space);
                } else {
                    request_.setPath(p, space);
                }
                p = space + 1;
                ok = end - p == 8 && std::equal(p, end-1, "HTTP/1.");
                if (ok) {
                    if (*(end - 1) == '1') {
                        request_.setVersion(HttpRequest::kHttp11);
                    } else if (*(end - 1) == '0') {
                        request_.setVersion(HttpRequest::kHttp10);
                    } else {
                        ok = false;
                        LOG_ERROR << "HttpContext::parseRequestLine, http version error";
                    }
                } else {
                    LOG_ERROR << "HttpContext::parseRequestLine, end - p = " << end - p << std::string(p, end);
                }
            } else {
                LOG_ERROR << "HttpContext::parseRequestLine, second space == end";
            }
        } else {
            LOG_ERROR << "HttpContext::parseRequestLine, first space == end"; 
        }
        return ok;
    }

private:
    HttpRequest request_;
    HttpRequestParseState state_;
};
}
}

#endif