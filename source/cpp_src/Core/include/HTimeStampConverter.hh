#ifndef HTimeStampConverter_HH__
#define HTimeStampConverter_HH__

#include <string>
#include <time.h>
#include <ctime>

namespace hops
{

class HTimeStampConverter
{
    public:
        HTimeStampConverter(){};
        ~HTimeStampConverter(){};

        //convert a long int epoch second count to a human readable date string in YYYY-MM-DDTHH:MM:SS.(F)Z format
        //fractional_part is rounded to the nearest nano second
        static bool ConvertEpochSecondToTimeStamp(const uint64_t& epoch_sec, const double& fractional_part, std::string& date);

        //time stamp must be in UTC/Zulu with format: YYYY-MM-DDTHH:MM:SS.(F)Z
        //number of digits in the fractional part (F) may vary
        static bool ConvertTimeStampToEpochSecond(const std::string& date, uint64_t& epoch_sec, double& fractional_part);

};

}//end of namespace

#endif /* end of include guard: HTimeStampConverter_HH__ */
