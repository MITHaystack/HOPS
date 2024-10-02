#include <chrono>
#include <iostream>
#include <istream>
#include <unistd.h>

#include "MHO_Message.hh"
#include "MHO_Profiler.hh"

using namespace hops;

void func1()
{
    //todo
}

int main(int /*argc*/, char** /*argv*/)
{
    profiler_start();

    sleep(4);

    profiler_stop();

    MHO_Profiler::GetInstance().DumpEvents();

    return 0;
}
