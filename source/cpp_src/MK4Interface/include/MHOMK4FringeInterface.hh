#ifndef MHOMK4FringeInterface_HH__
#define MHOMK4FringeInterface_HH__

/*
*File: MHOMK4FringeInterface.hh
*Class: MHOMK4FringeInterface
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

#include "MHOMultiTypeMap.hh"

namespace hops
{

class MHOMK4FringeInterface
{
    public:

        MHOMK4FringeInterface();
        virtual ~MHOMK4FringeInterface();

        void ReadFringeFile(const std::string& filename);

        void ExportFringeFile();

    private:

        bool fHaveFringe;
        struct mk4_fringe fFringe;




};

}//end of hops namespace

#endif /* end of include guard: MHOMK4FringeInterface */
