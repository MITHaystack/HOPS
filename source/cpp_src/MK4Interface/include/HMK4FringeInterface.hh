#ifndef HMK4FringeInterface_HH__
#define HMK4FringeInterface_HH__

/*
*File: HMK4FringeInterface.hh
*Class: HMK4FringeInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T18:54:28.140Z
*Description:
*/

#include <iostream>
#include <string>

//mk4 IO library
extern "C"
{
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
}

#include "HMultiTypeMap.hh"

namespace hops
{

class HMK4FringeInterface
{
    public:

        HMK4FringeInterface();
        virtual ~HMK4FringeInterface();

        void ReadFringeFile(const std::string& filename);

        void ExportFringeFile();

    private:

        bool fHaveFringe;
        struct mk4_fringe fFringe;




};

}//end of hops namespace

#endif /* end of include guard: HMK4FringeInterface */
