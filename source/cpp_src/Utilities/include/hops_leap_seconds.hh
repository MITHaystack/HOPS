#ifndef HOPS_LEAP_SECONDS_HH__
#define HOPS_LEAP_SECONDS_HH__

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>

//header-only portion of the H. Hinnant date library (civil calendar only, no
//linking required). We deliberately do NOT include "date/tz.h" here: that is the
//compiled part that loads the IANA timezone/leap-second database from the host
//at runtime and forces a link against libdate-tz.
#include "date/date.h"

/*!
 *@file hops_leap_seconds.hh
 *@brief Self-contained (link-free) reimplementation of the small slice of the
 * date library's tz facilities that HOPS actually uses: the utc/tai/gps clocks
 * and leap-second lookup. HOPS uses ZERO timezone functionality, so the only
 * runtime data the compiled libdate-tz supplied was the leap-second table.
 *
 * That table is short, well known, and last changed 2017-01-01 (CGPM Res. 4,
 * 2022 commits to abolishing leap seconds by ~2035, so none are expected). We
 * hardcode it here. This is so we can have:
 *   1) no link against libdate-tz,
 *   2) no runtime dependency on the host's /usr/share/zoneinfo (deterministic and
 *     reproducible across the manylinux / Ubuntu-LTS distribution targets),
 * However, this is important! :
 *     the hard-coded leap second table is now our maintenance responsibility.
 *     If IERS Bulletin C ever announces a new leap second, append one entry to the events[] list in
 *     leap_table() below: the day AFTER the event and a signed delta (+1 for the
 *     usual inserted 23:59:60 second, -1 for a negative/removed leap second where
 *     23:59:59 is skipped). TestLeapSecondDrift cross-checks the table against the
 *     installed system tz db at build time....which should catch a problem in most cases
 *
 * All 27 leap seconds so far are +1; We have not had any negative leap seconds yet, but
 * the signed representation below should in theory be able to handle either sign.
 *
 * Conventions match date exactly (so results are interchangeable):
 *   - sys_time  = POSIX/std::chrono::system_clock, 86400 s/day, ignores leaps.
 *   - utc_time  = counts real SI seconds since 1970-01-01 WITH leap seconds; the
 *                 leap count returned by get_leap_second_info() ranges 0..27 (the
 *                 pre-1972 10 s base is folded into the tai_time constant below).
 *   - tai_time  = SI seconds since 1958-01-01; to_utc subtracts a fixed constant.
 *   - gps_time  = SI seconds since 1980-01-06; to_utc adds a fixed constant.
 */

