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
        std::string fScanName; //difx scan name
        std::string fInputBaseDirectory; //root input directory for this experiment
        std::string fOutputBaseDirectory; //root output directory for this experiment
        std::string fScanDirectory; //.difx directory 
        std::string fInputFile; //.input file associated with this scan
        std::string fIMFile; //.im file associated with this scan
        std::string fCalcFile; //.calc file associated with this scan 
        std::string fFlagFile; //.flag file associated with this scan
        std::string fV2DFile; //.v2d file for this experiment 
        std::string fVexFile; //.vex file for this experiment 
        std::vector< std::string > fVisibilityFileList; //list of all DIFX_ files under the .difx directory 
        std::vector< std::string > fPCALFileList; //list of all PCAL_ files under the .difx directory 
};

}

#endif /* end of include guard: MHO_DiFXScanFileSet */