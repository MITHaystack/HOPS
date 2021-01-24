#ifndef MHO_MK4Interface_HH__
#define MHO_MK4Interface_HH__

/*
*File: MHO_MK4Interface.hh
*Class: MHO_MK4Interface
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

class MHO_MK4Interface
{
    public:

        MHO_MK4Interface();
        virtual ~MHO_MK4Interface();

        void OpenFringeFile(std::string file_path);


    private:


};


}//end of hops namespace

#endif /* end of include guard: MHO_MK4Interface */
