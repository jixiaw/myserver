#include "net/eventloop.h"
#include "net/tcpserver.h"
#include "net/inetaddress.h"
#include "net/buffer.h"
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
using namespace std;
using namespace server;
using namespace server::net;

string g_file;

string readFile(const char* filename)
{
    printf("readfile: %s\n", filename);
    FILE* fp = fopen(filename, "rb");
    string res;
    if (fp) {
        char buf[1024*1024];
        size_t nread = 0;
        while((nread = ::fread(buf, 1, sizeof buf, fp)) > 0) {
            res.append(buf, nread);
        }
        ::fclose(fp);
    }
    return res;
}

string readFile(const string& filename)
{
    fstream fs(filename.c_str(), ios::in);
    char buf[1024*1024];
    string res;
    while(!fs.eof()) {
        fs.read(buf, sizeof buf);
        res.append(buf);
    }
    return res;
}

void onHighWater(const TcpConnectionPtr& conn, size_t len)
{
    cout<<"onHighWater size: "<<len<<endl;
}

void onMessage(const TcpConnectionPtr& conn, Buffer *buffer)
{

}

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        conn->setHighWaterMarkCallback(onHighWater, 64*1024);
        string content = readFile(g_file.c_str());
        conn->send(content);
        conn->shutdown();
    }
}

void testReadFile()
{
    char path[256];
    if (::getcwd(path, sizeof path)) {
        printf("path: %s\n", path);
    }
    string filename(path);
    filename += "/core";
    string content = readFile(filename);
    printf("content: %s\n", content.c_str());
}

int main(int argc, char* argv[])
{
    char path[256];
    if (::getcwd(path, sizeof path)) {
        printf("path: %s\n", path);
    }
    string filename(path);
    filename += "/";
    if (argc > 1) {
        filename += argv[1];
    }
    g_file = filename;
    EventLoop loop;
    TcpServer server(&loop, InetAddress(1234), "file server");
    server.setConnectionCallback(onConnection);
    server.start();
    loop.loop();
    return 0;
}