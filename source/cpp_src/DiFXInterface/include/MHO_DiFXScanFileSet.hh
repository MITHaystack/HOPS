#ifndef MHO_DiFXScanFileSet_HH__
#define MHO_DiFXScanFileSet_HH__

/*
*@file: MHO_DiFXScanFileSet.hh
*@class: MHO_DiFXScanFileSet
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <vector>
#include <string>

namespace hops 
{

//stores the names of all the assorted files related to a difx scan 
class MHO_DiFXScanFileSet
{
    public:
        MHO_DiFXScanFileSet(){};
        virtual ~MHO_DiFXScanFileSet(){};

        //direct public access since all we are doing is get/set anyways
        std::string fBaseDirectory;
        std::string fScanDirectory;
        std::string fInputFile;
        std::string fIMFile;
        std::string fCalcFile;
        std::string fFlagFile;
        std::string fV2DFile;
        std::string fVexFile;
        std::vector< std::string > fVisbilityFileList;
        std::vector< std::string > fPCALFileList;
};

}

#endif /* end of include guard: MHO_DiFXScanFileSet */