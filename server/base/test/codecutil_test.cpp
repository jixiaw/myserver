#include "base/codecutil.h"
#include <iostream>
using namespace std;
using namespace server;

int main()
{
    string res = "abc啊你~";
    string str = "abc%E5%95%8A%E4%BD%A0~";
    cout<<res <<" "<<CodecUtil::hex2str(str)<<endl;

    return 0;
}