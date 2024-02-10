#include "MHO_Timer.hh"

#include <iostream>

namespace hops{

MHO_Timer::MHO_Timer():fName("generic_timer")
{
    fClockID = CLOCK_REALTIME; //default is wallclock
}

MHO_Timer::MHO_Timer(std::string name):fName(name)
{
    fClockID = CLOCK_REALTIME; //default is wallclock
}

MHO_Timer::~MHO_Timer(){}

//set the clock type used
void MHO_Timer::MeasureWallclockTime()
{
    fClockID = CLOCK_REALTIME;
}

void MHO_Timer::MeasureProcessTime()
{
    fClockID = CLOCK_PROCESS_CPUTIME_ID;
}

void MHO_Timer::MeasureThreadTime()
{
    fClockID = CLOCK_THREAD_CPUTIME_ID;
}

void MHO_Timer::Start()
{
    clock_gettime(fClockID, &fStart);
}

void MHO_Timer::Stop()
{
	clock_gettime(fClockID, &fStop);
}


timespec
MHO_Timer::GetTimeDifference(const timespec& start, const timespec& stop) const
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
MHO_Timer::GetDurationAsTimeSpec() const
{
    return GetTimeDifference(fStart,fStop);
}

double 
MHO_Timer::GetTimeSinceStart() const 
{
    timespec tmp;
    clock_gettime(fClockID, &tmp);
    auto diff = GetTimeDifference(fStart, tmp);
    double ret_val = diff.tv_sec;
    ret_val += ( (double) diff.tv_nsec )*1e-9;
    return ret_val;
}

double 
MHO_Timer::GetDurationAsDouble() const
{
    timespec duration = GetTimeDifference(fStart, fStop);
    double ret_val = (double) duration.tv_sec;
    ret_val += ( (double) duration.tv_nsec )*1e-9;
    return ret_val;
}

}
