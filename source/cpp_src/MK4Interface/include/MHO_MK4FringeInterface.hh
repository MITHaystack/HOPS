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
#include <fstream>
#include "MHO_MK4Type200Converter.hh"
#include "MHO_MK4Type201Converter.hh"
#include "MHO_MK4Type202Converter.hh"
#include "MHO_MK4Type203Converter.hh"
#include "MHO_MK4Type204Converter.hh"
#include "MHO_MK4Type205Converter.hh"
#include "MHO_MK4Type206Converter.hh"
#include "MHO_MK4Type207Converter.hh"
#include "MHO_MK4Type208Converter.hh"
#include "MHO_MK4Type210Converter.hh"
#include "MHO_MK4Type212Converter.hh"

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

        void ExportFringeFilesToJSON(const type_200 &t200, const type_201 &t201, const type_202 &t202, const type_203 &t203, const type_204 &t204,
     const type_205 &t205, const type_206 &t206, const type_207 &t207, const type_208 &t208, const type_210 &t210, const type_212 &t212);

        //void ExportFringeFilesToJSON();

    private:

        bool fHaveFringe;
        struct mk4_fringe fFringe;




};

}//end of hops namespace

#endif /* end of include guard: MHO_MK4FringeInterface */
