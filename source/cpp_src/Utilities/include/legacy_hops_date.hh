#ifndef LEGACY_HOPS_DATA_HH__
#define LEGACY_HOPS_DATA_HH__

namespace hops
{

/*!
 *@file  legacy_hops_date.hh
 *@class  legacy_hops_date
 *@author  J. Barrett - barrettj@mit.edu
 *@brief  A struct to avoid name collisions between the mk4utils 'data' struct and the 'date' header library
 *@date Fri Aug 18 10:22:26 2023 -0400
 */

//struct compatible with the legacy hops date format
struct legacy_hops_date
{
        short year;
        short day;
        short hour;
        short minute;
        // float second;
        double second;
};

} // namespace hops

#endif /*! end of include guard: LEGACY_HOPS_DATA_HH__ */
