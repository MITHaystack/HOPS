#include <iostream>
#include <chrono>

#include "date/date.h"
#include "date/tz.h"

using namespace date;
using namespace std;
using namespace std::chrono;

int main(int /*argc*/, char** /*argv*/)
{
    using tod = time_of_day<nanoseconds>;
    constexpr tod t1 = tod{hours{13} + minutes{7} + seconds{5} + nanoseconds{22}};

    auto now = system_clock::now();

    auto tai_now = tai_clock::now();

    std::cout<<"t1 = "<<t1<<std::endl;

    std::cout<<"now = "<<now<<std::endl;

    std::cout<<"tai now = "<<tai_now<<std::endl;

    return 0;
}
