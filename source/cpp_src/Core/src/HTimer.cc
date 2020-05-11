#include "HTimer.hh"

namespace hops{

HTimer::HTimer():fName("generic_timer")
{
}

HTimer::HTimer(std::string name):fName(name)
{
}

HTimer::~HTimer(){}

//set the clock type used
void HTimer::MeasureWallclockTime()
{
    fClockID = CLOCK_REALTIME;
}

void HTimer::MeasureProcessTime()
{
    fClockID = CLOCK_PROCESS_CPUTIME_ID;
}

void HTimer::MeasureThreadTime()
{
    fClockID = CLOCK_THREAD_CPUTIME_ID;
}

void HTimer::Start()
{
    clock_gettime(CLOCK_REALTIME, &fStart);
}

void HTimer::Stop()
{
	clock_gettime(CLOCK_REALTIME, &fStop);
}


timespec 
HTimer::GetTimeDifference(const timespec& start, const timespec& stop) const
{
    timespec ret_val;
    if( (stop.tv_nsec-start.tv_nsec) < 0)
    {
        ret_val.tv_sec = stop.tv_sec-start.tv_sec-1;
        ret_val.tv_nsec = 1000000000+stop.tv_nsec-start.tv_nsec;
    }
    else
    {
        ret_val.tv_sec = stop.tv_sec-start.tv_sec;
        ret_val.tv_nsec = stop.tv_nsec-start.tv_nsec;
    }
    return ret_val;
}


timespec 
HTimer::GetDurationAsTimeSpec() const
{
    return GetTimeDifference(fStart,fStop);
}

double HTimer::GetDurationAsDouble() const
{
    timespec duration = GetTimeDifference(fStart, fStop);
    double ret_val = duration.tv_sec;
    ret_val += ( (double) duration.tv_nsec )*1e-9;
    return ret_val;
}

}