namespace hops
{

/**
 * @brief Result of a leap-second lookup, mirroring date::leap_second_info.
 * elapsed = number of leap seconds inserted strictly before the query instant;
 * is_leap_second = true iff the instant falls inside an inserted (23:59:60) second.
 */
struct leap_second_info
{
        std::chrono::seconds elapsed;
        bool is_leap_second;
};

namespace leap_detail
{

//fixed continuous-scale offsets (see file header for the derivation/verification):
//  to_utc(tai) = utc_time(tai_count - kTaiEpochOffset)   [1958-01-01 gap + 10 s base]
//  to_utc(gps) = utc_time(gps_count + kGpsEpochOffset)   [1970->1980-01-06 + 9 leaps]
static const std::int64_t kTaiEpochOffset = 378691210;
static const std::int64_t kGpsEpochOffset = 315964809;

//One leap-second event: the sys-time (unix seconds) of 00:00:00 UTC on the day
//AFTER the event, and the SIGNED cumulative offset (utc_count - sys_count) that
//applies at/after it. Storing the cumulative value rather than deriving it from
//the row index lets the table represent negative (removed) leap seconds too: a
//+1 event steps the offset up (inserted 23:59:60), a -1 event steps it down
//(23:59:59 skipped). Entries are sorted ascending by transition_sys.
struct leap_entry
{
        std::int64_t transition_sys;
        std::int64_t cumulative;
};

inline const std::vector< leap_entry >& leap_table()
{
    static const std::vector< leap_entry > table = [] {
        //(year, month, day of the day-after, signed delta) for each leap event.
        //All leaps to date are +1; any future negative leap would use -1.
        static const struct
        {
                int y, m, d, delta;
        } events[] = {
            {1972, 7, 1, +1},
            {1973, 1, 1, +1},
            {1974, 1, 1, +1},
            {1975, 1, 1, +1},
            {1976, 1, 1, +1},
            {1977, 1, 1, +1},
            {1978, 1, 1, +1},
            {1979, 1, 1, +1},
            {1980, 1, 1, +1},
            {1981, 7, 1, +1},
            {1982, 7, 1, +1},
            {1983, 7, 1, +1},
            {1985, 7, 1, +1},
            {1988, 1, 1, +1},
            {1990, 1, 1, +1},
            {1991, 1, 1, +1},
            {1992, 7, 1, +1},
            {1993, 7, 1, +1},
            {1994, 7, 1, +1},
            {1996, 1, 1, +1},
            {1997, 7, 1, +1},
            {1999, 1, 1, +1},
            {2006, 1, 1, +1},
            {2009, 1, 1, +1},
            {2012, 7, 1, +1},
            {2015, 7, 1, +1},
            {2017, 1, 1, +1}
        };
        std::vector< leap_entry > t;
        t.reserve(sizeof(events) / sizeof(events[0]));
        std::int64_t cumulative = 0;
        for(std::size_t i = 0; i < sizeof(events) / sizeof(events[0]); ++i)
        {
            date::year_month_day d{date::year{events[i].y}, date::month{static_cast< unsigned >(events[i].m)},
                                   date::day{static_cast< unsigned >(events[i].d)}};
            std::int64_t transition =
                std::chrono::duration_cast< std::chrono::seconds >(date::sys_days{d}.time_since_epoch()).count();
            cumulative += events[i].delta;
            t.push_back(leap_entry{transition, cumulative});
        }
        return t;
    }();
    return table;
}

//signed cumulative offset (utc_count - sys_count) in effect at a given unix-second
//value on the SYS timescale.
inline std::int64_t offset_at_sys(std::int64_t sys_seconds)
{
    const std::vector< leap_entry >& T = leap_table();
    std::int64_t offset = 0;
    for(std::size_t k = 0; k < T.size(); ++k)
    {
        if(sys_seconds >= T[k].transition_sys)
        {
            offset = T[k].cumulative;
        }
        else
        {
            break;
        }
    }
    return offset;
}

} // namespace leap_detail

//clock tag types. We only ever instantiate time_points with an explicit Duration,
//so the nested typedefs exist purely to model a Clock (and to give utc_clock a now()).
struct utc_clock;
struct tai_clock;
struct gps_clock;
template< class Duration > using utc_time = std::chrono::time_point< utc_clock, Duration >;
template< class Duration > using tai_time = std::chrono::time_point< tai_clock, Duration >;
template< class Duration > using gps_time = std::chrono::time_point< gps_clock, Duration >;

/**
 * @brief Leap-second lookup for a utc_time, matching date::get_leap_second_info.
 */
template< class Duration > inline leap_second_info get_leap_second_info(const utc_time< Duration >& ut)
{
    using namespace leap_detail;
    const std::vector< leap_entry >& T = leap_table();
    //floor to whole seconds (all HOPS instants are positive, so truncation == floor)
    std::int64_t u = std::chrono::duration_cast< std::chrono::seconds >(ut.time_since_epoch()).count();

    //In the utc timescale the offset steps from C[k-1] to C[k] at the transition,
    //and the step lands at utc threshold  U_k = transition_sys + min(C[k-1], C[k]):
    //  +1 (inserted) leap: U_k = transition + C[k-1], and utc == U_k is the
    //     inserted 23:59:60 second;
    //  -1 (removed) leap:  U_k = transition + C[k], with no inserted second (the
    //     skip is on the sys side, and surfaces via to_sys).
    //U_k is strictly increasing, so scan for the last transition at/below u.
    std::int64_t elapsed = 0;
    long selected = -1;
    for(std::size_t k = 0; k < T.size(); ++k)
    {
        std::int64_t prev = (k == 0) ? 0 : T[k - 1].cumulative;
        std::int64_t threshold = T[k].transition_sys + std::min(prev, T[k].cumulative);
        if(u >= threshold)
        {
            elapsed = T[k].cumulative;
            selected = static_cast< long >(k);
        }
        else
        {
            break;
        }
    }

    bool is_ls = false;
    if(selected >= 0)
    {
        std::size_t k = static_cast< std::size_t >(selected);
        std::int64_t prev = (k == 0) ? 0 : T[k - 1].cumulative;
        std::int64_t delta = T[k].cumulative - prev;
        std::int64_t threshold = T[k].transition_sys + std::min(prev, T[k].cumulative);
        //only a positive (inserted) leap has a real 23:59:60 second, one wide
        is_ls = (delta > 0) && (u == threshold);
    }
    return leap_second_info{std::chrono::seconds{elapsed}, is_ls};
}

/**
 * @brief UTC clock: real SI seconds since 1970-01-01 including leap seconds.
 */
struct utc_clock
{
        using duration = std::chrono::system_clock::duration;
        using rep = duration::rep;
        using period = duration::period;
        using time_point = std::chrono::time_point< utc_clock >;
        static CONSTDATA bool is_steady = false;

