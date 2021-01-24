#ifndef MHO_DateTime_HH__
#define MHO_DateTime_HH__

/*
*File: MHO_DateTime.hh
*Class: MHO_DateTime
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-28T18:12:09.299Z
*Description: Extremely basic date/time containter, needs work
*/

#include <string>
#include <time.h>
#include <ctime>

#include "MHO_TimeStampConverter.hh"

namespace hops
{

class MHO_DateTime
{
    public:

        MHO_DateTime();
        MHO_DateTime(short year, short day, short hour, short minute, float second); //useful for init

        virtual ~MHO_DateTime();


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

#endif /* end of include guard: MHO_DateTime */
