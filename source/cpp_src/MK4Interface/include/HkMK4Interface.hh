#ifndef HkMK4Interface_HH__
#define HkMK4Interface_HH__

/*
*File: HkMK4Interface.hh
*Class: HkMK4Interface
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

class HkMK4Interface
{
    public:

        HkMK4Interface();
        virtual ~HkMK4Interface();

        void OpenFringeFile(std::string file_path);


    private:


};


}//end of hops namespace

#endif /* end of include guard: HkMK4Interface */
