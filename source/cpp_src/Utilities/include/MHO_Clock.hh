#ifndef MHO_Clock_HH__
#define MHO_Clock_HH__



#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <math.h>

//these can someday be replaced with STL versions in C++20
#include "date/date.h"
#include "date/tz.h"

#include "MHO_Tokenizer.hh"
#include "MHO_Message.hh"
#include "legacy_hops_date.hh"

#define J2000_TAI_EPOCH "2000-01-01 11:59:27.816"
#define ISO8601_UTC_FORMAT "%FT%TZ"
#define HOPS_TIMESTAMP_PREFIX "HOPS-J2000"
#define HOPS_TIME_DELIM "|"
#define HOPS_TIME_UNIT "ns"
#define NANOSEC_TO_SEC 1e-9
#define SEC_TO_NANOSEC 1000000000
#define JD_TO_SEC 86400.0

namespace hops
{

/*!
*@file  MHO_Clock.hh
*@class  hops_clock
*@author  J. Barrett - barrettj@mit.edu
*@brief  a clock for hops-time stamps, measures time in (UTC) nanoseconds since J2000 epoch.
*Functionality modelled on the gps_clock from the 'date' library (requires the timezone tz library too).
*@date Wed May 18 16:46:16 2022 -0400
*/

class hops_clock
{
    public:

        using duration                  = std::chrono::nanoseconds;
        using rep                       = duration::rep;
        using period                    = duration::period;
        using time_point                = std::chrono::time_point<hops_clock, std::chrono::nanoseconds>;
        static const bool is_steady     = false;

        static time_point now();

