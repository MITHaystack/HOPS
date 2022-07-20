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



// #ifndef HOPS3_USE_CXX
// extern "C"
// {
// #endif
//     #include "mk4_data.h"
//     #include "mk4_dfio.h"
//     #include "mk4_vex.h"
// #ifndef HOPS3_USE_CXX
// }
// #endif

#include <iostream>
#include <string>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_VexParser.hh"

namespace hops
{

class MHO_MK4VexInterface
{
    public:

        MHO_MK4VexInterface();
        virtual ~MHO_MK4VexInterface();

        void OpenVexFile(std::string file_path);

        // //optional point the vex interface to an external struct
        // //to where the data will be read into
        // void SetVex(struct vex* root);

        // struct vex* GetVex();
        mho_json GetVex();

        bool ExportVexFileToJSON(json& json_obj);

    private:
        // 
        bool fHaveVex;
        // bool fOwnVex;

        
        mho_json fVex;

        //struct vex* fVex;


};


}//end of hops namespace

#endif /* end of include guard: MHO_MK4VexInterface */
