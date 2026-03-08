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
    profiler_scope();

    sleep(4);

    

    MHO_Profiler::GetInstance().DumpEvents();

    return 0;
}