        template< class Duration >
        static utc_time< typename std::common_type< Duration, std::chrono::seconds >::type >
        from_sys(const date::sys_time< Duration >& st)
        {
            using CD = typename std::common_type< Duration, std::chrono::seconds >::type;
            std::int64_t s = std::chrono::duration_cast< std::chrono::seconds >(st.time_since_epoch()).count();
            std::chrono::seconds leaps{leap_detail::offset_at_sys(s)};
            return utc_time< CD >{st.time_since_epoch() + leaps};
        }

        template< class Duration >
        static date::sys_time< typename std::common_type< Duration, std::chrono::seconds >::type >
        to_sys(const utc_time< Duration >& ut)
        {
            using CD = typename std::common_type< Duration, std::chrono::seconds >::type;
            leap_second_info info = get_leap_second_info(ut);
            return date::sys_time< CD >{ut.time_since_epoch() - info.elapsed};
        }

        template< class Duration >
        static date::local_time< typename std::common_type< Duration, std::chrono::seconds >::type >
        to_local(const utc_time< Duration >& ut)
        {
            using CD = typename std::common_type< Duration, std::chrono::seconds >::type;
            return date::local_time< CD >{to_sys(ut).time_since_epoch()};
        }

        template< class Duration >
        static utc_time< typename std::common_type< Duration, std::chrono::seconds >::type >
        from_local(const date::local_time< Duration >& lt)
        {
            using CD = typename std::common_type< Duration, std::chrono::seconds >::type;
            return from_sys(date::sys_time< Duration >{lt.time_since_epoch()});
        }

        static utc_time< std::chrono::nanoseconds > now()
        {
            return from_sys(std::chrono::time_point_cast< std::chrono::nanoseconds >(std::chrono::system_clock::now()));
        }
};

/**
 * @brief TAI clock: SI seconds since 1958-01-01; constant offset from UTC.
 */
struct tai_clock
{
        using duration = std::chrono::system_clock::duration;
        using rep = duration::rep;
        using period = duration::period;
        using time_point = std::chrono::time_point< tai_clock >;
        static CONSTDATA bool is_steady = false;

        template< class Duration > static utc_time< Duration > to_utc(const tai_time< Duration >& t)
        {
            return utc_time< Duration >{t.time_since_epoch() - std::chrono::seconds{leap_detail::kTaiEpochOffset}};
        }

        template< class Duration > static tai_time< Duration > from_utc(const utc_time< Duration >& t)
        {
            return tai_time< Duration >{t.time_since_epoch() + std::chrono::seconds{leap_detail::kTaiEpochOffset}};
        }
};

/**
 * @brief GPS clock: SI seconds since 1980-01-06; constant offset from UTC.
 */
struct gps_clock
{
        using duration = std::chrono::system_clock::duration;
        using rep = duration::rep;
        using period = duration::period;
        using time_point = std::chrono::time_point< gps_clock >;
        static CONSTDATA bool is_steady = false;

        template< class Duration > static utc_time< Duration > to_utc(const gps_time< Duration >& t)
        {
            return utc_time< Duration >{t.time_since_epoch() + std::chrono::seconds{leap_detail::kGpsEpochOffset}};
        }

        template< class Duration > static gps_time< Duration > from_utc(const utc_time< Duration >& t)
        {
            return gps_time< Duration >{t.time_since_epoch() - std::chrono::seconds{leap_detail::kGpsEpochOffset}};
        }
};

} // namespace hops

#endif /*! end of include guard: HOPS_LEAP_SECONDS_HH__ */
