#include "net/inetaddress.h"
#include <iostream>
#include <string>
using namespace server::net;

int main()
{
    InetAddress addr("10.15.198.199", 9888);
    std::string a = addr.toString();
    std::cout<<a<<std::endl;
    InetAddress add(1090);
    a = add.toString();
    std::cout<<a<<std::endl;
}