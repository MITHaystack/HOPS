#include <iostream>
#include <chrono>
#include <istream>
#include <unistd.h>

#include "MHO_Timer.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Timer timer;
    double current_time;

    timer.Start();

    sleep(1);

    current_time = timer.GetTimeSinceStart();
    std::cout<<"current time = "<<current_time<<std::endl;

    sleep(2);

    current_time = timer.GetTimeSinceStart();
    std::cout<<"current time = "<<current_time<<std::endl;

    sleep(1);

    timer.Stop();

    current_time = timer.GetTimeSinceStart();
    std::cout<<"current time = "<<current_time<<std::endl;

    std::cout<<"duration = "<<timer.GetDurationAsDouble()<<std::endl;


    return 0;
}
