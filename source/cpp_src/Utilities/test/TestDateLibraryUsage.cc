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

date::days
day_of_year(date::sys_days sd)
{
    auto y = year_month_day{sd}.year();
    return sd - sys_days{y/jan/0};
}


int main(int /*argc*/, char** /*argv*/)
{
    auto now = system_clock::now();
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
    // std::string hops_epoch_start = {"1970-01-01 00:00:00.000001"};
    std::string hops_epoch_start = J2000_TAI_EPOCH;
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

    //std::string j2000 = "2000-01-01T00:00:00.0Z";
    std::string j2000 = "2000-01-01T11:59:27.816Z";
    frmt = "%FT%TZ";
    date::tai_time<std::chrono::nanoseconds> j2000_tai_tp;
    std::istringstream ss2(j2000);
    std::istream stream(ss2.rdbuf());
    date::from_stream(stream, frmt.c_str(), j2000_tai_tp);
    auto j2000_utc_tp =  std::chrono::time_point_cast<std::chrono::nanoseconds>( date::tai_clock::to_utc( j2000_tai_tp ) );
    std::cout<<"J2000-utc, ns since epoch = "<< j2000_utc_tp.time_since_epoch().count() <<std::endl;
    std::cout<<"J2000-tai, ns since epoch = "<< j2000_tai_tp.time_since_epoch().count() <<std::endl;



    auto hops_tp = hops_clock::from_utc(j2000_utc_tp);
    std::cout<<"J2000-hops, ns since epoch = "<<hops_tp.time_since_epoch().count()<<std::endl;

    date::tai_time<std::chrono::nanoseconds> tai_test_tp;
    tai_test_tp = hops_clock::to_tai(hops_tp);

    date::gps_time<std::chrono::nanoseconds> gps_test_tp;
    gps_test_tp = hops_clock::to_gps(hops_tp);

    std::cout<<"hops epoch start in utc time = "<<j2000_utc_tp<<std::endl;
    std::cout<<"hops epoch start time = "<<hops_tp<<std::endl;

    std::cout<<"hops epoch as tai time = "<<tai_test_tp<<std::endl;
    std::cout<<"hops epoch as gps time = "<<gps_test_tp<<std::endl;
    

    std::string test = "2020-12-31T11:59:07.010816Z";
    frmt = "%FT%TZ";
    std::istringstream ss3(test);
    std::istream stream3(ss3.rdbuf());
    from_stream(stream3, frmt.c_str(), hops_tp);
    std::cout<<"hops test time = "<< hops_tp <<std::endl;

    std::cout << day_of_year( sys_days( floor<date::days>( hops_clock::to_sys(hops_tp) )  ) )<< '\n';
    
    std::string test_string = hops_clock::to_iso8601_format(hops_tp);
    std::cout<<"test string = "<<test_string<<std::endl;

    auto hops_tp2 = hops_clock::from_iso8601_format(test_string);

    std::cout<<"hops tp from test string = "<<hops_tp2<<std::endl;

    std::string vex_str = hops_clock::to_vex_format(hops_tp2);

    std::cout<<"hops test tp in vex format = "<<vex_str<<std::endl;

    std::string hops_str = hops_clock::to_hops_raw_format(hops_tp2);

    std::cout<<"hops raw time point string = "<<hops_str<<std::endl;

    auto hops_tp3 = hops_clock::from_hops_raw_format(hops_str);

    std::cout<<"hops tp from raw string = "<<hops_tp3<<std::endl;

    return 0;
}