        template<typename Duration>
        static
        std::chrono::time_point<date::utc_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_utc(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_utc(const std::chrono::time_point<date::utc_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<date::tai_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_tai(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_tai(const std::chrono::time_point<date::tai_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<date::gps_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_gps(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_gps(const std::chrono::time_point<date::gps_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<std::chrono::system_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_sys(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_sys(const std::chrono::time_point<std::chrono::system_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<date::local_t, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_local(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_local(const std::chrono::time_point<date::local_t, Duration>&) NOEXCEPT;

        static
        time_point
        from_iso8601_format(const std::string& timestamp);

        static
        std::string
        to_iso8601_format(const time_point& tp);

        static
        time_point
        from_hops_format(const std::string& timestamp);

        static
        std::string
        to_hops_format(const time_point& tp);

        static
        time_point
        from_legacy_hops_date(legacy_hops_date& ldate);

        static
        legacy_hops_date
        to_legacy_hops_date(const time_point& tp);

        static
        time_point
        from_vex_format(const std::string& timestamp);


        static
        time_point
        from_mjd(const time_point& mjd_epoch, const double& epoch_offset, const double& mjd);

        static
        double
        to_mjd(const time_point& mjd_epoch, const double& epoch_offset, const time_point& tp);


        static
        std::string
        to_vex_format(const time_point& tp, bool truncate_to_nearest_second = false);

        static date::utc_time< std::chrono::nanoseconds > get_hops_epoch_utc()
        {
            std::string frmt = "%F %T";
            std::string j2000 = J2000_TAI_EPOCH;
            date::tai_time<std::chrono::nanoseconds> j2000_tai_epoch;
            std::istringstream ss(j2000);
            std::istream stream(ss.rdbuf());
            date::from_stream(stream, frmt.c_str(), j2000_tai_epoch);
            return std::chrono::time_point_cast<std::chrono::nanoseconds>( date::tai_clock::to_utc( j2000_tai_epoch ) );
        }

        static time_point get_hops_epoch()
        {
            return from_utc( get_hops_epoch_utc() );
        }

        //calculates the number of leap seconds inserted between two hops time points (UTC based clock)
        static std::chrono::seconds get_leap_seconds_between
        (
            const time_point& t_start,
            const time_point& t_end
        )
        {
            auto t_start_utc = to_utc(t_start);
            auto t_end_utc = to_utc(t_end);
            auto lp_info0  = date::get_leap_second_info(t_start_utc);
            auto lp_info1 = date::get_leap_second_info(t_end_utc);
            int delta = lp_info1.elapsed.count() - lp_info0.elapsed.count();
            return std::chrono::seconds(delta);
        }


    private:

        static
        date::days
        day_of_year(date::sys_days sd)
        {
            using namespace date;
            auto y = date::year_month_day{sd}.year();
            return sd - date::sys_days{y/jan/0};
        }

        static
        date::sys_days
        get_year_month_day(date::year y, date::days ord_day)
        {
            using namespace date;
            return date::sys_days{y/jan/0} + ord_day;
        }

        struct vex_date
        {
            int year;
            int day_of_year;
            int hours;
            int minutes;
            double seconds;
        };

        static
        vex_date extract_vex_date(const std::string& timestamp);

        static
        std::string vex_date_to_iso8601_string(vex_date vdate);

        static
        vex_date vex_date_from_legacy(const legacy_hops_date& legacy_date);

        static
        std::string remove_trailing_zeros(std::string value)
        {
            std::size_t nzeros_on_end = 0;
            for(auto rit = value.rbegin(); rit != value.rend(); rit++)
            {
                if( *rit != '0'){break;}
                nzeros_on_end++;
            }
            std::size_t useful_length = value.size() - nzeros_on_end;
            std::string ret_val;
            for(std::size_t i=0; i<useful_length; i++)
            {
                ret_val.push_back(value[i]);
            }
            return ret_val;
        }

};



template <class Duration>
using hops_time = std::chrono::time_point<hops_clock, Duration>;

template <class Duration>
inline
date::utc_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_utc(const hops_time<Duration>& t) NOEXCEPT
{

    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time< std::chrono::nanoseconds > hops_epoch_start = get_hops_epoch_utc();
    return date::utc_time<CD>(t.time_since_epoch() + hops_epoch_start.time_since_epoch());
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_utc(const date::utc_time<Duration>& t) NOEXCEPT
{
    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time< std::chrono::nanoseconds > hops_epoch_start = get_hops_epoch_utc();
    return hops_time<CD>(t.time_since_epoch() - hops_epoch_start.time_since_epoch());
}

template <class Duration>
inline
date::tai_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_tai(const hops_time<Duration>& t) NOEXCEPT
{
    return date::tai_clock::from_utc( to_utc(t) );
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_tai(const date::tai_time<Duration>& t) NOEXCEPT
{
    return from_utc( date::tai_clock::to_utc(t) );
}


template <class Duration>
inline
date::gps_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_gps(const hops_time<Duration>& t) NOEXCEPT
{
    return date::gps_clock::from_utc( to_utc(t) );
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_gps(const date::gps_time<Duration>& t) NOEXCEPT
{
    return from_utc( date::gps_clock::to_utc(t) );
}


template <class Duration>
inline
date::sys_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_sys(const hops_time<Duration>& t) NOEXCEPT
{
    return date::utc_clock::to_sys( to_utc(t) );
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_sys(const date::sys_time<Duration>& t) NOEXCEPT
{
    return from_utc( date::utc_clock::from_sys(t) );
}


inline
hops_clock::time_point
hops_clock::now()
{
    return from_utc(date::utc_clock::now());
}

template <class Duration>
inline
date::local_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_local(const hops_time<Duration>& t) NOEXCEPT
{
    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time<CD> hops_epoch_start = std::chrono::time_point_cast<CD>( get_hops_epoch_utc() );
    date::utc_time<CD> ut_time{t.time_since_epoch() + std::chrono::time_point_cast<Duration>(hops_epoch_start).time_since_epoch()};
    return date::utc_clock::to_local(ut_time);
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_local(const date::local_time<Duration>& t) NOEXCEPT
{
    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time<CD> t2 = date::utc_clock::from_local(t);
    date::utc_time<CD> hops_epoch_start = std::chrono::time_point_cast<CD>( get_hops_epoch_utc() );
    return hops_time<CD>{t2.time_since_epoch() - std::chrono::time_point_cast<Duration>(hops_epoch_start).time_since_epoch()};
}

template <class CharT, class Traits, class Duration>
std::basic_ostream<CharT, Traits>&
to_stream(std::basic_ostream<CharT, Traits>& os, const CharT* fmt,
          const hops_time<Duration>& t)
{
    const std::string abbrev("HOPS");
    CONSTDATA std::chrono::seconds offset{0};
    return date::to_stream(os, fmt, hops_clock::to_local(t), &abbrev, &offset);
}

template <class Duration, class CharT, class Traits, class Alloc = std::allocator<CharT>>
std::basic_istream<CharT, Traits>&
from_stream(std::basic_istream<CharT, Traits>& is, const CharT* fmt,
            hops_time<Duration>& tp,
            std::basic_string<CharT, Traits, Alloc>* abbrev = nullptr,
            std::chrono::minutes* offset = nullptr)
{
    date::local_time<Duration> lp;
    from_stream(is, fmt, lp, abbrev, offset);
    if (!is.fail())
        tp = hops_clock::from_local(lp);
    return is;
}

template <class CharT, class Traits, class Duration>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const hops_time<Duration>& t)
{
    const CharT fmt[] = {'%', 'F', 'T', '%', 'T', 'Z', CharT{}};
    return to_stream(os, fmt, t);
}

inline
hops_clock::time_point
hops_clock::from_iso8601_format(const std::string& timestamp)
{
    using namespace date;
    using namespace std::chrono;
    std::string frmt = ISO8601_UTC_FORMAT;
    std::istringstream ss(timestamp);
    std::istream tmp_stream(ss.rdbuf());
    hops_time<std::chrono::nanoseconds> hops_tp;
    from_stream(tmp_stream, frmt.c_str(), hops_tp);
    return hops_tp;
}

inline
std::string
hops_clock::to_iso8601_format(const time_point& tp)
{
    std::stringstream ss;
    ss << tp;
    return ss.str();
}

inline
hops_clock::time_point
hops_clock::from_hops_format(const std::string& timestamp)
{
    using namespace date;
    using namespace std::chrono;

    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter(std::string(HOPS_TIME_DELIM));
    std::vector<std::string> tokens;
    tokenizer.SetString(&timestamp);
    tokenizer.GetTokens(&tokens);
    if(tokens.size() == 3 )
    {
        std::string hops_prefix = tokens[0];
        std::string unit = tokens[1];
        std::string nanosecond_count = tokens[2];
        if( hops_prefix == std::string(HOPS_TIMESTAMP_PREFIX) && unit == std::string(HOPS_TIME_UNIT) )
        {
            std::stringstream ss;
            ss << nanosecond_count;
            int64_t ns;
            ss >> ns;
            return time_point( std::chrono::nanoseconds(ns) );
        }
    }
    msg_error("utility", "hops timestamp string not understood or supported, returning epoch start. "<< eom);
    return time_point( std::chrono::nanoseconds(0) );

}

inline
std::string
hops_clock::to_hops_format(const time_point& tp)
{
    std::stringstream ss;
    ss << HOPS_TIMESTAMP_PREFIX;
    ss << HOPS_TIME_DELIM;
    ss << HOPS_TIME_UNIT;
    ss << HOPS_TIME_DELIM;
    ss << tp.time_since_epoch().count();
    return ss.str();
}

inline
hops_clock::time_point
hops_clock::from_legacy_hops_date(legacy_hops_date& ldate)
{
    vex_date vdate = vex_date_from_legacy(ldate);
    std::string vex_as_iso8601 = vex_date_to_iso8601_string(vdate);
    return hops_clock::from_iso8601_format(vex_as_iso8601);
}

inline
legacy_hops_date
hops_clock::to_legacy_hops_date(const time_point& tp)
{
    using namespace date;
    using namespace std::chrono;

    //convert the time point to sys time, and extract the date
    auto sys_tp = hops_clock::to_sys(tp);
    auto dp = date::sys_days( floor<date::days>( sys_tp ) );

    //get all of the date information
    date::year_month_day ymd{dp};
    auto year = ymd.year();
    auto month = ymd.month();
    auto day = ymd.day();

    //get the ordinal day of the year
    auto ordinal_day = day_of_year(dp);

    //get the time
    date::hh_mm_ss< std::chrono::nanoseconds> time{floor< std::chrono::nanoseconds>( sys_tp-dp) };
    auto hours = time.hours();
    auto mins = time.minutes();
    auto secs = time.seconds();
    auto nanos = time.subseconds();

    legacy_hops_date ldate;
    ldate.year = (int) year;
    ldate.day = ordinal_day.count();
    ldate.hour = hours.count();
    ldate.minute = mins.count();
    //note there may be loss of precision when converting from seconds/nanoseconds to legacy float
    ldate.second = (float) secs.count() + ( (float) (nanos.count() ) )*NANOSEC_TO_SEC;

    return ldate;
}




inline
hops_clock::time_point
hops_clock::from_mjd(const time_point& mjd_epoch, const double& epoch_offset, const double& mjd)
{
    double delta = (mjd - epoch_offset);
    delta *= JD_TO_SEC;
    std::chrono::duration<double> duration_seconds(delta);

    auto mjd_epoch_utc = to_utc(mjd_epoch);
    auto utc_time_point = mjd_epoch_utc + std::chrono::duration_cast< std::chrono::nanoseconds >(duration_seconds);
    auto hops_time_point = from_utc(utc_time_point);
    return hops_time_point;
}


inline
double
hops_clock::to_mjd(const time_point& mjd_epoch, const double& epoch_offset, const time_point& tp)
{
    auto mjd_epoch_utc = to_utc(mjd_epoch);
    auto tp_utc = to_utc(tp);

    double delta = (tp_utc - mjd_epoch_utc).count();
    delta *= NANOSEC_TO_SEC; //convert to seconds
    delta /= JD_TO_SEC; //convert to days
    delta += epoch_offset; //subtract epoch offset
    return delta;
}

inline
hops_clock::time_point
hops_clock::from_vex_format(const std::string& timestamp)
{
    vex_date vdate = hops_clock::extract_vex_date(timestamp);
    //convert the vex date info to an ISO-8601-style year-month-day type format
    std::string vex_as_iso8601 = vex_date_to_iso8601_string(vdate);
    return hops_clock::from_iso8601_format(vex_as_iso8601);
}


inline
std::string
hops_clock::to_vex_format(const time_point& tp, bool truncate_to_nearest_second)
{
    using namespace date;
    using namespace std::chrono;

    //convert the time point to sys time, and extract the date
    auto sys_tp = hops_clock::to_sys(tp);
    auto dp = sys_days( floor<date::days>( sys_tp ) );

    //get all of the date information
    year_month_day ymd{dp};
    auto year = ymd.year();
    auto month = ymd.month();
    auto day = ymd.day();

    //get the ordinal day of the year
    auto ordinal_day = day_of_year(dp);

    //get the time
    hh_mm_ss< std::chrono::nanoseconds> time{floor< std::chrono::nanoseconds>( sys_tp-dp) };
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



inline
hops_clock::vex_date
hops_clock::extract_vex_date(const std::string& timestamp)
{
    vex_date vdate;
    if(timestamp.size() == 0 )
    {
        msg_error("utilities", "cannot extract vex date from empty string." << eom);
        return vdate;
    }

    MHO_Tokenizer tokenizer;
    std::vector<std::string> tokens;
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

inline
std::string
hops_clock::vex_date_to_iso8601_string(hops_clock::vex_date vdate)
{
    using namespace date;
    using namespace std::chrono;

    std::stringstream ss;
    ss << vdate.year;
    ss << "-";

    date::year y(vdate.year);
    date::days ord_day(vdate.day_of_year);
    date::sys_days ymd = get_year_month_day(y,ord_day);

    //convert day-of-year to month-day
    auto month = date::year_month_day{ymd}.month();
    auto mday = date::year_month_day{ymd}.day();
    ss << std::setfill('0') << std::setw(2) << (unsigned) month;
    ss << "-";
    ss << std::setfill('0') << std::setw(2) << (unsigned) mday;

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
    int integer_nanosec = frac*SEC_TO_NANOSEC;
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


inline
hops_clock::vex_date
hops_clock::vex_date_from_legacy(const legacy_hops_date& legacy_date)
{
    hops_clock::vex_date vdate;
    vdate.year = legacy_date.year;
    vdate.day_of_year = legacy_date.day;
    vdate.hours = legacy_date.hour;
    vdate.minutes = legacy_date.minute;
    vdate.seconds = legacy_date.second;
    return vdate;
}


}//end of namespace


#endif /*! end of include guard: MHO_Clock */
