#include "MHO_LegacyRootCodeGenerator.hh"

/* [a-z]x6 before this unix clock and [0-9A-Z]x6 after it */
#define HOPS_ROOT_BREAK (1519659904)

namespace hops 
{

std::string 
MHO_LegacyRootCodeGenerator::GetCode()
{
    fNow = time ((time_t *) NULL);
    struct tm* t = gmtime (&fNow);
    fYear = t->tm_year;
    fDay = t->tm_yday+1;
    fHour = t->tm_hour;
    fMin = t->tm_min;
    fSec = t->tm_sec;
    fNow -= fSec;
    return root_id_break (fNow, fYear, fDay, fHour, fMin, fSec);
}

std::vector<std::string> 
MHO_LegacyRootCodeGenerator::GetCodes(std::size_t N)
{
    std::vector< std::string > codes;
    fNow = time ((time_t *) NULL);

    for(std::size_t i=0; i<N; i++)
    {
        time_t next = fNow + i*root_id_delta(fNow);
        struct tm* t = gmtime (&next);
        fYear = t->tm_year;
        fDay = t->tm_yday+1;
        fHour = t->tm_hour;
        fMin = t->tm_min;
        fSec = t->tm_sec;
        next -= fSec;
        codes.push_back( root_id_break (next, fYear, fDay, fHour, fMin, fSec) );
    }
    return codes;
}

int 
MHO_LegacyRootCodeGenerator::root_id_delta(time_t now)
{
    int delta = 4;
    if (now >= HOPS_ROOT_BREAK) delta = 1;
    return delta;
}

std::string
MHO_LegacyRootCodeGenerator::root_id_later(time_t now)
{
    long m, u = now - HOPS_ROOT_BREAK;
    char datestr[8];
    char *date = datestr;
    m = u / (36*36*36*36*36);
    u -= m * (36*36*36*36*36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    m = u / (36*36*36*36);
    u -= m * (36*36*36*36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    m = u / (36*36*36);
    u -= m * (36*36*36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    m = u / (36*36);
    u -= m * (36*36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    m = u / (36);
    u -= m * (36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    *date++ = (u < 10) ? (char)('0' + u) : (char)('A' + u - 10);
    *date = 0;

    std::string ret_val = datestr;
    return ret_val;
}

/* original implementation follows */
std::string
MHO_LegacyRootCodeGenerator::root_id(int year, int day, int hour, int min, int sec)
{
    int year_79, elapsed, nleaps;
    char code[7];

                            /* Check inputs for validity */
    year_79 = (year % 100) - 79;                    /* Years since 1979 */
    if (year_79 < 0) year_79 += 100;                /* Take care of Y2K */
    nleaps = (year_79 + 2) / 4;                     /* Count leap days */
    if(day < 1 || day > 366) return(NULL);
    if(day == 366 && (year_79 + 3) % 4 != 0) return(NULL);  /* Leap year? */
    if(hour < 0 || hour > 23) return(NULL);
    if(min < 0 || min > 59) return(NULL);
    if(sec < 0) return(NULL);  // allow secs beyond 59 for sequential rcode gen

                            /* 4-sec periods elapsed since 00:00 Jan 1 1979 */
    elapsed = year_79*7884000 + (day+nleaps-1)*21600 + hour*900 + min*15 + (sec/4);

    code[6] = '\0';
    code[5] = 'a' + elapsed % 26;
    code[4] = 'a' + (elapsed/26) % 26;
    code[3] = 'a' + (elapsed/676) % 26;
    code[2] = 'a' + (elapsed/17576) % 26;
    code[1] = 'a' + (elapsed/456976) % 26;                  /* 0 --> 'aaaaaa' */
    code[0] = 'a' + elapsed/11881376;

    std::string ret_val = code;
    return ret_val;
}

std::string
MHO_LegacyRootCodeGenerator::root_id_break(time_t now, int year, int day, int hour, int min, int sec)
{
    if (now + sec >= HOPS_ROOT_BREAK)
    {
        return root_id_later(now + sec);
    }
    else
    {
        return root_id(year, day, hour, min, sec);
    }
}


}