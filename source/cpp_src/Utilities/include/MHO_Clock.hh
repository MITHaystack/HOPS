#ifndef MHO_Clock_HH__
#define MHO_Clock_HH__

/*
*@file: MHO_Clock.hh
*@class: hops_clock
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date: a clock for hops-time stamps, measures time in (UTC) nanoseconds since J2000 epoch.
*Functionality modelled on the gps_clock from the 'date' library (requires the timezone tz library too).
*@brief:
*/

#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>

//these can someday be replaced with STL versions in C++20
#include "date/date.h"
#include "date/tz.h"

#include "MHO_Tokenizer.hh"

#define J2000_TAI_EPOCH "2000-01-01T11:59:27.816Z"
#define ISO8601_UTC_FORMAT "%FT%TZ"
#define HOPS_TIMESTAMP_PREFIX "HOPS-J2000"
#define HOPS_TIME_DELIM "|"
#define HOPS_REF_FRAME "UTC"
#define HOPS_TIME_UNIT "ns"

namespace hops 
{


class hops_clock
{
    public:

        using duration                  = std::chrono::nanoseconds;
        using rep                       = duration::rep;
        using period                    = duration::period;
        using time_point                = std::chrono::time_point<hops_clock>;
        static const bool is_steady     = false;

        static time_point now();

