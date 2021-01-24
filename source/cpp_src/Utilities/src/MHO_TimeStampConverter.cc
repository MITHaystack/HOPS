#include "MHO_TimeStampConverter.hh"

#include <iostream>
#include <cmath>
#include <sstream>


namespace hops
{

bool MHO_TimeStampConverter::ConvertEpochSecondToTimeStamp(const uint64_t& epoch_sec, const double& fractional_part, std::string& date)
{
    if( fractional_part < 0 || fractional_part >= 1.0)
    {
        return false;
    }

    std::time_t time_point = epoch_sec;
    std::tm time_stamp;
    //use thread safe version (posix extension)
    std::tm* tmp = gmtime_r(&time_point, &time_stamp);
    (void) tmp; //shut up compiler

    char buff[128];
    for(unsigned int i=0; i<128; i++){buff[i] = '\0';}
    size_t tmp_val = std::strftime(buff, 128, "%Y-%m-%dT%H:%M:%S", &time_stamp);
    if(tmp_val == 0)
    {
        return false;
    }

    //get fractional part in nano seconds
    unsigned int nano_sec = std::round( std::abs(fractional_part/1e-9) );
    std::stringstream ss;
    ss << nano_sec;
    std::string snano_sec;
    ss >> snano_sec;
    //removing trailing zeros
    snano_sec.erase(snano_sec.find_last_not_of('0') + 1, std::string::npos);

    if(snano_sec.size() != 0)
    {
        date = std::string(buff) + std::string(".") + snano_sec + std::string("Z");
    }
    else
    {
        date = std::string(buff) + std::string("Z");
    }

    return true;
}


bool MHO_TimeStampConverter::ConvertTimeStampToEpochSecond(const std::string& date, uint64_t& epoch_sec, double& fractional_part)
{
    //number of digits in the fractional part (F) may vary
    if( date.back() != 'Z' ||  date.size() < 20 || date[10] != 'T')
    {
        //only support UTC/Zulu time zone, if something else snuck in, fail
        return false;
    }
    else
    {
        std::string syear = date.substr(0,4);
        std::string smonth = date.substr(5,2);
        std::string sday = date.substr(8,2);
        std::string shour = date.substr(11,2);
        std::string smin = date.substr(14,2);
        std::string ssec = date.substr(17,2);

        //look for delimiter to fractional second
        size_t dec_pos = date.find('.');
        std::string sfrac = "0.0";
        if(dec_pos != std::string::npos && (date.size() - dec_pos) >= 1 )
        {
            sfrac = date.substr(dec_pos, (date.size() - dec_pos) - 1);
        }

        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int min = 0;
        int sec = 0;
        double frac = 0.0;

        //convert to int/double with some sanity checks
        std::stringstream ss;
        ss.str(std::string()); ss.clear();
        ss << syear;
        ss >> year;
        std::cout<<"year = "<<year<<std::endl;
        if(year < 1970 || year > 3000 ){epoch_sec = 0; return false;}
        ss.str(std::string()); ss.clear();
        ss << smonth;
        ss >> month;
        std::cout<<"month = "<<smonth<<std::endl;
        std::cout<<"month = "<<month<<std::endl;
        if(month < 1 || month > 12 ){epoch_sec = 0; return false;}
        ss.str(std::string()); ss.clear();
        ss << sday;
        ss >> day;
        std::cout<<"day = "<<day<<std::endl;
        if(day < 1 || day > 31 ){epoch_sec = 0; return false;}
        ss.str(std::string()); ss.clear();
        ss << shour;
        ss >> hour;
        std::cout<<"hour = "<<hour<<std::endl;
        if(hour < 0 || hour > 23 ){epoch_sec = 0; return false;}
        ss.str(std::string()); ss.clear();
        ss << smin;
        ss >> min;
        std::cout<<"min = "<<min<<std::endl;
        if(min < 0 || min > 59 ){epoch_sec = 0; return false;}
        ss.str(std::string()); ss.clear();
        ss << ssec;
        ss >> sec;
        std::cout<<"sec = "<<sec<<std::endl;
        if( sec < 0 || sec > 61 ){epoch_sec = 0; return false;}
        ss.str(std::string()); ss.clear();
        ss << sfrac;
        ss >> frac;
        std::cout<<"frac = "<<frac<<std::endl;
        if( frac < 0.0 || frac > 1.0 ){epoch_sec = 0; return false;}

        // tm_sec	int	seconds after the minute	0-61*
        // tm_min	int	minutes after the hour	0-59
        // tm_hour	int	hours since midnight	0-23
        // tm_mday	int	day of the month	1-31
        // tm_mon	int	months since January	0-11
        // tm_year	int	years since 1900
        // tm_wday	int	days since Sunday	0-6
        // tm_yday	int	days since January 1	0-365
        // tm_isdst	int	Daylight Saving Time flag

        //now convert year, doy, hour, min, sec to epoch second
        std::tm tmdate;
        tmdate.tm_sec = sec;
        tmdate.tm_min = min;
        tmdate.tm_hour = hour;
        tmdate.tm_mon = month - 1;
        tmdate.tm_mday = day;
        tmdate.tm_year = year - 1900;
        tmdate.tm_isdst	= 0;
        std::time_t epsec = timegm(&tmdate);

        epoch_sec = (uint64_t) epsec;
        fractional_part = frac;

        return true;
    }

    //format error
    epoch_sec = 0;
    return false;
}



}
