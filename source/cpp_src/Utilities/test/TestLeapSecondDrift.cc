/*
 * TestLeapSecondDrift
 *
 * Cross-checks the self-contained internal leap-second table + clock casts
 * (hops_leap_seconds.hh) against the upstream compiled date/tz.h backend, which
 * reads the installed system timezone/leap-second database at runtime.
 *
 * This test ALWAYS links libdate-tz and exercises BOTH backends directly,
 * independent of HOPS_USE_INTERNAL_LEAP_SECONDS, so it detects any drift between
 * our hardcoded table and the host tz db (e.g. a newly announced leap second, or
 * a system whose zoneinfo was built without leap-second data).
 *
 * It compares, over a weekly sweep 1970..2035 plus per-second sampling around
 * every leap-second boundary:
 *   - get_leap_second_info(utc): elapsed count and is_leap_second flag
 *   - utc_clock::from_sys / to_sys
 *   - tai_clock::from_utc  (continuous TAI scale)
 *   - gps_clock::from_utc  (continuous GPS scale)
 *
 * Exit status is non-zero on any mismatch (or if the system tz db carries no
 * leap-second data, which would make the date-tz backend itself unreliable).
 */

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "date/date.h"
#include "date/tz.h"            // reference backend: system tz db
#include "hops_leap_seconds.hh" // implementation under test

namespace
{

using seconds = std::chrono::seconds;
using date::sys_time;

int g_failures = 0;

void check(bool cond, const std::string& what, std::int64_t s)
{
    if(!cond)
    {
        ++g_failures;
        if(g_failures <= 40) //cap the noise
        {
            std::cout << "DRIFT @ sys=" << s << " : " << what << std::endl;
        }
    }
}

//compare both backends at a given unix-second value.
void compare_at_sys(std::int64_t s)
{
    sys_time< seconds > st{seconds{s}};

    // sys -> utc
    auto d_utc = date::utc_clock::from_sys(st);
    auto h_utc = hops::utc_clock::from_sys(st);
    std::int64_t du = d_utc.time_since_epoch().count();
    std::int64_t hu = h_utc.time_since_epoch().count();
    check(du == hu, "utc_clock::from_sys count " + std::to_string(du) + " vs " + std::to_string(hu), s);

    // leap-second info evaluated at the utc instant (use the reference count so
    // both backends are queried at the identical utc value)
    date::utc_time< seconds > d_ut{seconds{du}};
    hops::utc_time< seconds > h_ut{seconds{du}};
    auto d_info = date::get_leap_second_info(d_ut);
    auto h_info = hops::get_leap_second_info(h_ut);
    check(d_info.elapsed.count() == h_info.elapsed.count(),
          "leap elapsed " + std::to_string(d_info.elapsed.count()) + " vs " + std::to_string(h_info.elapsed.count()), s);
    check(d_info.is_leap_second == h_info.is_leap_second, "is_leap_second mismatch", s);

    // utc -> sys round trip
    std::int64_t d_back = date::utc_clock::to_sys(d_ut).time_since_epoch().count();
    std::int64_t h_back = hops::utc_clock::to_sys(h_ut).time_since_epoch().count();
    check(d_back == h_back, "utc_clock::to_sys " + std::to_string(d_back) + " vs " + std::to_string(h_back), s);

    // utc -> tai (continuous)
    std::int64_t d_tai = date::tai_clock::from_utc(d_ut).time_since_epoch().count();
    std::int64_t h_tai = hops::tai_clock::from_utc(h_ut).time_since_epoch().count();
    check(d_tai == h_tai, "tai_clock::from_utc " + std::to_string(d_tai) + " vs " + std::to_string(h_tai), s);

    // utc -> gps (continuous)
    std::int64_t d_gps = date::gps_clock::from_utc(d_ut).time_since_epoch().count();
    std::int64_t h_gps = hops::gps_clock::from_utc(h_ut).time_since_epoch().count();
    check(d_gps == h_gps, "gps_clock::from_utc " + std::to_string(d_gps) + " vs " + std::to_string(h_gps), s);
}

} // namespace

int main(int /*argc*/, char** /*argv*/)
{
    // Guard: the reference backend needs leap-second data from the system tz db.
    try
    {
        std::size_t n_sys_leaps = date::get_tzdb().leap_seconds.size();
        std::cout << "system tz db leap_seconds count: " << n_sys_leaps << std::endl;
        if(n_sys_leaps == 0)
        {
            std::cout << "ERROR: system tz db reports zero leap seconds; date-tz reference is "
                         "unusable on this host (zoneinfo built without leap data)."
                      << std::endl;
            return 1;
        }
    }
    catch(const std::exception& e)
    {
        std::cout << "ERROR: could not load system tz db: " << e.what() << std::endl;
        return 1;
    }

    // 1) weekly sweep 1970-01-01 .. ~2035-01-01
    const std::int64_t week = 7 * 86400;
    const std::int64_t end = 2051222400; // ~2035-01-01
    for(std::int64_t s = 0; s < end; s += week)
    {
        compare_at_sys(s);
    }

    // 2) per-second sampling in a +/-5 s window around every leap-second boundary
    //    (both the sys transition instant and the utc leap-second itself land here)
    const std::vector< hops::leap_detail::leap_entry >& T = hops::leap_detail::leap_table();
    for(std::size_t k = 0; k < T.size(); ++k)
    {
        for(std::int64_t off = -5; off <= 5; ++off)
        {
            compare_at_sys(T[k].transition_sys + off);
        }
    }

    if(g_failures == 0)
    {
        std::cout << "PASS: internal leap-second table matches system tz db across all samples." << std::endl;
        return 0;
    }
    std::cout << "FAIL: " << g_failures << " drift mismatch(es) detected." << std::endl;
    return 1;
}
