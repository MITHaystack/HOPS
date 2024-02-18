#ifndef MHO_TimeStampConverter_HH__
#define MHO_TimeStampConverter_HH__

#include <string>
#include <time.h>
#include <ctime>


namespace hops
{

/**
*@file MHO_TimeStampConverter.hh
*@class MHO_TimeStampConverter
*@author J. Barrett - barrettj@mit.edu
* A class responsible for converting times/dates stored in a human readable string
* into Unix epoch seconds and fractions of a second. The conversion can be done
* in both directions
*/


class MHO_TimeStampConverter
{
    public:
        MHO_TimeStampConverter(){};
        ~MHO_TimeStampConverter(){};

        /** convert a long int epoch second count to a human readable date string in YYYY-MM-DDTHH:MM:SS.(F)Z format.
        * The fractional_part is rounded to the nearest nano second
        * @param epoch_sec const reference to a uint64_t to the epoch second
        * @param fractional_part const reference to a double containing the factional second
        * @param date reference to a std::string object where the date string is to be stored
        * @returns a boolean indicating if the conversion was successful
        */
        static bool ConvertEpochSecondToTimeStamp(const uint64_t& epoch_sec, const double& fractional_part, std::string& date);

        /** Convert a date string into an epoch second and fractional second.
        * The time stamp must be in UTC/Zulu with format: YYYY-MM-DDTHH:MM:SS.(F)Z, the
        * number of digits in the fractional part (F) may vary.
        * @param date const reference to the date string to be converted
        * @param epoch_sec reference to a uint64_t where the epoch second value is to be stored
        * @param fractional_part reference to a double where the factional second value is to be stored
        * @returns a boolean indicating if the conversion was successful
        */
        static bool ConvertTimeStampToEpochSecond(const std::string& date, uint64_t& epoch_sec, double& fractional_part);

};

}//end of namespace

#endif /* end of include guard: MHO_TimeStampConverter_HH__ */
