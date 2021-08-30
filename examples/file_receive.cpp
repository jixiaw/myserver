#include "net/tcpserver.h"
#include "net/eventloop.h"
#include "net/inetaddress.h"
#include "base/logging.h"
#include <string>
#include <unistd.h>

using namespace server::net;
using namespace std;


class FileRecvServer
{
public:
    FileRecvServer(){}
    ~FileRecvServer(){}

private:
    TcpServer
};