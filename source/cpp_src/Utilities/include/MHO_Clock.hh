#ifndef MHO_Clock_HH__
#define MHO_Clock_HH__

#include <chrono>
#include <iomanip>
#include <math.h>
#include <sstream>
#include <string>

//these can someday be replaced with STL versions in C++20
#include "date/date.h"
#include "date/tz.h"

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "legacy_hops_date.hh"

#define J2000_TAI_EPOCH "2000-01-01 11:59:27.816"
#define ISO8601_UTC_FORMAT "%FT%TZ"
#define HOPS_TIMESTAMP_PREFIX "HOPS-J2000"
#define HOPS_TIME_DELIM "|"
#define HOPS_TIME_UNIT "ns"
#define NANOSEC_TO_SEC 1e-9
#define SEC_TO_NANOSEC 1000000000
#define JD_TO_SEC 86400.0
#define MINUTE_TO_SEC 60.0
#define HOUR_TO_SEC 3600.0

namespace hops
{

/*!
 *@file  MHO_Clock.hh
 *@class  hops_clock
 *@author  J. Barrett - barrettj@mit.edu
 *@brief  a clock for hops-time stamps, measures time in (UTC) nanoseconds since J2000 epoch.
 * Functionality modelled on the gps_clock from the 'date' library (requires the timezone tz library too).
 * Relys on the H. Hinnant date library (and tzdata), this library is expected to be adopted into cxx-20 standard
 *@date Wed May 18 16:46:16 2022 -0400
 */

class hops_clock
{
    public:
        using duration = std::chrono::nanoseconds;
        using rep = duration::rep;
        using period = duration::period;
        using time_point = std::chrono::time_point< hops_clock, std::chrono::nanoseconds >;
        static const bool is_steady = false;

        /**
         * @brief Returns current time as a time_point using hops_clock's epoch.
         *
         * @return time_point representing current time in hops_clock's epoch.
         * @note This is a static function.
         */
        static time_point now();

        /**
         * @brief Converts a time point to UTC using hops_clock and Duration.
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< hops_clock, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< date::utc_clock, typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        to_utc(const std::chrono::time_point< hops_clock, Duration >&) NOEXCEPT;

        /**
         * @brief Converts UTC time point to hops_clock time point
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< date::utc_clock, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< hops_clock, typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        from_utc(const std::chrono::time_point< date::utc_clock, Duration >&) NOEXCEPT;

        /**
         * @brief Converts a time point from hops_clock to TAI (International Atomic Time).
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< hops_clock, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< date::tai_clock, typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        to_tai(const std::chrono::time_point< hops_clock, Duration >&) NOEXCEPT;

        /**
         * @brief Converts a TAI time point to UTC and returns the corresponding Hops clock time.
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< date::tai_clock, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< hops_clock, typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        from_tai(const std::chrono::time_point< date::tai_clock, Duration >&) NOEXCEPT;

        /**
         * @brief Converts a time point to GPS clock time.
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< hops_clock, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< date::gps_clock, typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        to_gps(const std::chrono::time_point< hops_clock, Duration >&) NOEXCEPT;

        /**
         * @brief Converts GPS time to Hops clock time.
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< date::gps_clock, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< hops_clock, typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        from_gps(const std::chrono::time_point< date::gps_clock, Duration >&) NOEXCEPT;

        /**
         * @brief Converts a time point from hops_clock to system clock.
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< hops_clock, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< std::chrono::system_clock,
                                        typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        to_sys(const std::chrono::time_point< hops_clock, Duration >&) NOEXCEPT;

        /**
         * @brief Converts a system time point to UTC and returns the corresponding hops clock time.
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< std::chrono::system_clock, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< hops_clock, typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        from_sys(const std::chrono::time_point< std::chrono::system_clock, Duration >&) NOEXCEPT;

        /**
         * @brief Converts a global time point to local time.
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< hops_clock, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< date::local_t, typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        to_local(const std::chrono::time_point< hops_clock, Duration >&) NOEXCEPT;

        /**
         * @brief Calculates time difference between input local time and Hops epoch in UTC.
         *
         * @tparam Duration Template parameter Duration
         * @param & Parameter & of type const std::chrono::time_point< date::local_t, Duration
         */
        template< typename Duration >
        static std::chrono::time_point< hops_clock, typename std::common_type< Duration, std::chrono::nanoseconds >::type >
        from_local(const std::chrono::time_point< date::local_t, Duration >&) NOEXCEPT;

