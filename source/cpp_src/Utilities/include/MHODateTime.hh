#ifndef MHODateTime_HH__
#define MHODateTime_HH__

/*
*File: MHODateTime.hh
*Class: MHODateTime
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-28T18:12:09.299Z
*Description: Extremely basic date/time containter, needs work
*/

#include <string>
#include <time.h>
#include <ctime>

#include "MHOTimeStampConverter.hh"

namespace hops
{

class MHODateTime
{
    public:

        MHODateTime();
        MHODateTime(short year, short day, short hour, short minute, float second); //useful for init

        virtual ~HDateTime();


    private:

        std::tm fDate;


        // typedef struct date
        //     {
        //     short	year;
        //     short	day;
        //     short	hour;
        //     short	minute;
        //     float	second;
        //     } date_struct;




};




} //end of namespace

#endif /* end of include guard: MHODateTime */
