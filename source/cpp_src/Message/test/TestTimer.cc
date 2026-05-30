#include <chrono>
#include <iostream>
#include <istream>
#include <unistd.h>

#include "MHO_Timer.hh"

using namespace hops;

// MHO_Timer::GetTimeDifference is protected; this shim exposes it so the test
// can drive the borrow branch (stop.tv_nsec < start.tv_nsec) deterministically.
class TimerProbe: public MHO_Timer
{
    public:
        using MHO_Timer::GetTimeDifference;
};

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Timer timer;
    double current_time;

    timer.Start();

    sleep(1);

    current_time = timer.GetTimeSinceStart();
    std::cout << "current time = " << current_time << std::endl;

    sleep(2);

    current_time = timer.GetTimeSinceStart();
    std::cout << "current time = " << current_time << std::endl;

    sleep(1);

    timer.Stop();

    current_time = timer.GetTimeSinceStart();
    std::cout << "current time = " << current_time << std::endl;

    std::cout << "duration = " << timer.GetDurationAsDouble() << std::endl;

    // Named ctor + the three clock-source setters.
    MHO_Timer named("named_timer");
    named.MeasureWallclockTime();
    named.MeasureProcessTime();
    named.MeasureThreadTime();
    named.Start();
    named.Stop();

    // GetDurationAsTimeSpec on the original timer (already Start/Stop'd).
    timespec dur = timer.GetDurationAsTimeSpec();
    std::cout << "duration tv_sec=" << dur.tv_sec << " tv_nsec=" << dur.tv_nsec << std::endl;

    // Force the nsec-borrow branch in GetTimeDifference by feeding it crafted
    // timespecs where stop.tv_nsec < start.tv_nsec.
    TimerProbe probe;
    timespec s = {};
    timespec e = {};
    s.tv_sec = 1;
    s.tv_nsec = 900000000;
    e.tv_sec = 2;
    e.tv_nsec = 100000000;
    timespec borrowed = probe.GetTimeDifference(s, e);
    std::cout << "borrowed tv_sec=" << borrowed.tv_sec << " tv_nsec=" << borrowed.tv_nsec << std::endl;

    return 0;
}
