#ifndef HkMK4CorelInterface_HH__
#define HkMK4CorelInterface_HH__

/*
*File: HkMK4CorelInterface.hh
*Class: HkMK4CorelInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T18:54:28.140Z
*Description:
*/

#include <iostream>
#include <string>
#include <vector>

//mk4 IO library
extern "C"
{
    #include "mk4_records.h"
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
}

#include "HkMultiTypeMap.hh"

namespace hops
{

typedef HkMultiTypeMap< std::string, std::string, int, short, float, double > Type100MetaData;
typedef HkMultiTypeMap< std::string, std::string, short, int, std::vector<int> > Type101Map;
typedef HkMultiTypeMap< std::string, std::string, short, int, float, std::vector< std::complex<double> > > Type120Map;

class HkMK4CorelInterface
{
    public:

        HkMK4CorelInterface();
        virtual ~HkMK4CorelInterface();

        void ReadCorelFile(const std::string& filename);

        void ExportCorelFile(Type100MetaData& meta, std::vector< Type101Map >& type101vector, std::vector< Type120Map >& type120vector);

    private:

        std::string getstr(const char* char_array, size_t max_size);

        bool fHaveCorel;
        struct mk4_corel* fCorel;

};

}//end of hops namespace

#endif /* end of include guard: HkMK4CorelInterface */
