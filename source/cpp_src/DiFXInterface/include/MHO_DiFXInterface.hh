#ifndef MHO_DiFXInterface_HH__
#define MHO_DiFXInterface_HH__

/*
*@file: MHO_DiFXInterface.hh
*@class: MHO_DiFXInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_DirectoryInterface.hh"
#include "MHO_LegacyRootCodeGenerator.hh"
#include "MHO_StationCodeMap.hh"

#include "MHO_DiFXScanFileSet.hh"
#include "MHO_DiFXScanProcessor.hh"

namespace hops
{

class MHO_DiFXInterface
{
    public:

        MHO_DiFXInterface();
        virtual ~MHO_DiFXInterface();

        void SetInputDirectory(std::string dir);
        void SetOutputDirectory(std::string dir);

        void SetStationCodes(MHO_StationCodeMap* code_map);
        void SetExperimentNumber(int num){fExperNum = num;}

        void Initialize(); //read the directory and construct the scan file-lists 
        void ProcessScans(); //convert the scans 

    private:

        std::string fInputDirectory;
        std::string fOutputDirectory;
        MHO_DirectoryInterface fDirInterface;
        std::string fVexFile;
        std::string fV2DFile;
        std::vector< MHO_DiFXScanFileSet > fScanFileSetList;

        int fExperNum;
        MHO_DiFXScanProcessor fScanProcessor;
};

}//end of namespace

#endif /* end of include guard: MHO_DiFXInterface */
