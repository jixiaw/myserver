#ifndef SERVER_BASE_CODECUTIL_H
#define SERVER_BASE_CODECUTIL_H
#include <string>

namespace server 
{
class CodecUtil
{
public:
    static std::string hex2str(const std::string& hex);
    static int hex2int(char hex);
};
}
#endif