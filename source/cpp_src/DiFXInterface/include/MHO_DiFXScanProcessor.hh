#ifndef MHO_DiFXScanProcessor_HH__
#define MHO_DiFXScanProcessor_HH__

/*
*@file: MHO_DiFXScanProcessor.hh
*@class: MHO_DiFXScanProcessor
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

#include "MHO_ContainerDefinitions.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DiFXScanFileSet.hh"
#include "MHO_DiFXVisibilityRecord.hh"
#include "MHO_DiFXVisibilityProcessor.hh"
#include "MHO_DiFXInputProcessor.hh"
#include "MHO_DiFXPCalProcessor.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_StationCodeMap.hh"
#include "MHO_DirectoryInterface.hh"

namespace hops 
{

class MHO_DiFXScanProcessor
{
    public:

        MHO_DiFXScanProcessor();
        virtual ~MHO_DiFXScanProcessor();

        void SetRootCode(std::string rcode){fRootCode = rcode;}
        void SetExperimentNumber(int num){fExperNum = num;}
        void SetStationCodes(MHO_StationCodeMap* code_map);
        void ProcessScan(MHO_DiFXScanFileSet& fileSet);


        void SetPreserveDiFXScanNamesTrue(){fPreserveDiFXScanNames = true;}
        void SetPreserveDiFXScanNamesFalse(){fPreserveDiFXScanNames = false;};

        //use json representation of vex-scan information to return epoch string of frt
        std::string get_fourfit_reftime_for_scan(mho_json scan_obj);

    private:

        //the station 2-char to 1-char code map (user specified)
        MHO_StationCodeMap* fStationCodeMap;

        ////////////////////////////////////////////////////////////////////////
        //members for dealing with a single (current) scan of data /////////////
        MHO_DiFXScanFileSet* fFileSet;

        bool CreateScanOutputDirectory();
        void LoadInputFile();
        void CreateRootFileObject(std::string vexfile);
        void ConvertVisibilityFileObjects();
        void ConvertStationFileObjects();

        void ExtractPCalData();
        void ExtractStationCoords();
        void CleanUp();

        //the DiFX input file structure 
        json fInput;

        //the root code assigned to this scan 
        std::string fRootCode;

        //integer experiment number 
        int fExperNum;

        //the output directory for this scan 
        std::string fOutputDirectory;

        std::map< int, MHO_DiFXBaselineProcessor > fAllBaselineVisibilities;
        MHO_DiFXPCalProcessor fPCalProcessor;

        std::map< std::string, multitone_pcal_type* > fStationCode2PCal;
        std::map< std::string, station_coord_type* > fStationCode2Coords;

        bool fPreserveDiFXScanNames;

};

}//end of hops namespace

#endif /* end of include guard: MHO_DiFXScanProcessor */