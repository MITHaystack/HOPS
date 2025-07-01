#ifndef MHO_TimeStamp_HH__
#define MHO_TimeStamp_HH__

#include <chrono>
#include <string>

#include "MHO_Clock.hh"

namespace hops
{

/*!
 *@file  MHO_TimeStamp.hh
 *@class  MHO_TimeStamp
 *@author  J. Barrett - barrettj@mit.edu
 *@date Tue May 17 14:00:36 2022 -0400
 *@brief
 */

/**
 * @brief Class MHO_TimeStamp
 */
class MHO_TimeStamp
{
    public:
        MHO_TimeStamp(){};
        virtual ~MHO_TimeStamp(){};

        void std::string GetClockEpoch();
        void std::string GetTicks();

        /**
         * @brief Function FromTimeStamp
         * @note This is a virtual function.
         */
        virtual void FromTimeStamp(){};
        /**
         * @brief Function ToTimeStamp
         * @note This is a virtual function.
         */
        virtual void ToTimeStamp()

            virtual std::string to_iso8601_format(); //
        /**
         * @brief Function to_vex_format
         * 
         * @return Return value (std::string)
         * @note This is a virtual function.
         */
        virtual std::string to_vex_format();         //

    private:
        // std::string vex_to_iso8601(std::string vex_date_time);
        //
        // std::string iso8601_to_vex(std::string )

        hops_clock::time_point fTimePoint;
};

} // namespace hops

#endif /*! end of include guard: MHO_TimeStamp */