        /**
         * @brief Converts an ISO8601 formatted timestamp string to a hops_clock time_point object.
         *
         * @param timestamp Input timestamp string in ISO8601 format
         * @return Time point object representing the parsed timestamp
         * @note This is a static function.
         */
        static time_point from_iso8601_format(const std::string& timestamp);

        /**
         * @brief Converts a time_point to ISO8601 formatted string.
         *
         * @param tp Input time_point to be converted
         * @return ISO8601 formatted string representation of tp
         * @note This is a static function.
         */
        static std::string to_iso8601_format(const time_point& tp);

        /**
         * @brief Converts a timestamp string in HOPS format to a time_point object.
         *
         * @param timestamp Input timestamp string in HOPS format.
         * @return time_point representing the parsed timestamp or epoch start if parsing fails.
         * @note This is a static function.
         */
        static time_point from_hops_format(const std::string& timestamp);

        /**
         * @brief Converts a time_point to HOPS format string.
         *
         * @param tp Input time_point to convert
         * @return String representation of tp in HOPS format
         * @note This is a static function.
         */
        static std::string to_hops_format(const time_point& tp);

        /**
         * @brief Converts a legacy hops data struct to a hops_clock time_point
         *
         * @param ldate (legacy_hops_date)
         * @return time_point
         * @note This is a static function.
         */
        static time_point from_legacy_hops_date(legacy_hops_date& ldate);

        /**
         * @brief Converts a hops_clock time_point to a legacy hops data struct
         *
         * @param tp (time_point)
         * @return legacy_hops_date struct
         * @note This is a static function.
         */
        static legacy_hops_date to_legacy_hops_date(const time_point& tp);

        /**
         * @brief Converts a VDIF (epoch, second) timestamp to a hops_clock time_point
         *
         * @param vdif_epoch (int)
         * @param vdif_seconds (int)
         * @return time_point
         * @note This is a static function.
         */
        static time_point from_vdif_format(int& vdif_epoch, int& vdif_seconds);

        /**
         * @brief Converts a hops_clock time_point to a VDIF (epoch, second) timestamp
         *
         * @param tp (time_point)
         * @param reference to vdif_epoch (int&)
         * @param reference to vdif_seconds (int&)
         * @return time_point
         * @note This is a static function.
         */
        static void to_vdif_format(const time_point& tp, int& vdif_epoch, int& vdif_second);

        /**
         * @brief Converts a Modified Julian date (floating point epoch and day) timestamp to a hops_clock time_point
         *
         * @param mjd_epoch (time_point)
         * @param epoch_offset (double&)
         * @param mjd (double&)
         * @return time_point
         * @note This is a static function.
         */
        static time_point from_mjd(const time_point& mjd_epoch, const double& epoch_offset, const double& mjd);

        /**
         * @brief Converts a hops_clock time_point to a  Modified Julian date (floating point day) timestamp, givent the specified epoch
         *
         * @param mjd_epoch (time_point)
         * @param epoch_offset (double&)
         * @param tp (time_point&)
         * @return MJD (double)
         * @note This is a static function.
         */
        static double to_mjd(const time_point& mjd_epoch, const double& epoch_offset, const time_point& tp);

        /**
         * @brief Converts a VEX-style formatted string (e.g. 2019y106d18h30m15s) to a hops_clock time_point
         *
         * @param timestamp VEX formatted string representation of time
         * @return time_point
         * @note This is a static function.
         */
        static time_point from_vex_format(const std::string& timestamp);

        /**
         * @brief Converts a hops_clock time_point to VEX-style formatted string (e.g. 2019y106d18h30m15s)
         *
         * @param tp Input time_point to be converted
         * @param truncate_to_nearest_second (bool) optionally truncate the time-point to the nearest second
         * @return VEX formatted string representation of tp
         * @note This is a static function.
         */
        static std::string to_vex_format(const time_point& tp, bool truncate_to_nearest_second = false);

        /**
         * @brief Converts a year + floating point day since start of the year to a hops_clock time_point,
         * needed for ad_hoc flag files (time-stamps are given in floating-point days)
         *
         * @param year (int)
         * @param floating_point_days (double) - days since start of the year
         * @return time_point
         * @note This is a static function.
         */
        static time_point from_year_fpday(int year, double floating_point_days);

        /**
         * @brief Converts a hops_clock time_point to a floating point day since start of the year
         * needed for ad_hoc flag files (time-stamps are given in floating-point days)
         *
         * @param tp Input time_point to be converted
         * @param year (int&)
         * @param floating_point_days (double&) - days since start of the year
         * @note This is a static function.
         */
        static void to_year_fpday(const time_point& tp, int& year, double& floating_point_days);

