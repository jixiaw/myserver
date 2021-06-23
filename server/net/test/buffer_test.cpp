#include "net/buffer.h"
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
using namespace server::net;
using namespace std;
int main()
{
    int fd = ::open("/home/jxw/PycharmProjects/vs_code/c++/myserver/CMakeLists.txt", O_RDONLY);

    Buffer buffer;
    buffer.readFd(fd);
    cout<<buffer.readableBytes()<<endl<<buffer.retrieveAllString()<<endl;
    cout<<"read: "<<buffer.readableBytes()<<endl;
    cout<<"write: "<<buffer.writableBytes()<<endl;
    char str[64] = "test buffer hello 1234567890\n";
    buffer.append(str, 11);
    buffer.append(str+11, 6);
    for (int i = 0; i < 120; ++i) {
        buffer.append(str+18, 10);
    }
    cout<<"read: "<<buffer.readableBytes()<<endl;
    cout<<"write: "<<buffer.writableBytes()<<endl;
}