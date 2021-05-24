#include "timer.h"
#include "base/logging.h"

using namespace server::net;


void Timer::restart(TimeStamp now)
{
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        LOG_ERROR << "Error in Timer::restart.";
    }

}