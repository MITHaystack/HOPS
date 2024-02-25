#ifndef MHO_MK4FringeInterface_HH__
#define MHO_MK4FringeInterface_HH__

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


/*!
*@file MHO_MK4FringeInterface.hh
*@class MHO_MK4FringeInterface
*@author J. Barrett - barrettj@mit.edu
*@date Thu May 28 19:47:51 2020 -0400
*@brief
*/


class MHO_MK4FringeInterface
{
    public:

        MHO_MK4FringeInterface();
        virtual ~MHO_MK4FringeInterface();

        void ReadFringeFile(const std::string& filename);

        void ExportFringeFile();

    private:

        bool fHaveFringe;
        struct mk4_fringe fFringe;




};

}//end of hops namespace

#endif /*! end of include guard: MHO_MK4FringeInterface */
