#ifndef MHO_ClockEpoch_HH__
#define MHO_ClockEpoch_HH__

/*
*@file: MHO_ClockEpoch.hh
*@class: MHO_ClockEpoch
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <chrono>

//these can someday be replaced with STL versions in C++20
#include "date/date.h"
#include "date/tz.h"

// using MHO_UTCClock = date::utc_clock;
// using MHO_TAIClock = date::tai_clock;
// using MHO_GPSClock = date::gps_clock; 

// struct MHO_UTCJ2000Clock
// {
//     typedef std::chrono::nanoseconds duration;
//     typedef duration::rep rep;
//     typedef duration::period period;
//     typedef std::chrono::time_point<MHO_UTCJ2000Clock> time_point;
// 
//     static const bool is_steady = date::utc_clock::is_steady; //false
// 
//     static time_point now() noexcept
//     {
//         using namespace date;
//         using namespace std::chrono;
//         date::utc_clock::time_point hops_epoch_start = date::utc_clock::from_sys( sys_days{January/1/2000} );
//         duration time_since_utc_epoch = duration_cast<duration>( date::utc_clock::now().time_since_epoch() );
//         duration time_between_utc_and_hops_epoch = duration_cast<duration>(hops_epoch_start.time_since_epoch() );
//         return time_point(time_since_utc_epoch - time_between_utc_and_hops_epoch);
//     }
// 
// };


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
        std::chrono::time_point<local_t, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        to_local(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::nanoseconds>::type>
        from_local(const std::chrono::time_point<local_t, Duration>&) NOEXCEPT;
        

    private:

        static date::utc_time< std::chrono::nanoseconds > get_hops_epoch()
        {
            std::string frmt = "%F %T";
            std::string j2000 = "2000-01-01 11:59:27.816";
            date::tai_time<std::chrono::nanoseconds> j2000_tai_epoch;
            std::istringstream ss(j2000);
            std::istream stream(ss.rdbuf());
            date::from_stream(stream, frmt.c_str(), j2000_tai_epoch);
            return std::chrono::time_point_cast<std::chrono::nanoseconds>( date::tai_clock::to_utc( j2000_tai_epoch ) );
        }

};



template <class Duration>
using hops_time = std::chrono::time_point<hops_clock, Duration>;

using hops_seconds = hops_time<std::chrono::seconds>;

template <class Duration>
inline
utc_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::to_utc(const hops_time<Duration>& t) NOEXCEPT
{

    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time< std::chrono::nanoseconds > fHOPSEpochStart = get_hops_epoch();
    return utc_time<CD>(t.time_since_epoch() + fHOPSEpochStart.time_since_epoch());
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_utc(const utc_time<Duration>& t) NOEXCEPT
{
    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time< std::chrono::nanoseconds > fHOPSEpochStart = get_hops_epoch();
    return hops_time<CD>(t.time_since_epoch() - fHOPSEpochStart.time_since_epoch());
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
    date::utc_time<CD> fHOPSEpochStart = std::chrono::time_point_cast<CD>( get_hops_epoch() );
    date::utc_time<CD> ut_time{t.time_since_epoch() + std::chrono::time_point_cast<Duration>(fHOPSEpochStart).time_since_epoch()};
    return utc_clock::to_local(ut_time);
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::nanoseconds>::type>
hops_clock::from_local(const local_time<Duration>& t) NOEXCEPT
{
    using CD = typename std::common_type<Duration, std::chrono::nanoseconds>::type;
    date::utc_time<CD> t2 = utc_clock::from_local(t);
    date::utc_time<CD> fHOPSEpochStart = std::chrono::time_point_cast<CD>( get_hops_epoch() );
    return hops_time<CD>{t2.time_since_epoch() - std::chrono::time_point_cast<Duration>(fHOPSEpochStart).time_since_epoch()};
}

template <class CharT, class Traits, class Duration>
std::basic_ostream<CharT, Traits>&
to_stream(std::basic_ostream<CharT, Traits>& os, const CharT* fmt,
          const hops_time<Duration>& t)
{
    const std::string abbrev("HOPS");
    CONSTDATA std::chrono::seconds offset{0};
    return to_stream(os, fmt, hops_clock::to_local(t), &abbrev, &offset);
}

template <class CharT, class Traits, class Duration>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const hops_time<Duration>& t)
{
    const CharT fmt[] = {'%', 'F', ' ', '%', 'T', CharT{}};
    return to_stream(os, fmt, t);
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


}//end of namespace


#endif /* end of include guard: MHO_ClockEpoch */