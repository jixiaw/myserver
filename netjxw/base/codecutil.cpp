#include "codecutil.h"
#include <map>
using namespace server;

int CodecUtil::hex2int(char hex)
{
    int res = hex - '0';
    if (res > 9) {
        res = hex - 'A' + 10;
    }
    return res;
}

std::string CodecUtil::hex2str(const std::string& hex)
{
    std::string res(hex);
    int len = res.size();
    int j = 0, i = 0;
    while (i < len) {
        if (res[i] == '%') {
            if (i+2 >= len) break;
            uint8_t temp = hex2int(res[i+1]) << 4 | hex2int(res[i+2]);
            res[j++] = temp;
            i += 3;
        } else {
            res[j++] = res[i++];
        }
    }
    res.resize(j);
    return res;
}