        /**
         * @brief returns the hops_clock epoch as a utc_time time_point
         */
        static date::utc_time< std::chrono::nanoseconds > get_hops_epoch_utc()
        {
            std::string frmt = "%F %T";
            std::string j2000 = J2000_TAI_EPOCH;
            date::tai_time< std::chrono::nanoseconds > j2000_tai_epoch;
            std::istringstream ss(j2000);
            std::istream stream(ss.rdbuf());
            date::from_stream(stream, frmt.c_str(), j2000_tai_epoch);
            return std::chrono::time_point_cast< std::chrono::nanoseconds >(date::tai_clock::to_utc(j2000_tai_epoch));
        }

        /**
         * @brief returns the hops_clock epoch as a hops_clock time_point
         */
        static time_point get_hops_epoch() { return from_utc(get_hops_epoch_utc()); }

        /**
         * @brief calculates the number of leap seconds inserted between two hops time points (UTC based clock)
         */
        static std::chrono::seconds get_leap_seconds_between(const time_point& t_start, const time_point& t_end)
        {
            auto t_start_utc = to_utc(t_start);
            auto t_end_utc = to_utc(t_end);
            auto lp_info0 = date::get_leap_second_info(t_start_utc);
            auto lp_info1 = date::get_leap_second_info(t_end_utc);
            int delta = lp_info1.elapsed.count() - lp_info0.elapsed.count();
            return std::chrono::seconds(delta);
        }

    private:
        static date::days day_of_year(date::sys_days sd)
        {
            using namespace date;
            auto y = date::year_month_day{sd}.year();
            return sd - date::sys_days{y / jan / 0};
        }

        static date::sys_days get_year_month_day(date::year y, date::days ord_day)
        {
            using namespace date;
            return date::sys_days{y / jan / 0} + ord_day;
        }

        struct vex_date
        {
                int year;
                int day_of_year;
                int hours;
                int minutes;
                double seconds;
        };

        static vex_date extract_vex_date(const std::string& timestamp);

        static std::string vex_date_to_iso8601_string(vex_date vdate);

        static vex_date vex_date_from_legacy(const legacy_hops_date& legacy_date);

