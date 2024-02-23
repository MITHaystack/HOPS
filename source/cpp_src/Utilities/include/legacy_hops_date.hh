#ifndef LEGACY_HOPS_DATA_HH__
#define LEGACY_HOPS_DATA_HH__

/*!
*@file: legacy_hops_date.hh
*@class: legacy_hops_date
*@author: J. Barrett
*@email: barrettj@mit.edu
*@brief: A struct to avoid name collisions between the mk4utils 'data' struct and the 'date' header library
*@date:
*/

namespace hops
{

//struct compatible with the legacy hops date format
struct legacy_hops_date
{
    short   year;
    short   day;
    short   hour;
    short   minute;
    float   second;
};

}

#endif /* end of include guard: LEGACY_HOPS_DATA_HH__ */
