#include <iostream>
#include <chrono>
#include <istream>
#include <unistd.h>

#include "MHO_Profiler.hh"
#include "MHO_Message.hh"

using namespace hops;

void func1()
{
    //todo
}

int main(int /*argc*/, char** /*argv*/)
{
    prof_start();
    
    sleep(4);
    
    prof_stop();

    MHO_Profiler::GetInstance().DumpEvents();

    return 0;
}
