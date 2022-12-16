#ifndef MHO_MK4FringeInterface_HH__
#define MHO_MK4FringeInterface_HH__

/*
*File: MHO_MK4FringeInterface.hh
*Class: MHO_MK4FringeInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T18:54:28.140Z
*Description:
*/

#include <iostream>
#include <string>

//mk4 IO library
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    #include "mk4_data.h"
    #include "mk4_dfio.h"
#ifndef HOPS3_USE_CXX
}
#endif

#include "MHO_MultiTypeMap.hh"

namespace hops
{

class MHO_MK4FringeInterface
{
    public:

        MHO_MK4FringeInterface();
        virtual ~MHO_MK4FringeInterface();

        void ReadFringeFile(const std::string& filename);

        void ExportFringeFilesToStructs();

        void ExportFringeStructsToJSON();

        void ExportFringeFilesToJSON(const std::string& outputfile);

    private:

        bool fHaveFringe;
        struct mk4_fringe fFringe;




};

}//end of hops namespace

#endif /* end of include guard: MHO_MK4FringeInterface */
