#include "net/http/httpserver.h"
#include "net/inetaddress.h"
#include "net/eventloop.h"
#include "base/logging.h"
#include "net/http/httpcontext.h"
#include "net/buffer.h"
#include <string>
#include <vector>
using namespace server::net;
using namespace std;
int main()
{
    Logger::setLogLevel(Logger::TRACE);
    {   
        vector<string> v{
            "GET /56.jpg?q=1234 HTTP/1.1",
            "Host: www.hostname.com",
            "User-Agent: Mozilla/5.0 (Windows NT 10.0;",
            "Accept: image/webp",
            "Accept-Encoding: gzip, deflate, sdch",
            "Accept-Language: zh-CN,zh;q=0.8"
        };
        Buffer buffer;
        for (int i = 0; i < v.size(); ++i) {
            buffer.append(v[i]);
            buffer.append("\r\n");
        }
        buffer.append("\r\n");
        HttpContext context_;
        if (context_.parseRequest(&buffer)) {
            const HttpRequest& req = context_.getRequest();
            cout<<req.getMethodString() <<endl;
            cout<<req.getVersion() <<endl;
            cout<<req.getPath() <<endl;
            cout<<req.getQuery() <<endl;
            cout<<req.getBody()<<endl;
        }
    }
    {
        vector<string> v{
            "POST /56.jpg?q=1234 HTTP/1.1",
            "Host: www.hostname.com",
            "User-Agent: Mozilla/5.0 (Windows NT 10.0;",
            "Accept: image/webp",
            "Accept-Encoding: gzip, deflate, sdch",
            "Accept-Language: zh-CN,zh;q=0.8",
            "Content-Length: 10"
        };
        string body("1234567890");
        Buffer buffer;
        for (int i = 0; i < v.size(); ++i) {
            buffer.append(v[i]);
            buffer.append("\r\n");
        }
        buffer.append("\r\n");
        buffer.append(body);
        HttpContext context_;
        if (context_.parseRequest(&buffer)) {
            const HttpRequest& req = context_.getRequest();
            cout<<req.getMethodString() <<endl;
            cout<<req.getVersion() <<endl;
            cout<<req.getPath() <<endl;
            cout<<req.getQuery() <<endl;
            cout<<req.getBody()<<endl;
        }
    }

    return 0;
}
