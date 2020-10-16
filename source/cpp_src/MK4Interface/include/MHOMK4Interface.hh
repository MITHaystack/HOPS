#ifndef MHOMK4Interface_HH__
#define MHOMK4Interface_HH__

/*
*File: MHOMK4Interface.hh
*Class: MHOMK4Interface
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

class MHOMK4Interface
{
    public:

        MHOMK4Interface();
        virtual ~MHOMK4Interface();

        void OpenFringeFile(std::string file_path);


    private:


};


}//end of hops namespace

#endif /* end of include guard: MHOMK4Interface */
