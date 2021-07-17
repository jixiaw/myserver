#include "net/tcpserver.h"
#include "net/eventloop.h"
#include "net/inetaddress.h"
#include "base/logging.h"
#include <string>
#include <functional>
#include <unistd.h>

using namespace server::net;
using namespace std;


class FileRecvServer
{
public:
    FileRecvServer(EventLoop *loop, const InetAddress& listenAddr)
    : loop_(loop),
      server_(loop_, listenAddr, "file server")
    {
        server_.setConnectionCallback(std::bind(&FileRecvServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&FileRecvServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    }
    ~FileRecvServer(){}

    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer)
    {
        FILE* fp;
        if (conn->getContext()) {
            fp = (FILE*)conn->getContext();
        } else {
            string filename = TimeStamp::now().toFormatString();
            fp = ::fopen("vgg.py", "wb");
            if (fp) conn->setContext(fp);
            else return;
        }
        if (buffer->readableBytes() >= 4) {
            int size = 0;
            for (int i = 0; i < 4; ++i)
                size = (size << 8) + *(buffer->peek()+i);
            LOG_INFO << "file size "<<size <<" "<<buffer->readableBytes();
            if (buffer->readableBytes() >= 4 + size) {
                buffer->retrieve(4);
                // string content = buffer->retrieveString(size);
                size_t n = ::fwrite(buffer->peek(), 1, size, fp);
                LOG_INFO << "write "<<n<<" bytes to";
                ::fclose(fp);
                conn->setContext(NULL);
                conn->send("All received!");
            }
        }
    }

    void onConnection(const TcpConnectionPtr& conn)
    {
        if (conn->connected()){
            printf("onConnection: new connection [%s] from %s\n",
                conn->getName().c_str(),
                conn->getPeerAddr().toString().c_str());
            conn->setTcpNoDelay(true);
            conn->setKeepAlive(true);
        }
    }

    void start()
    {
        server_.start();
    }
private:
    EventLoop* loop_;

    TcpServer server_;
};

int main(int argc, char* argv[])
{
    Logger::setLogLevel(Logger::DEBUG);
    // cwd = FileUtil::getcwd();
    EventLoop loop;
    InetAddress listenaddr(1235);
    FileRecvServer server(&loop, listenaddr);
    server.start();
    loop.loop();
}