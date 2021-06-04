#include "base/timestamp.h"
#include <iostream>
#include <assert.h>
using namespace server;


int main(){
    TimeStamp ts = TimeStamp::now();
    std::cout<<ts.toString()<<std::endl;
    std::cout<<ts.toFormatString()<<std::endl;

    TimeStamp ts2 = TimeStamp::now();
    assert(ts < ts2);
    TimeStamp a = addTime(ts, 10);
    std::cout<<a.toFormatString()<<std::endl;


    return 0;
}