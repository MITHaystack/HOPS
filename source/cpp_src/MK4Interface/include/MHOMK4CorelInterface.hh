#ifndef MHOMK4CorelInterface_HH__
#define MHOMK4CorelInterface_HH__

/*
*File: MHOMK4CorelInterface.hh
*Class: MHOMK4CorelInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T18:54:28.140Z
*Description:
*/


//mk4 IO library
extern "C"
{
    #include "mk4_records.h"
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
}


#include <iostream>
#include <string>
#include <vector>
#include <complex>


#include "MHOMultiTypeMap.hh"

namespace hops
{

typedef MHOMultiTypeMap< std::string, std::string, int, short, float, double > Type100MetaData;
typedef MHOMultiTypeMap< std::string, std::string, short, int, std::vector<int> > Type101Map;
typedef MHOMultiTypeMap< std::string, std::string, short, int, float, std::vector< std::complex<double> > > Type120Map;

class MHOMK4CorelInterface
{
    public:

        MHOMK4CorelInterface();
        virtual ~MHOMK4CorelInterface();

        void ReadCorelFile(const std::string& filename);

        struct mk4_corel* GetCorel();

        void ExportCorelFile(Type100MetaData& meta, std::vector< Type101Map >& type101vector, std::vector< Type120Map >& type120vector);

        std::string getstr(const char* char_array, size_t max_size);

    private:


        bool fHaveCorel;
        struct mk4_corel* fCorel;

};

}//end of hops namespace

#endif /* end of include guard: MHOMK4CorelInterface */
