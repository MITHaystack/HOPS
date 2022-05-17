#ifndef MHO_TimeStamp_HH__
#define MHO_TimeStamp_HH__

/*
*@file: MHO_TimeStamp.hh
*@class: MHO_TimeStamp
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <chrono>
#include <string>

#include "MHO_Clocks.hh"

namespace hops
{

class MHO_TimeStamp
{
    public:

        MHO_TimeStamp(){};
        virtual ~MHO_TimeStamp(){};
    
        void std::string GetClockEpoch();
        void std::string GetTicks();


        virtual void FromTimeStamp(){};
        virtual void ToTimeStamp()

        virtual std::string ToString();
        virtual bool FromString(std::string timestamp);

        



    private:

        std::string fClockEpoch;
        int64_t fNanoseconds

};


}

#endif /* end of include guard: MHO_TimeStamp */