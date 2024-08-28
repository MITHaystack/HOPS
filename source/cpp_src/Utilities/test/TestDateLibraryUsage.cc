#include <iostream>
#include <chrono>
#include <istream>


#include "date/date.h"
#include "date/tz.h"

using namespace date;
using namespace std;
using namespace std::chrono;

#include "MHO_Clock.hh"


/*
From wikipedia -- TODO: find an actual source for this!!

The currently-used standard epoch "J2000" is defined by international agreement to be equivalent to:

The Gregorian date January 1, 2000, at 12:00 TT (Terrestrial Time).
The Julian date 2451545.0 TT (Terrestrial Time).[9]
January 1, 2000, 11:59:27.816 TAI (International Atomic Time).[10]
January 1, 2000, 11:58:55.816 UTC (Coordinated Universal Time).[b]
*/


#define DIFX_J2000_MJD_EPOCH_UTC_ISO8601 "2000-01-01T12:00:00.000000000Z"
#define DIFX_J2000_MJD_EPOCH_OFFSET 51544.50000


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

    std::string hops_str = hops_clock::to_hops_format(hops_tp2);

    std::cout<<"hops-formated time-point string = "<<hops_str<<std::endl;

    auto hops_tp3 = hops_clock::from_hops_format(hops_str);

    std::cout<<"hops time-point converted back from hops-formatted string = "<<hops_tp3<<std::endl;

    auto hops_tp4 = hops_clock::from_vex_format(vex_str);

    std::cout<<"hops time-point converted back from vex string = "<<hops_tp4<<std::endl;

    legacy_hops_date ldate = hops_clock::to_legacy_hops_date( hops_tp4 );

    std::cout<<"hops time-point converted to legacy hops-date-struct: "<<std::endl;
    std::cout<<"year = "<<ldate.year<<std::endl;
    std::cout<<"date = "<<ldate.day<<std::endl;
    std::cout<<"hour = "<<ldate.hour<<std::endl;
    std::cout<<"mins = "<<ldate.minute<<std::endl;
    std::cout<<"secs = "<< std::setprecision(9) <<ldate.second<<std::endl;

    std::string vtest = "2020y63d12h08m04s";

    std::cout<<"converting the following vex date/time: "<<vtest<<std::endl;

    std::cout<<"to hops date in iso-8601 format: "<<hops_clock::to_iso8601_format( hops_clock::from_vex_format(vtest) )<<std::endl;

    auto hops_tp5 = hops_clock::from_vex_format(vtest);

    std::cout<<"hops tp5, time since epoch = "<<hops_tp5.time_since_epoch().count()<<std::endl;

    // auto utc_tp1 = hops_clock::to_utc( hops_tp5 );
    // auto utc_tp0 = hops_clock::get_hops_epoch_utc();

    auto tp1 = hops_tp5;
    auto tp0 = hops_clock::get_hops_epoch();

    // auto lp_info0  = date::get_leap_second_info(utc_tp0);
    // auto lp_info1 = date::get_leap_second_info(utc_tp1);
    //
    // std::cout<<"leap seconds elapsed at epoch start: "<<lp_info0.elapsed.count()<<std::endl;
    // std::cout<<"leap seconds elapsed at test time: "<<lp_info1.elapsed.count()<<std::endl;
    // std::cout<<"leaps seconds added in between: "<<lp_info1.elapsed.count() - lp_info0.elapsed.count()<<std::endl;

    auto n_leaps = hops_clock::get_leap_seconds_between(tp0, tp1);

    std::cout<<"n leap seconds inserted between hops (J2000) epoch and: "<<vtest<<" = "<<n_leaps.count()<<std::endl;

    std::cout<<"number of leap seconds in tzdb: "<<get_tzdb().leap_seconds.size()<<" -> should be 27 (as of 9/12/2023)"<<std::endl;

    auto nom_difx_epoch = hops_clock::from_iso8601_format(DIFX_J2000_MJD_EPOCH_UTC_ISO8601);

    double mjd = hops_clock::to_mjd(nom_difx_epoch, DIFX_J2000_MJD_EPOCH_OFFSET, hops_tp5);

    std::cout<<vtest<<" as difx mjd  = "<<mjd<<std::endl;

    int vdif_epoch;
    int vdif_secs;

    hops_clock::to_vdif_format(hops_now, vdif_epoch, vdif_secs);
    std::cout<<"current time now vdif epoch-secs = "<<vdif_epoch<<"-"<<vdif_secs<<std::endl;

    hops_clock::to_vdif_format(hops_tp5, vdif_epoch, vdif_secs);
    std::cout<<"time: "<<vtest<<", as vdif epoch-secs = "<<vdif_epoch<<"-"<<vdif_secs<<std::endl;

    //test the conversion of a random date
    std::string test2 = "2024-07-01T00:00:00.0Z";
    auto hops_tp6 = hops_clock::from_iso8601_format(test2);

    hops_clock::to_vdif_format(hops_tp6, vdif_epoch, vdif_secs);
    std::cout<<"time: "<<test2<<", as vdif epoch-secs = "<<vdif_epoch<<"-"<<vdif_secs<<std::endl;

    //test the conversion of a random date
    test2 = "2024-12-31T23:59:59.0Z";
    hops_tp6 = hops_clock::from_iso8601_format(test2);

    hops_clock::to_vdif_format(hops_tp6, vdif_epoch, vdif_secs);
    std::cout<<"time: "<<test2<<", as vdif epoch-secs = "<<vdif_epoch<<"-"<<vdif_secs<<std::endl;

    auto hops_tp7 = hops_clock::from_vdif_format(vdif_epoch, vdif_secs);

    std::string vdif_date = hops_clock::to_iso8601_format(hops_tp7);
    std::cout<<"vdif: "<<vdif_epoch<<"@"<<vdif_secs<<" = "<<vdif_date<<std::endl;


    return 0;
}
