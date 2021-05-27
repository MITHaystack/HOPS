#ifndef MHO_MK4VexInterface_HH__
#define MHO_MK4VexInterface_HH__

/*
*File: MHO_MK4VexInterface.hh
*Class: MHO_MK4VexInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T04:44:32.593Z
*Description:
*/



extern "C"
{
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
}

#include <iostream>
#include <string>
#include "json_wrapper.hh"

namespace hops
{

class MHO_MK4VexInterface
{
    public:

        MHO_MK4VexInterface();
        virtual ~MHO_MK4VexInterface();

        void OpenVexFile(std::string file_path);

        struct vex* GetVex();

        bool ExportVexFileToJSON(json& json_obj);

    private:

        bool fHaveVex;
        struct vex* fVex;


};


}//end of hops namespace

#endif /* end of include guard: MHO_MK4VexInterface */
