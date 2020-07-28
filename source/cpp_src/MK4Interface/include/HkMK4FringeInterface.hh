#ifndef HkMK4FringeInterface_HH__
#define HkMK4FringeInterface_HH__

/*
*File: HkMK4FringeInterface.hh
*Class: HkMK4FringeInterface
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

#include "HkMultiTypeMap.hh"

namespace hops
{

class HkMK4FringeInterface
{
    public:

        HkMK4FringeInterface();
        virtual ~HkMK4FringeInterface();

        void ReadFringeFile(const std::string& filename);

        void ExportFringeFile();

    private:

        bool fHaveFringe;
        struct mk4_fringe fFringe;




};

}//end of hops namespace

#endif /* end of include guard: HkMK4FringeInterface */
