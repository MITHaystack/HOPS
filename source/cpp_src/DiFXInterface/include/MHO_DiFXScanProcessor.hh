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

#include "difxio/difx_input.h"
#include "difxio/parsevis.h"

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

namespace hops 
{

class MHO_DiFXScanProcessor
{
    public:

        MHO_DiFXScanProcessor();
        virtual ~MHO_DiFXScanProcessor();

        void SetStationCodes(std::map<std::string, std::string> code_map);
        void ProcessScan(MHO_DiFXScanFileSet& fileSet);

    private:

        //the station codes 
        std::map< std::string, std::string> fStationCodeMap;

        ////////////////////////////////////////////////////////////////////////
        //members for dealing with a single (current) scan of data /////////////
        MHO_DiFXScanFileSet* fFileSet;

        void LoadInputFile();

        // void ReadPCALFile(std::string filename);
        // void ReadIMFile(std::string filename);
        void ConvertRootFileObject();
        void ConvertVisibilityFileObjects();
        void ConvertStationFileObjects();

        void ExtractStationCoords();
        void CleanUp();

        //the DiFX input file structure 
        json fInput;

        std::map< int, MHO_DiFXBaselineProcessor > fAllBaselineVisibilities;
        MHO_DiFXPCalProcessor fPCalProcessor;
        std::map< std::string, multitone_pcal_type* > fStationCode2PCal;
        std::map< std::string, station_coord_type* > fStationCode2Coords;

};

}//end of hops namespace

#endif /* end of include guard: MHO_DiFXScanProcessor */