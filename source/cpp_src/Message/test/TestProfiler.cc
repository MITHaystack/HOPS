#include <chrono>
#include <iostream>
#include <istream>
#include <unistd.h>

#include "MHO_Message.hh"
#include "MHO_Profiler.hh"

using namespace hops;

void func1()
{
    profiler_scope();
    sleep(1);
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Profiler::GetInstance().Enable();

    profiler_start();

    sleep(1);

    func1();

    profiler_stop();

    MHO_Profiler::GetInstance().DumpEvents();

    // Directly exercise the singleton API so coverage of MHO_Profiler.cc does
    // not depend on the build being configured with HOPS_USE_PROFILER (which ifdef's out the macros)
    auto& prof = MHO_Profiler::GetInstance();
    prof.Enable();
    (void)prof.IsEnabled();

    prof.Lock();
    prof.AddEntry(pStart, 0, __FILE__, __LINE__, __PRETTY_FUNCTION__);
    prof.AddEntry(pStop, 0, __FILE__, __LINE__, __PRETTY_FUNCTION__);
    prof.Unlock();

    std::vector< MHO_ProfileEvent > events;
    prof.GetEvents(events);
    std::cout << "captured " << events.size() << " profile events" << std::endl;
    prof.DumpEvents();

    // Disabled path: AddEntry should early-return without recording anything.
    prof.Disable();
    prof.AddEntry(pStart, 0, "ignored", 0, "ignored");

    return 0;
}
