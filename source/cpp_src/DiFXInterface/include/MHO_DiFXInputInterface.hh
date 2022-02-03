#ifndef MHO_DiFXInputInterface_HH__
#define MHO_DiFXInputInterface_HH__

/*
*@file: MHO_DiFXInputInterface.hh
*@class: MHO_DiFXInputInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <string>
#include <vector>
#include <algorithm>
#include <map>


#include "difxio/difx_input.h"
#include "difxio/parsevis.h"

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DirectoryInterface.hh"

#include "MHO_DiFXScanFileSet.hh"
#include "MHO_DiFXVisibilityRecord.hh"

namespace hops
{

class MHO_DiFXInputInterface
{
    public:
        MHO_DiFXInputInterface();
        virtual ~MHO_DiFXInputInterface();

        void SetInputDirectory(std::string dir);
        void SetOutputDirectory(std::string dir);

        void Initialize(); //read the directory and construct the scan file-lists 
        void ProcessScans(); //convert the scans 

    private:

        void ProcessScan(MHO_DiFXScanFileSet& fileSet);
        void OrganizeBaseline(int baseline);

        void ReadDIFX_File(std::string filename);
        // void ReadPCAL_File(std::string filename);
        // void ReadIM_File(std::string filename);
        void ReadInputFile(std::string filename);

        std::string fInputDirectory;
        std::string fOutputDirectory;
        MHO_DirectoryInterface fDirInterface;

        std::string fVexFile;
        std::string fV2DFile;
        std::vector< MHO_DiFXScanFileSet > fScanFileSetList;

        ////////////////////////////////////////////////////////////////////////
        //members for dealing with a single (current) scan of data /////////////
        //maps DiFX baseline index to vector of all associated visibility records 
        std::map<int, std::vector<MHO_DiFXVisibilityRecord> > fBaselineVisibilities;
        //maps freqindex to vector of associated visiblity records
        std::map<int, std::vector<MHO_DiFXVisibilityRecord> > fChannels;
        

        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////


        //comparison predicate for time-sorting data
        typedef struct 
        {
            bool operator()(MHO_DiFXVisibilityRecord& a, MHO_DiFXVisibilityRecord& b) const 
            {
                if(a.mjd == b.mjd){return a.seconds < b.seconds;}
                else{return a.mjd < b.mjd;}
            }
        } VisRecordTimeLess;
        VisRecordTimeLess fTimePredicate;

};

}//end of namespace

#endif /* end of include guard: MHO_DiFXInputInterface */