        static std::string remove_trailing_zeros(std::string value)
        {
            std::size_t nzeros_on_end = 0;
            for(auto rit = value.rbegin(); rit != value.rend(); rit++)
            {
                if(*rit != '0')
                {
                    break;
                }
                nzeros_on_end++;
            }
            std::size_t useful_length = value.size() - nzeros_on_end;
            std::string ret_val;
            for(std::size_t i = 0; i < useful_length; i++)
            {
                ret_val.push_back(value[i]);
            }
            return ret_val;
        }
};

/**
 * @brief Class hops_time
 */
template< class Duration > using hops_time = std::chrono::time_point< hops_clock, Duration >;

/**
 * @brief Converts a time duration to UTC using Hops clock epoch.
 *
 * @tparam Duration Template parameter Duration
 * @param t Input time duration in specified Duration.
 */
template< class Duration >
inline date::utc_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::to_utc(const hops_time< Duration >& t) NOEXCEPT
{

    using CD = typename std::common_type< Duration, std::chrono::nanoseconds >::type;
    date::utc_time< std::chrono::nanoseconds > hops_epoch_start = get_hops_epoch_utc();
    return date::utc_time< CD >(t.time_since_epoch() + hops_epoch_start.time_since_epoch());
}

/**
 * @brief Converts UTC time to hops clock time.
 *
 * @tparam Duration Template parameter Duration
 * @param t Input UTC time.
 */
template< class Duration >
inline hops_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::from_utc(const date::utc_time< Duration >& t) NOEXCEPT
{
    using CD = typename std::common_type< Duration, std::chrono::nanoseconds >::type;
    date::utc_time< std::chrono::nanoseconds > hops_epoch_start = get_hops_epoch_utc();
    return hops_time< CD >(t.time_since_epoch() - hops_epoch_start.time_since_epoch());
}

/**
 * @brief Converts a Hops time to TAI (International Atomic Time).
 *
 * @tparam Duration Template parameter Duration
 * @param t Input Hops time in UTC
 */
template< class Duration >
inline date::tai_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::to_tai(const hops_time< Duration >& t) NOEXCEPT
{
    return date::tai_clock::from_utc(to_utc(t));
}

/**
 * @brief Converts TAI time to UTC and updates hops clock.
 *
 * @tparam Duration Template parameter Duration
 * @param t Input TAI time
 */
template< class Duration >
inline hops_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::from_tai(const date::tai_time< Duration >& t) NOEXCEPT
{
    return from_utc(date::tai_clock::to_utc(t));
}

/**
 * @brief Converts hops_time to GPS time using UTC conversion.
 *
 * @tparam Duration Template parameter Duration
 * @param t Input hops_time with Duration template parameter
 */
template< class Duration >
inline date::gps_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::to_gps(const hops_time< Duration >& t) NOEXCEPT
{
    return date::gps_clock::from_utc(to_utc(t));
}

/**
 * @brief Converts GPS time to UTC and updates hops clock without exception.
 *
 * @tparam Duration Template parameter Duration
 * @param t Input GPS time in floating-point days.
 */
template< class Duration >
inline hops_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::from_gps(const date::gps_time< Duration >& t) NOEXCEPT
{
    return from_utc(date::gps_clock::to_utc(t));
}

/**
 * @brief Converts a hops_time to system time using UTC clock.
 *
 * @tparam Duration Template parameter Duration
 * @param t Input hops_time with Duration template parameter
 */
template< class Duration >
inline date::sys_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::to_sys(const hops_time< Duration >& t) NOEXCEPT
{
    return date::utc_clock::to_sys(to_utc(t));
}

/**
 * @brief Converts system time to UTC clock time and returns it.
 *
 * @tparam Duration Template parameter Duration
 * @param t Input system time in floating-point days.
 */
template< class Duration >
inline hops_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::from_sys(const date::sys_time< Duration >& t) NOEXCEPT
{
    return from_utc(date::utc_clock::from_sys(t));
}

/**
 * @brief Returns the current time point in HOPS clock format.
 *
 * @return Current time point as hops_clock::time_point.
 */
inline hops_clock::time_point hops_clock::now()
{
    return from_utc(date::utc_clock::now());
}

/**
 * @brief Converts a UTC time to local time using hops_clock.
 *
 * @tparam Duration Template parameter Duration
 * @param t Input UTC time in hops_time format
 */
template< class Duration >
inline date::local_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::to_local(const hops_time< Duration >& t) NOEXCEPT
{
    using CD = typename std::common_type< Duration, std::chrono::nanoseconds >::type;
    date::utc_time< CD > hops_epoch_start = std::chrono::time_point_cast< CD >(get_hops_epoch_utc());
    date::utc_time< CD > ut_time{t.time_since_epoch() +
                                 std::chrono::time_point_cast< Duration >(hops_epoch_start).time_since_epoch()};
    return date::utc_clock::to_local(ut_time);
}

/**
 * @brief Calculates time difference between two UTC time points considering leap seconds.
 *
 * @tparam Duration Template parameter Duration
 * @param t Input local time to convert to UTC.
 */
template< class Duration >
inline hops_time< typename std::common_type< Duration, std::chrono::nanoseconds >::type >
hops_clock::from_local(const date::local_time< Duration >& t) NOEXCEPT
{
    using CD = typename std::common_type< Duration, std::chrono::nanoseconds >::type;
    date::utc_time< CD > t2 = date::utc_clock::from_local(t);
    date::utc_time< CD > hops_epoch_start = std::chrono::time_point_cast< CD >(get_hops_epoch_utc());
    return hops_time< CD >{t2.time_since_epoch() -
                           std::chrono::time_point_cast< Duration >(hops_epoch_start).time_since_epoch()};
}

/**
 * @brief Converts hops_time to stream format using given fmt and outputs to os.
 *
 * @tparam CharT Template parameter CharT
 * @tparam Traits Template parameter Traits
 * @tparam Duration Template parameter Duration
 * @param os Output stream for formatted time
 * @param fmt Format string for time output
 * @param t Input hops_time object
 * @return Reference to the modified output stream
 */
template< class CharT, class Traits, class Duration >
std::basic_ostream< CharT, Traits >& to_stream(std::basic_ostream< CharT, Traits >& os, const CharT* fmt,
                                               const hops_time< Duration >& t)
{
    const std::string abbrev("HOPS");
    CONSTDATA std::chrono::seconds offset{0};
    return date::to_stream(os, fmt, hops_clock::to_local(t), &abbrev, &offset);
}

/**
 * @brief Reads time and abbreviation from stream using given format, updates hops_time if successful.
 *
 * @tparam Duration Template parameter Duration
 * @tparam CharT Template parameter CharT
 * @tparam Traits Template parameter Traits
 * @tparam Alloc Template parameter Alloc
 * @param is Input stream to read from
 * @param fmt Format string for reading time
 * @param tp hops_time object to update with parsed time
 * @param abbrev Optional pointer to std::basic_string for abbreviation (default nullptr)
 * @param offset Optional pointer to std::chrono::minutes for offset (default nullptr)
 */
template< class Duration, class CharT, class Traits, class Alloc = std::allocator< CharT > >
std::basic_istream< CharT, Traits >&
from_stream(std::basic_istream< CharT, Traits >& is, const CharT* fmt, hops_time< Duration >& tp,
            std::basic_string< CharT, Traits, Alloc >* abbrev = nullptr, std::chrono::minutes* offset = nullptr)
{
    date::local_time< Duration > lp;
    from_stream(is, fmt, lp, abbrev, offset);
    if(!is.fail())
        tp = hops_clock::from_local(lp);
    return is;
}

template< class CharT, class Traits, class Duration >
std::basic_ostream< CharT, Traits >& operator<<(std::basic_ostream< CharT, Traits >& os, const hops_time< Duration >& t)
{
    const CharT fmt[] = {'%', 'F', 'T', '%', 'T', 'Z', CharT{}};
    return to_stream(os, fmt, t);
}

/**
 * @brief Converts an ISO8601 formatted timestamp string to hops_clock::time_point.
 *
 * @param timestamp Input timestamp string in ISO8601 format
 * @return hops_clock::time_point representing the parsed timestamp
 */
inline hops_clock::time_point hops_clock::from_iso8601_format(const std::string& timestamp)
{
    using namespace date;
    using namespace std::chrono;
    std::string frmt = ISO8601_UTC_FORMAT;
    std::istringstream ss(timestamp);
    std::istream tmp_stream(ss.rdbuf());
    hops_time< std::chrono::nanoseconds > hops_tp;
    from_stream(tmp_stream, frmt.c_str(), hops_tp);
    return hops_tp;
}

/**
 * @brief Converts a time_point to ISO8601 formatted string.
 *
 * @param tp Input time_point to be converted
 * @return ISO8601 formatted string representation of tp
 */
inline std::string hops_clock::to_iso8601_format(const time_point& tp)
{
    std::stringstream ss;
    ss << tp;
    return ss.str();
}

/**
 * @brief Function hops_clock::from_hops_format
 *
 * @param timestamp (const std::string&)
 * @return Return value (hops_clock::time_point)
 */
inline hops_clock::time_point hops_clock::from_hops_format(const std::string& timestamp)
{
    using namespace date;
    using namespace std::chrono;

    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter(std::string(HOPS_TIME_DELIM));
    std::vector< std::string > tokens;
    tokenizer.SetString(&timestamp);
    tokenizer.GetTokens(&tokens);
    if(tokens.size() == 3)
    {
        std::string hops_prefix = tokens[0];
        std::string unit = tokens[1];
        std::string nanosecond_count = tokens[2];
        if(hops_prefix == std::string(HOPS_TIMESTAMP_PREFIX) && unit == std::string(HOPS_TIME_UNIT))
        {
            std::stringstream ss;
            ss << nanosecond_count;
            int64_t ns;
            ss >> ns;
            return time_point(std::chrono::nanoseconds(ns));
        }
    }
    msg_error("utility", "hops timestamp string not understood or supported, returning epoch start. " << eom);
    return time_point(std::chrono::nanoseconds(0));
}

/**
 * @brief Converts a time_point to HOPS format string.
 *
 * @param tp Input time_point to convert
 * @return String representation of tp in HOPS format
 */
inline std::string hops_clock::to_hops_format(const time_point& tp)
{
    std::stringstream ss;
    ss << HOPS_TIMESTAMP_PREFIX;
    ss << HOPS_TIME_DELIM;
    ss << HOPS_TIME_UNIT;
    ss << HOPS_TIME_DELIM;
    ss << tp.time_since_epoch().count();
    return ss.str();
}

inline hops_clock::time_point hops_clock::from_legacy_hops_date(legacy_hops_date& ldate)
{
    vex_date vdate = vex_date_from_legacy(ldate);
    std::string vex_as_iso8601 = vex_date_to_iso8601_string(vdate);
    return hops_clock::from_iso8601_format(vex_as_iso8601);
}

inline legacy_hops_date hops_clock::to_legacy_hops_date(const time_point& tp)
{
    using namespace date;
    using namespace std::chrono;

    //convert the time point to sys time, and extract the date
    auto sys_tp = hops_clock::to_sys(tp);
    auto dp = date::sys_days(floor< date::days >(sys_tp));

    //get all of the date information
    date::year_month_day ymd{dp};
    auto year = ymd.year();

    //get the ordinal day of the year
    auto ordinal_day = day_of_year(dp);

    //get the time
    date::hh_mm_ss< std::chrono::nanoseconds > time{floor< std::chrono::nanoseconds >(sys_tp - dp)};
    auto hours = time.hours();
    auto mins = time.minutes();
    auto secs = time.seconds();
    auto nanos = time.subseconds();

    legacy_hops_date ldate;
    ldate.year = (int)year;
    ldate.day = ordinal_day.count();
    ldate.hour = hours.count();
    ldate.minute = mins.count();
    //note there may be loss of precision when converting to/from the actual legacy struct (single precision)
    ldate.second = (double)secs.count() + ((double)(nanos.count())) * NANOSEC_TO_SEC;

    return ldate;
}

//needed for ad_hoc flag files (time-stamps are given in floating-point days)
inline hops_clock::time_point hops_clock::from_year_fpday(int year, double floating_point_days)
{
    int integer_days = (int)floating_point_days;
    double fractional_day = floating_point_days - integer_days;
    int integer_hours = (int)24 * fractional_day;
    double fractional_hour = 24 * fractional_day - integer_hours;
    int integer_minutes = (int)60 * fractional_hour;
    double fractional_seconds = (60 * fractional_hour - integer_minutes) * 60;

    //we co-opt the legacy date format to handle this format
    legacy_hops_date ldate;
    ldate.year = (short)year;
    ldate.day = (short)integer_days + 1; //note: ordinal day count starts at 1
    ldate.hour = (short)integer_hours;
    ldate.minute = (short)integer_minutes;
    ldate.second = fractional_seconds;

    return from_legacy_hops_date(ldate);
}

inline void hops_clock::to_year_fpday(const hops_clock::time_point& tp, int& year, double& floating_point_days)
{
    using namespace date;
    using namespace std::chrono;

    //convert the time point to sys time, and extract the date
    auto sys_tp = hops_clock::to_sys(tp);
    auto dp = sys_days(floor< date::days >(sys_tp));

    //get all of the date information
    year_month_day ymd{dp};
    auto year_value = ymd.year();
    //get the ordinal day of the year
    auto ordinal_day = day_of_year(dp); //note: count starts at 1
    int integer_days = ordinal_day.count() - 1;
    //get the time and convert to fractional day
    hh_mm_ss< std::chrono::nanoseconds > time{floor< std::chrono::nanoseconds >(sys_tp - dp)};
    int ihours = time.hours().count();
    int imins = time.minutes().count();
    int isecs = time.seconds().count();
    int inanos = time.subseconds().count();

    double frac_day = (inanos * NANOSEC_TO_SEC + isecs + MINUTE_TO_SEC * imins + HOUR_TO_SEC * ihours) / (JD_TO_SEC);
    floating_point_days = integer_days + frac_day;
}

inline hops_clock::time_point hops_clock::from_mjd(const time_point& mjd_epoch, const double& epoch_offset, const double& mjd)
{
    double delta = (mjd - epoch_offset);
    delta *= JD_TO_SEC;
    std::chrono::duration< double > duration_seconds(delta);

    auto mjd_epoch_utc = to_utc(mjd_epoch);
    auto utc_time_point = mjd_epoch_utc + std::chrono::duration_cast< std::chrono::nanoseconds >(duration_seconds);
    auto hops_time_point = from_utc(utc_time_point);
    return hops_time_point;
}

inline double hops_clock::to_mjd(const time_point& mjd_epoch, const double& epoch_offset, const time_point& tp)
{
    auto mjd_epoch_utc = to_utc(mjd_epoch);
    auto tp_utc = to_utc(tp);

    double delta = (tp_utc - mjd_epoch_utc).count();
    delta *= NANOSEC_TO_SEC; //convert to seconds
    delta /= JD_TO_SEC;      //convert to days
    delta += epoch_offset;   //subtract epoch offset
    return delta;
}

inline hops_clock::time_point hops_clock::from_vex_format(const std::string& timestamp)
{
    vex_date vdate = hops_clock::extract_vex_date(timestamp);
    //convert the vex date info to an ISO-8601-style year-month-day type format
    std::string vex_as_iso8601 = vex_date_to_iso8601_string(vdate);
    return hops_clock::from_iso8601_format(vex_as_iso8601);
}

inline std::string hops_clock::to_vex_format(const time_point& tp, bool truncate_to_nearest_second)
{
    using namespace date;
    using namespace std::chrono;

    //convert the time point to sys time, and extract the date
    auto sys_tp = hops_clock::to_sys(tp);
    auto dp = sys_days(floor< date::days >(sys_tp));

    //get all of the date information
    year_month_day ymd{dp};
    auto year = ymd.year();
    // auto month = ymd.month();
    // auto day = ymd.day();

    //get the ordinal day of the year
    auto ordinal_day = day_of_year(dp);

    //get the time
    hh_mm_ss< std::chrono::nanoseconds > time{floor< std::chrono::nanoseconds >(sys_tp - dp)};
    auto hours = time.hours();
    auto mins = time.minutes();
    auto secs = time.seconds();
    auto nanos = time.subseconds();

    //we need to make sure that year, day-of-year, hour, minute, and integer sec
    //are all preprended with the proper number of zeros

    std::stringstream ss;
    ss << year;
    ss << "y";
    ss << std::setfill('0') << std::setw(3) << ordinal_day.count();
    ss << "d";
    ss << std::setfill('0') << std::setw(2) << hours.count();
    ss << "h";
    ss << std::setfill('0') << std::setw(2) << mins.count();
    ss << "m";
    ss << std::setfill('0') << std::setw(2) << secs.count();

    if(!truncate_to_nearest_second)
    {
        std::stringstream nss;
        nss << std::setfill('0') << std::setw(9) << nanos.count();
        std::string snano_sec;
        nss >> snano_sec;
        std::string trimmed_nanosec = remove_trailing_zeros(snano_sec);
        if(trimmed_nanosec.size() != 0)
        {
            ss << ".";
            ss << trimmed_nanosec;
        }
    }
    ss << "s";

    return ss.str();
}

inline void hops_clock::to_vdif_format(const time_point& tp, int& vdif_epoch, int& vdif_seconds)
{
    using namespace date;
    using namespace std::chrono;

    //convert the time point to sys time, and extract the date
    auto sys_tp = hops_clock::to_sys(tp);
    auto dp = sys_days(floor< date::days >(sys_tp));

    //get all of the date information so we can figure out the epoch
    year_month_day ymd{dp};
    auto year = ymd.year();
    auto month = ymd.month();
    auto day = ymd.day();

    //(we have two 6 month epochs per year, and count from start of century)
    int iyear = static_cast< int >(year);
    unsigned int imonth = static_cast< unsigned int >(month);
    int epoch = (iyear % 100) * 2;

    //now figure out if we are using the Jan 1st epoch, or the July 1st epoch
    if(imonth < 7)
    {
        imonth = 1;
    }
    else
    {
        epoch += 1;
        imonth = 7;
    }
    //set the day to the first of the month
    day = date::day(1);
    int hours = 0;
    int minutes = 0;
    int integer_sec = 0;

    //now figure out the epoch date
    //may want to eliminate string conversion in favor of something faster
    std::stringstream ss;
    ss << iyear;
    ss << "-";
    ss << std::setfill('0') << std::setw(2) << imonth;
    ss << "-";
    ss << std::setfill('0') << std::setw(2) << static_cast< unsigned int >(day);
    ss << "T";
    ss << std::setfill('0') << std::setw(2) << hours;
    ss << ":";
    ss << std::setfill('0') << std::setw(2) << minutes;
    ss << ":";
    ss << std::setfill('0') << std::setw(2) << integer_sec;
    ss << "Z";
    std::string epoch_iso8601 = ss.str();
    auto epoch_tp = from_iso8601_format(epoch_iso8601);
    int secs = std::chrono::duration_cast< std::chrono::seconds >(tp - epoch_tp).count();

    vdif_epoch = epoch;
    vdif_seconds = secs;
}

inline hops_clock::time_point hops_clock::from_vdif_format(int& vdif_epoch, int& vdif_seconds)
{
    using namespace date;
    using namespace std::chrono;

    int start_year = 2000;
    int n_years = std::floor(vdif_epoch / 2);
    int iyear = start_year + n_years;

    std::cout << "n_years = " << n_years << " iyear = " << iyear << std::endl;

    unsigned int imonth = 1;
    if(vdif_epoch % 2 == 1)
    {
        imonth = 7;
    } //second half of the year
    unsigned int iday = 1;
    int hours = 0;
    int minutes = 0;
    int integer_sec = 0;

    //now figure out the epoch date
    //may want to eliminate string conversion in favor of something faster
    std::stringstream ss;
    ss << iyear;
    ss << "-";
    ss << std::setfill('0') << std::setw(2) << imonth;
    ss << "-";
    ss << std::setfill('0') << std::setw(2) << iday;
    ss << "T";
    ss << std::setfill('0') << std::setw(2) << hours;
    ss << ":";
    ss << std::setfill('0') << std::setw(2) << minutes;
    ss << ":";
    ss << std::setfill('0') << std::setw(2) << integer_sec;
    ss << "Z";
    std::string epoch_iso8601 = ss.str();
    std::cout << "epoch = " << epoch_iso8601 << std::endl;
    auto epoch_tp = from_iso8601_format(epoch_iso8601);
    auto tp = epoch_tp + std::chrono::seconds(vdif_seconds);
    return tp;
}

inline hops_clock::vex_date hops_clock::extract_vex_date(const std::string& timestamp)
{
    vex_date vdate;
    if(timestamp.size() == 0)
    {
        msg_error("utilities", "cannot extract vex date from empty string." << eom);
        return vdate;
    }

    MHO_Tokenizer tokenizer;
    std::vector< std::string > tokens;
    std::stringstream ss;
    std::string rest;
    std::string syear, sord_day, shour, smin, ssec;

    tokenizer.SetDelimiter(std::string("y"));
    tokenizer.SetString(&timestamp);
    tokenizer.GetTokens(&tokens);

    syear = tokens[0];
    ss << syear;
    ss >> vdate.year;
    rest = tokens[1];

    tokenizer.SetDelimiter(std::string("d"));
    tokenizer.SetString(&rest);
    tokenizer.GetTokens(&tokens);

    sord_day = tokens[0];
    ss.str(std::string());
    ss.clear();
    ss << sord_day;
    ss >> vdate.day_of_year;
    rest = tokens[1];

    tokenizer.SetDelimiter(std::string("h"));
    tokenizer.SetString(&rest);
    tokenizer.GetTokens(&tokens);

    shour = tokens[0];
    ss.str(std::string());
    ss.clear();
    ss << shour;
    ss >> vdate.hours;
    rest = tokens[1];

    tokenizer.SetDelimiter(std::string("m"));
    tokenizer.SetString(&rest);
    tokenizer.GetTokens(&tokens);

    smin = tokens[0];
    ss.str(std::string());
    ss.clear();
    ss << smin;
    ss >> vdate.minutes;
    rest = tokens[1];

    tokenizer.SetDelimiter(std::string("s"));
    tokenizer.SetString(&rest);
    tokenizer.GetTokens(&tokens);

    ssec = tokens[0];
    ss.str(std::string());
    ss.clear();
    ss << std::setprecision(15) << ssec;
    ss >> vdate.seconds;

    return vdate;
}

inline std::string hops_clock::vex_date_to_iso8601_string(hops_clock::vex_date vdate)
{
    using namespace date;
    using namespace std::chrono;

    std::stringstream ss;
    ss << vdate.year;
    ss << "-";

    date::year y(vdate.year);
    date::days ord_day(vdate.day_of_year);
    date::sys_days ymd = get_year_month_day(y, ord_day);

    //convert day-of-year to month-day
    auto month = date::year_month_day{ymd}.month();
    auto mday = date::year_month_day{ymd}.day();
    ss << std::setfill('0') << std::setw(2) << (unsigned)month;
    ss << "-";
    ss << std::setfill('0') << std::setw(2) << (unsigned)mday;

    ss << "T";
    ss << std::setfill('0') << std::setw(2) << vdate.hours;
    ss << ":";
    ss << std::setfill('0') << std::setw(2) << vdate.minutes;
    ss << ":";

    //nss << std::setprecision(9) << vdate.seconds;

    double intpart;
    double frac = modf(vdate.seconds, &intpart);
    int integer_sec = intpart;

    ss << std::setfill('0') << std::setw(2) << integer_sec;

    //now convert the fraction part into integer nano seconds
    int integer_nanosec = frac * SEC_TO_NANOSEC;
    std::stringstream nss;
    nss << integer_nanosec;
    std::string nanoseconds_value = nss.str();
    std::string trimmed_int_nanosec = remove_trailing_zeros(nanoseconds_value);
    if(trimmed_int_nanosec.size() != 0)
    {
        ss << ".";
        ss << trimmed_int_nanosec;
    }
    ss << "Z";
    return ss.str();
}

inline hops_clock::vex_date hops_clock::vex_date_from_legacy(const legacy_hops_date& legacy_date)
{
    hops_clock::vex_date vdate;
    vdate.year = legacy_date.year;
    vdate.day_of_year = legacy_date.day;
    vdate.hours = legacy_date.hour;
    vdate.minutes = legacy_date.minute;
    vdate.seconds = legacy_date.second;
    return vdate;
}

} // namespace hops

#endif /*! end of include guard: MHO_Clock */
