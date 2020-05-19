#ifndef HMK4Interface_HH__
#define HMK4Interface_HH__

/*
*File: HMK4Interface.hh
*Class: HMK4Interface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T04:44:32.593Z
*Description:
*/

#include <iostream>
#include <string>

extern "C"
{
    #define T230_VERSION 0 //have to do this of conflicting complex.h in type_230
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
}

namespace hops
{

class HMK4Interface
{
    public:

        HMK4Interface();
        virtual ~HMK4Interface();

        void OpenFringeFile(std::string file_path);


    private:


};


}//end of hops namespace

#endif /* end of include guard: HMK4Interface */