        template<typename Duration>
        static
        std::chrono::time_point<utc_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_utc(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_utc(const std::chrono::time_point<utc_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<tai_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_tai(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_tai(const std::chrono::time_point<tai_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<gps_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_gps(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_gps(const std::chrono::time_point<gps_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<system_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_sys(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_sys(const std::chrono::time_point<system_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<local_t, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_local(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_local(const std::chrono::time_point<local_t, Duration>&) NOEXCEPT;
        
        static
        std::chrono::time_point<hops_clock, std::chrono::nanoseconds >
        from_iso8601_format(const string& timestamp);

        static
        std::string
        to_iso8601_format(const std::chrono::time_point<hops_clock, std::chrono::nanoseconds >& tp);

        static
        std::chrono::time_point<hops_clock, std::chrono::nanoseconds >
        from_hops_raw_format(const string& timestamp);

        static
        std::string
        to_hops_raw_format(const std::chrono::time_point<hops_clock, std::chrono::nanoseconds >& tp);

        static
        std::chrono::time_point<hops_clock, std::chrono::nanoseconds >
        from_vex_format(const string& timestamp);

        static
        std::string
        to_vex_format(const std::chrono::time_point<hops_clock, std::chrono::nanoseconds >& tp, bool truncate_to_nearest_second = false);

        static date::utc_time< std::chrono::nanoseconds > get_hops_epoch()
        {
            std::string frmt = ISO8601_UTC_FORMAT;
            std::string j2000 = J2000_TAI_EPOCH;
            date::tai_time<std::chrono::nanoseconds> j2000_tai_epoch;
            std::istringstream ss(j2000);
            std::istream stream(ss.rdbuf());
            date::from_stream(stream, frmt.c_str(), j2000_tai_epoch);
            return std::chrono::time_point_cast<std::chrono::nanoseconds>( date::tai_clock::to_utc( j2000_tai_epoch ) );
        }

    private:

        static
        date::days
        day_of_year(date::sys_days sd)
        {
            auto y = year_month_day{sd}.year();
            return sd - sys_days{y/jan/0};
        }



};



template <class Duration>
using hops_time = std::chrono::time_point<hops_clock, Duration>;

template <class Duration>
inline
utc_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_utc(const hops_time<Duration>& t) NOEXCEPT
{

    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time< std::chrono::nanoseconds > hops_epoch_start = get_hops_epoch();
    return utc_time<CD>(t.time_since_epoch() + hops_epoch_start.time_since_epoch());
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_utc(const utc_time<Duration>& t) NOEXCEPT
{
    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time< std::chrono::nanoseconds > hops_epoch_start = get_hops_epoch();
    return hops_time<CD>(t.time_since_epoch() - hops_epoch_start.time_since_epoch());
}

template <class Duration>
inline
tai_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_tai(const hops_time<Duration>& t) NOEXCEPT
{
    return tai_clock::from_utc( to_utc(t) );
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_tai(const tai_time<Duration>& t) NOEXCEPT
{
    return from_utc( tai_clock::to_utc(t) );
}


template <class Duration>
inline
gps_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_gps(const hops_time<Duration>& t) NOEXCEPT
{
    return gps_clock::from_utc( to_utc(t) );
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_gps(const gps_time<Duration>& t) NOEXCEPT
{
    return from_utc( gps_clock::to_utc(t) );
}


template <class Duration>
inline
sys_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_sys(const hops_time<Duration>& t) NOEXCEPT
{
    return utc_clock::to_sys( to_utc(t) );
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_sys(const sys_time<Duration>& t) NOEXCEPT
{
    return from_utc( utc_clock::from_sys(t) );
}


inline
hops_clock::time_point
hops_clock::now()
{
    return from_utc(utc_clock::now());
}

template <class Duration>
inline
local_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_local(const hops_time<Duration>& t) NOEXCEPT
{
    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time<CD> hops_epoch_start = std::chrono::time_point_cast<CD>( get_hops_epoch() );
    date::utc_time<CD> ut_time{t.time_since_epoch() + std::chrono::time_point_cast<Duration>(hops_epoch_start).time_since_epoch()};
    return utc_clock::to_local(ut_time);
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_local(const local_time<Duration>& t) NOEXCEPT
{
    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time<CD> t2 = utc_clock::from_local(t);
    date::utc_time<CD> hops_epoch_start = std::chrono::time_point_cast<CD>( get_hops_epoch() );
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
    local_time<Duration> lp;
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
std::chrono::time_point<hops_clock, std::chrono::nanoseconds >
hops_clock::from_iso8601_format(const string& timestamp)
{
    std::string frmt = ISO8601_UTC_FORMAT;
    std::istringstream ss(timestamp);
    std::istream tmp_stream(ss.rdbuf());
    hops_time<std::chrono::nanoseconds> hops_tp;
    from_stream(tmp_stream, frmt.c_str(), hops_tp);
    return hops_tp;
}

inline
std::string
hops_clock::to_iso8601_format(const std::chrono::time_point<hops_clock, std::chrono::nanoseconds >& tp)
{
    std::stringstream ss;
    ss << tp;
    return ss.str();
}

inline
std::chrono::time_point<hops_clock, std::chrono::nanoseconds >
hops_clock::from_vex_format(const string& timestamp)
{
    //TODO FIXME !! 
    std::cout<<"NO IMPL!"<<std::endl;
    hops_time<std::chrono::nanoseconds> hops_tp;
    return hops_tp;
}


inline
std::chrono::time_point<hops_clock, std::chrono::nanoseconds >
hops_clock::from_hops_raw_format(const string& timestamp)
{
    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter(std::string(HOPS_TIME_DELIM));
    std::vector<std::string> tokens;
    tokenizer.SetString(&timestamp);
    tokenizer.GetTokens(&tokens);

    //std::cout<<"ntokens = "<<tokens.size()<<std::endl;

    //TODO FIXME -- CHECK hops_prefix, ref_frame, unit, etc for validity!
    std::string hops_prefix = tokens[0];
    std::string ref_frame = tokens[1]; //eventually we may also want to support TAI in addition to UTC
    std::string unit = tokens[2];
    std::string nanosecond_count = tokens[3];

    //std::cout<<"# nanoseconds = "<<nanosecond_count<<std::endl;

    std::stringstream ss;
    ss << nanosecond_count;
    int64_t ns;
    ss >> ns;
    return std::chrono::time_point<hops_clock, std::chrono::nanoseconds >( std::chrono::nanoseconds(ns) );
    //return hops_tp;
}

inline
std::string
hops_clock::to_hops_raw_format(const std::chrono::time_point<hops_clock, std::chrono::nanoseconds >& tp)
{
    std::stringstream ss;
    ss << HOPS_TIMESTAMP_PREFIX;
    ss << HOPS_TIME_DELIM;
    ss << HOPS_REF_FRAME;
    ss << HOPS_TIME_DELIM;
    ss << HOPS_TIME_UNIT;
    ss << HOPS_TIME_DELIM;
    ss << tp.time_since_epoch().count();
    return ss.str();
}

inline
std::string
hops_clock::to_vex_format(const std::chrono::time_point<hops_clock, std::chrono::nanoseconds >& tp, bool truncate_to_nearest_second)
{

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
        //removing trailing zeros
        snano_sec.erase(snano_sec.find_last_not_of('0') + 1, std::string::npos);
        ss << ".";
        ss << snano_sec;
    }
    ss << "s";

    return ss.str();
}


}//end of namespace


#endif /* end of include guard: MHO_Clock */