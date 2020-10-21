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

#include <vector>
#include <cstdlib>
#include <cstring>
#include <string>

#include "MHOVisibilities.hh"
#include "MHOMessenger.hh"

//forward declaration of mk4_corel and vex structs
//we do this to keep the mk4 structures from 'leaking' into the new code via includes,
//We want to make sure any interface to the old mk4 IO libraries is kept only
//within the MK4Interface library.
struct mk4_corel;
struct vex;

namespace hops
{

class MHOMK4CorelInterface
{
    public:

        MHOMK4CorelInterface();
        virtual ~MHOMK4CorelInterface();

        //need both the vex (root) file and corel file to extract the data
        void SetVexFile(const std::string& vex){fVexFile = vex;}
        void SetCorelFile(const std::string& corel){fCoreFile = corel;}

        //read the vex and corel files and dump into new formatvoid

        void ExtractCorelFile();

    private:

        void ReadCorelFile();
        void ReadVexFile();

        //helper function to convert raw char arrays to strings
        std::string getstr(const char* char_array, size_t max_size);

        bool fHaveCorel;
        bool fHaveVex;
        struct mk4_corel* fCorel;
        struct mk4_vex* fVex;
        std::string fVexFile;
        std::string fCoreFile;
};

}//end of hops namespace

#endif /* end of include guard: MHOMK4CorelInterface */
