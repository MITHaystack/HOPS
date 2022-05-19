#include <iostream>
#include <chrono>
#include <istream>


#include "date/date.h"
#include "date/julian.h"
#include "date/tz.h"

using namespace date;
using namespace std;
using namespace std::chrono;


#include "MHO_Clock.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    auto hops_now = hops_clock::now();
    auto tai_now = hops_clock::to_tai(hops_now);
    auto utc_now = hops_clock::to_utc(hops_now);
    auto gps_now = hops_clock::to_gps(hops_now);

    std::cout<<"hops now = "<<hops_now<<std::endl;
    std::cout<<"utc now = "<<utc_now<<std::endl;
    std::cout<<"tai now = "<<tai_now<<std::endl;
    std::cout<<"gps now = "<<gps_now<<std::endl;

    std::cout<<"ticks since tai clock epoch = "<<tai_now.time_since_epoch().count()<<std::endl;
    std::cout<<"ticks since utc clock epoch = "<<utc_now.time_since_epoch().count()<<std::endl;
    std::cout<<"ticks since gps clock epoch = "<<gps_now.time_since_epoch().count()<<std::endl;
    std::cout<<"ticks since hops clock epoch = "<<hops_now.time_since_epoch().count()<<std::endl;

    auto hops_epoch_start_utc = hops_clock::get_hops_epoch_utc();
    auto hops_epoch_start_tai = tai_clock::from_utc(hops_epoch_start_utc);

    std::cout<<"hops clock epoch start as UTC date/time = "<<hops_epoch_start_utc<<std::endl;
    std::cout<<"hops clock epoch start as TAI date/time = "<<hops_epoch_start_tai<<std::endl;

    auto info = date::get_leap_second_info(utc_now);
    std::cout<<"leap seconds elapsed = "<<info.elapsed<<std::endl;

    std::cout<<"hops epoch nanoseconds from system utc epoch start = "<<hops_epoch_start_utc.time_since_epoch().count()<<std::endl;
    std::cout<<"hops epoch nanoseconds from system tai epoch start = "<<hops_epoch_start_tai.time_since_epoch().count()<<std::endl;

    //test the conversion of a random date
    std::string test = "2020-12-31T11:59:07.010916Z";

    std::cout<<"testing conversion of the UTC date string = "<<test<<std::endl;

    auto hops_tp2 = hops_clock::from_iso8601_format(test);

    std::cout<<"hops time-point from test string = "<<hops_tp2<<std::endl;

    std::string vex_str = hops_clock::to_vex_format(hops_tp2);

    std::cout<<"hops test time-point in vex format = "<<vex_str<<std::endl;

    std::string hops_str = hops_clock::to_hops_raw_format(hops_tp2);

    std::cout<<"hops-formated time-point string = "<<hops_str<<std::endl;

    auto hops_tp3 = hops_clock::from_hops_raw_format(hops_str);

    std::cout<<"hops time-point converted back from hops-formatted string = "<<hops_tp3<<std::endl;

    auto hops_tp4 = hops_clock::from_vex_format(vex_str); 

    std::cout<<"hops time-point converted back from vex string = "<<hops_tp4<<std::endl;

    std::string vtest = "2020y63d12h08m04s";

    std::cout<<"converting the following vex date/time: "<<vtest<<std::endl;

    std::cout<<"to hops date in iso-8601 format: "<<hops_clock::to_iso8601_format( hops_clock::from_vex_format(vtest) )<<std::endl;

    return 0;
}
