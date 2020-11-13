#ifndef MHOMK4StationInterface_HH__
#define MHOMK4StationInterface_HH__

/*
*File: MHOMK4StationInterface.hh
*Class: MHOMK4StationInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T18:54:28.140Z
*Description: This class implicitly assumes that the frequency/channel configuration
* is shared among all polarization products, we may want to loosen this restriction
* in the future
*/

#include <vector>
#include <cstdlib>
#include <cstring>
#include <string>

#include "MHOVisibilities.hh"
#include "MHOMessage.hh"

//forward declaration of mk4_corel and vex structs
//we do this to keep the mk4 structures from 'leaking' into the new code via includes,
//We want to make sure any interface to the old mk4 IO libraries is kept only
//within the MK4Interface library.
extern "C"
{
    struct mk4_sdata;
    struct vex;
}

namespace hops
{

class MHOMK4StationInterface
{
    public:

        MHOMK4StationInterface();
        virtual ~MHOMK4StationInterface();

        //need both the vex (root) file and corel file to extract the data
        void SetVexFile(const std::string& vex){fVexFile = vex;}
        void SetStationFile(const std::string& station){fStationFile = station;}

    private:

        //corel and vex file members
        void ReadStationFile();
        void ReadVexFile();
        bool fHaveStation;
        bool fHaveVex;
        struct mk4_sdata* fStation;
        struct vex* fVex;
        std::string fVexFile;
        std::string fStationFile;
};

}//end of hops namespace

#endif /* end of include guard: MHOMK4StationInterface */
