#include "base/timestamp.h"
#include <iostream>
#include <assert.h>
using namespace server::base;


int main(){
    TimeStamp ts = TimeStamp::now();
    std::cout<<ts.toString()<<std::endl;
    std::cout<<ts.toFormatString()<<std::endl;

    TimeStamp ts2 = TimeStamp::now();
    assert(ts < ts2);


    return 0;
}