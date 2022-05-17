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

// hops_clock

/*


class hops_clock
{
    public:

        using duration                  = std::chrono::nanoseconds;
        using rep                       = duration::rep;
        using period                    = duration::period;
        using time_point                = std::chrono::time_point<hops_clock>;
        static const bool is_steady     = false;

        hops_clock()
        {
            std::string frmt = "%Y %m %d %T";
            std::string j2000 = "2000 01 01 11:59:27.816";
            date::tai_time<std::chrono::seconds> j2000_tai_epoch;
            std::istringstream ss(j2000);
            std::istream stream(ss.rdbuf());
            date::from_stream(stream, frmt.c_str(), j2000_tai_epoch);
            fHOPSEpochStart = std::chrono::time_point_cast<std::chrono::nanoseconds>( date::tai_clock::to_utc( j2000_tai_tp ) );
        }

        virtual ~hops_clock(){};


        static time_point now();

        template<typename Duration>
        static
        std::chrono::time_point<utc_clock, typename std::common_type<Duration, std::chrono::seconds>::type>
        to_utc(const std::chrono::time_point<hops_clock, Duration>&) NOEXCEPT;

        template<typename Duration>
        static
        std::chrono::time_point<hops_clock, typename std::common_type<Duration, std::chrono::seconds>::type>
        from_utc(const std::chrono::time_point<utc_clock, Duration>&) NOEXCEPT;

    private:

        date::utc_clock::time_point< duration> 

};



template <class Duration>
using hops_time = std::chrono::time_point<hops_clock, Duration>;

using hops_seconds = hops_time<std::chrono::seconds>;

template <class Duration>
inline
utc_time<typename std::common_type<Duration, std::chrono::seconds>::type>
hops_clock::to_utc(const hops_time<Duration>& t) NOEXCEPT
{
    using std::chrono::seconds;
    using CD = typename std::common_type<Duration, seconds>::type;




    return utc_time<CD>{t.time_since_epoch()} +
            (sys_days(year{1980}/January/Sunday[1]) - sys_days(year{1970}/January/1) +
             seconds{9});
}

template <class Duration>
inline
hops_time<typename std::common_type<Duration, std::chrono::seconds>::type>
hops_clock::from_utc(const utc_time<Duration>& t) NOEXCEPT
{
    using std::chrono::seconds;
    using CD = typename std::common_type<Duration, seconds>::type;
    return hops_time<CD>{t.time_since_epoch()} -
            (sys_days(year{1980}/January/Sunday[1]) - sys_days(year{1970}/January/1) +
             seconds{9});
}




inline
hops_clock::time_point
hops_clock::now()
{
    return from_utc(utc_clock::now());
}


template <class CharT, class Traits, class Duration>
std::basic_ostream<CharT, Traits>&
to_stream(std::basic_ostream<CharT, Traits>& os, const CharT* fmt,
          const hops_time<Duration>& t)
{
    const std::string abbrev("GPS");
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



*/























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
    // using tod = date::time_of_day<nanoseconds>;
    // constexpr tod t1 = tod{hours{13} + minutes{7} + seconds{5} + nanoseconds{22}};
    // 

    auto now = system_clock::now();
    auto test_now = test_clock::now();
    auto tai_now = date::tai_clock::now();
    auto utc_now = date::utc_clock::now();
    auto gps_now = date::gps_clock::now();
    auto hops_now = MHO_UTCJ2000Clock::now();

    // std::cout<<"t1 = "<<t1<<std::endl;
    // std::cout<<"now = "<<now<<std::endl;
    std::cout<<"utc now = "<<utc_now<<std::endl;
    std::cout<<"tai now = "<<tai_now<<std::endl;
    std::cout<<"gps now = "<<gps_now<<std::endl;

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
    std::string hops_epoch_start = {"19700101 00:00:00"};
    std::chrono::system_clock::time_point hops_epoch_start_tp;
    std::istringstream ss{ hops_epoch_start };
    std::string frmt = "%Y%m%d %T";
    ss >> date::parse(frmt, hops_epoch_start_tp);

    // std::istringstream utcss{ hops_epoch_start };
    // date::utc_clock::time_point utc_start_tp;
    // utcss >> date::parse(frmt, utc_start_tp);

    auto utc_hops_epoch_start = date::utc_clock::from_sys(hops_epoch_start_tp);
    auto gps_hops_epoch_start = date::gps_clock::from_utc(utc_hops_epoch_start);

    std::cout<<"hops epoch nanoseconds from system time start = "<< hops_epoch_start_tp.time_since_epoch().count()<<std::endl;
    std::cout<<"hops epoch nanoseconds from system utc epoch start = "<<utc_hops_epoch_start.time_since_epoch().count()<<std::endl;
    std::cout<<"hops epoch nanoseconds from system gps epoch start = "<<gps_hops_epoch_start.time_since_epoch().count()<<std::endl;

    std::string j2000 = "20000101 12:00:00";
    date::tai_time<std::chrono::seconds> j2000_tai_tp;
    std::istringstream ss2(j2000);
    std::istream stream(ss2.rdbuf());
    date::from_stream(stream, frmt.c_str(), j2000_tai_tp);
    auto j2000_utc_tp =  std::chrono::time_point_cast<std::chrono::nanoseconds>( date::tai_clock::to_utc( j2000_tai_tp ) );
    std::cout<<"J2000 utc, ns since epoch = "<< j2000_utc_tp.time_since_epoch().count() <<std::endl;
    std::cout<<"J2000 tai, ns since epoch = "<< j2000_tai_tp.time_since_epoch().count() <<std::endl;


    return 0;
}
