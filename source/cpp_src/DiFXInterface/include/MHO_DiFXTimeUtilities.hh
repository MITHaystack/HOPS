#ifndef MHO_DiFXTimeUtilities_HH__
#define MHO_DiFXTimeUtilities_HH__


#include "MHO_Clock.hh"

//this is the nominal DiFX MJD epoch start...however, it will be off by however
//many leap seconds have been inserted between this time and the time point of
//interest...so when UTC times are calculated from DiFX MJD values, this epoch
//start must be corrected by the number of leap seconds inserted (total of 5 as of 2023)
#define DIFX_J2000_MJD_EPOCH_UTC_ISO8601 "2000-01-01T12:00:00.000000000Z"
#define DIFX_J2000_MJD_EPOCH_OFFSET 51544.50000

namespace hops
{

//given a mjd date and number of seconds, compute the vex string representation
static std::string get_vexdate_from_mjd_sec(double mjd, double sec)
{
    double total_mjd = (double)mjd + (double)sec / 86400.0;
    auto difx_mjd_epoch = hops_clock::from_iso8601_format(DIFX_J2000_MJD_EPOCH_UTC_ISO8601);
    //figure out the approximate time of this mjd (ignoring leap seconds)
    auto approx_tp = hops_clock::from_mjd(difx_mjd_epoch, DIFX_J2000_MJD_EPOCH_OFFSET, total_mjd);
    //calculate the number of leap seconds
    auto n_leaps = hops_clock::get_leap_seconds_between(difx_mjd_epoch, approx_tp);
    //now correct the nominal difx start epoch to deal with the number of leap seconds
    auto actual_epoch_start = difx_mjd_epoch + n_leaps;
    msg_debug("difx_interface", "the leap-second corrected difx mjd epoch start is: "
                                    << hops_clock::to_iso8601_format(actual_epoch_start) << eom);
    auto mjd_tp = hops_clock::from_mjd(actual_epoch_start, DIFX_J2000_MJD_EPOCH_OFFSET, total_mjd);
    msg_debug("difx_interface",
              "the MJD value: " << total_mjd << " as a vex timestamp is: " << hops_clock::to_vex_format(mjd_tp) << eom);
    return hops_clock::to_vex_format(mjd_tp);
}

}


#endif /* end of include guard: MHO_DiFXTimeUtilities_HH__ */
