#ifndef HMK4CorelInterface_HH__
#define HMK4CorelInterface_HH__

/*
*File: HMK4CorelInterface.hh
*Class: HMK4CorelInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T18:54:28.140Z
*Description:
*/

#include <iostream>
#include <string>

//mk4 IO library
extern "C"
{
    #include "mk4_records.h"
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
}

#include "HMultiTypeMap.hh"

namespace hops
{

class HMK4CorelInterface
{
    public:

        HMK4CorelInterface();
        virtual ~HMK4CorelInterface();

        void ReadCorelFile(const std::string& filename);

        void ExportCorelFile();

    private:

        std::string getstr(const char* char_array, size_t max_size);

        bool fHaveCorel;
        struct mk4_corel* fCorel;

};

}//end of hops namespace

#endif /* end of include guard: HMK4CorelInterface */
