#include "timer.h"


using namespace server::net;


void Timer::restart(TimeStamp now)
{
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        printf("Error in Timer::restart.\n");
    }

}