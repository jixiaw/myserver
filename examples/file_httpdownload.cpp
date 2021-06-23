#include "net/http/httpserver.h"
#include "net/http/httprequest.h"
#include "net/http/httpresponse.h"
#include "net/inetaddress.h"
#include "net/eventloop.h"
#include "base/logging.h"
#include "base/timestamp.h"
#include "base/fileutil.h"
#include "base/codecutil.h"
#include <unistd.h>
#include <algorithm>
using namespace server;
using namespace server::net;
using namespace std;

extern char favicon[555];
bool benchmark = false;

string cwd;

string html[] =  {"<html><head><title>", 
"</title></head><body><h1>",
"</h1><hr><ul>",
"</ul><hr></body></html>"};
string li = "<li><a href=\"%s\">%s</a></li>";

void onRequest(const HttpRequest& req, HttpResponse* resp)
{
    // std::cout << "Headers " << req.getMethodString() << " " << req.getPath() << std::endl;
    if (!benchmark)
    {
        const std::map<string, string>& headers = req.getHeaderMap();
        for (const auto& header : headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }
    string path = cwd + CodecUtil::hex2str(req.getPath());
    FileUtil::FileType type = FileUtil::getType(path);
    if (type == FileUtil::T_DIR) {
        // text/html
        vector<string> files = FileUtil::listdir(path);
        std::cout << "open: "<<path<<endl;
        sort(files.begin(), files.end());
        string title("Directory listing for ");
        title += req.getPath();
        string lis;
        for (const string& file: files) {
            char buf[512];
            snprintf(buf, sizeof buf, li.c_str(), file.c_str(), file.c_str());
            lis += buf;
        }
        string body = html[0] + title + html[1] + title + html[2] + lis + html[3];
        resp->setBody(body);
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html; charset=UTF-8");
        resp->addHeader("Server", "file_downloader");
        LOG_INFO << "onRequest() sent html 200 ok";
    } 
    else if (type == FileUtil::T_FILE) {
        // application/octet-stream
        string content = FileUtil::readFile(path);
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        // resp->setContentType("text/plain; charset=UTF-8");
        resp->setContentType("application/octet-stream; charset=UTF-8");
        resp->addHeader("Server", "file_downloader");
        resp->setBody(content);
        LOG_INFO << "onRequest() sent stream 200 ok";
    } 
    else {
        // 404
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
        LOG_INFO << "onRequest() 404 Not Found";
    }
}


int main(int argc, char* argv[])
{
    Logger::setLogLevel(Logger::DEBUG);
    cwd = FileUtil::getcwd();
    EventLoop loop;
    InetAddress listenaddr(1235);
    HttpServer server(&loop, listenaddr, "http_file_downloader");
    server.setHttpCallback(onRequest);
    if (argc > 1) {
        Logger::setLogLevel(Logger::ERROR);
        benchmark = true;
        server.setThreadNum(atoi(argv[1]));
    }
    server.start();
    loop.loop();
    return 0;
}