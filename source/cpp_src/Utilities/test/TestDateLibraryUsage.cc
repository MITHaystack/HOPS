#include <iostream>
#include <chrono>
#include <istream>


#include "date/date.h"
#include "date/julian.h"
#include "date/tz.h"

using namespace date;
using namespace std;
using namespace std::chrono;


#include "MHO_Clocks.hh"

using namespace hops;


struct test_clock
{
    typedef std::chrono::nanoseconds              duration;
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
    auto now = system_clock::now();
    auto test_now = test_clock::now();
    auto tai_now = date::tai_clock::now();
    auto utc_now = date::utc_clock::now();
    auto gps_now = date::gps_clock::now();
    auto hops_now = hops_clock::now();

    // std::cout<<"t1 = "<<t1<<std::endl;
    // std::cout<<"now = "<<now<<std::endl;
    std::cout<<"utc now = "<<utc_now<<std::endl;
    std::cout<<"tai now = "<<tai_now<<std::endl;
    std::cout<<"gps now = "<<gps_now<<std::endl;
    std::cout<<"hops now = "<<hops_now<<std::endl;

    std::cout<<"ticks since tai clock epoch = "<<tai_now.time_since_epoch().count()<<std::endl;
    std::cout<<"ticks since utc clock epoch = "<<utc_now.time_since_epoch().count()<<std::endl;
    std::cout<<"ticks since gps clock epoch = "<<gps_now.time_since_epoch().count()<<std::endl;
    std::cout<<"ticks since hops clock epoch = "<<hops_now.time_since_epoch().count()<<std::endl;

    auto info = date::get_leap_second_info(utc_now);
    std::cout<<"leap seconds elapsed = "<<info.elapsed<<std::endl;

    // auto zt = date::make_zoned(date::current_zone(), std::chrono::system_clock::now());
    // auto ld = date::floor<date::days>(zt.get_local_time());
    // julian::year_month_day ymd{ld};
    // auto time = date::make_time(zt.get_local_time() - ld);
    // std::cout << ymd << ' ' << time << '\n';

    // std::string hops_epoch_start = {"20000101 12:00:00"};
    std::string hops_epoch_start = {"1970-01-01 00:00:00.000001"};
    std::chrono::system_clock::time_point hops_epoch_start_tp;
    std::istringstream ss{ hops_epoch_start };
    std::string frmt = "%F %T";
    ss >> date::parse(frmt, hops_epoch_start_tp);

    // std::istringstream utcss{ hops_epoch_start };
    // date::utc_clock::time_point utc_start_tp;
    // utcss >> date::parse(frmt, utc_start_tp);

    auto utc_hops_epoch_start = date::utc_clock::from_sys(hops_epoch_start_tp);
    auto gps_hops_epoch_start = date::gps_clock::from_utc(utc_hops_epoch_start);

    std::cout<<"hops epoch nanoseconds from system time start = "<< hops_epoch_start_tp.time_since_epoch().count()<<std::endl;
    std::cout<<"hops epoch nanoseconds from system utc epoch start = "<<utc_hops_epoch_start.time_since_epoch().count()<<std::endl;
    std::cout<<"hops epoch nanoseconds from system gps epoch start = "<<gps_hops_epoch_start.time_since_epoch().count()<<std::endl;

    //std::string j2000 = "2000-01-01 12:00:00.0";
    std::string j2000 = "2000-01-01 11:59:27.816";
    frmt = "%F %T";
    date::tai_time<std::chrono::nanoseconds> j2000_tai_tp;
    std::istringstream ss2(j2000);
    std::istream stream(ss2.rdbuf());
    date::from_stream(stream, frmt.c_str(), j2000_tai_tp);
    auto j2000_utc_tp =  std::chrono::time_point_cast<std::chrono::nanoseconds>( date::tai_clock::to_utc( j2000_tai_tp ) );
    std::cout<<"J2000 utc, ns since epoch = "<< j2000_utc_tp.time_since_epoch().count() <<std::endl;
    std::cout<<"J2000 tai, ns since epoch = "<< j2000_tai_tp.time_since_epoch().count() <<std::endl;

    auto hops_tp = hops_clock::from_utc(j2000_utc_tp);
    std::cout<<"J2000 hops, ns since epoch "<<hops_tp.time_since_epoch().count()<<std::endl;

    std::cout<<"utc time = "<<j2000_utc_tp<<std::endl;
    std::cout<<"hops time = "<<hops_tp<<std::endl;
    
    return 0;
}
