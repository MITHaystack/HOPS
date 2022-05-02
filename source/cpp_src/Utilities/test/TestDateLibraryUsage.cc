#include <iostream>
#include <chrono>

#include "date/date.h"
#include "date/tz.h"

using namespace date;
using namespace std;
using namespace std::chrono;


struct test_clock
{
    typedef std::chrono::seconds              duration;
    typedef duration::rep                     rep;
    typedef duration::period                  period;
    typedef std::chrono::time_point<test_clock> time_point;
    static const bool is_steady =             false;

    static time_point now() noexcept
    {
        return time_point
          (
            duration_cast<duration>(system_clock::now().time_since_epoch()) -
            hours(6768*24)
          );
    }
};

int main(int /*argc*/, char** /*argv*/)
{
    using tod = time_of_day<nanoseconds>;
    constexpr tod t1 = tod{hours{13} + minutes{7} + seconds{5} + nanoseconds{22}};

    auto now = system_clock::now();

    auto tai_now = tai_clock::now();
    auto utc_now = utc_clock::now();
    auto test_now = test_clock::now();

    std::cout<<"t1 = "<<t1<<std::endl;
    std::cout<<"now = "<<now<<std::endl;
    std::cout<<"tai now = "<<tai_now<<std::endl;
    std::cout<<"time since test clock epoch (sec) = "<<test_now.time_since_epoch().count()<<std::endl;
    

    auto info = get_leap_second_info(utc_now);
    std::cout<<"leap seconds elapsed = "<<info.elapsed<<std::endl;

    return 0;
}
