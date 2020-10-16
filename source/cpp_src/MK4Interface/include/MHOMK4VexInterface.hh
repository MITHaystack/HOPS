#ifndef MHOMK4VexInterface_HH__
#define MHOMK4VexInterface_HH__

/*
*File: MHOMK4VexInterface.hh
*Class: MHOMK4VexInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T04:44:32.593Z
*Description:
*/

#include <iostream>
#include <string>

extern "C"
{
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
}

namespace hops
{

class MHOMK4VexInterface
{
    public:

        MHOMK4VexInterface();
        virtual ~MHOMK4VexInterface();

        void OpenVexFile(std::string file_path);

        struct vex* GetVex();

        //void ExportVexFile();

    private:

        bool fHaveVex;
        struct vex* fVex;


};


}//end of hops namespace

#endif /* end of include guard: MHOMK4VexInterface */
