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
#include "MHO_DiFXChannelNameConstructor.hh"
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

        void SetNormalizeFalse(){fNormalize = false;}
        void SetNormalizeTrue(){fNormalize = true;}

        void SetPreserveDiFXScanNamesTrue(){fPreserveDiFXScanNames = true;}
        void SetPreserveDiFXScanNamesFalse(){fPreserveDiFXScanNames = false;};

        void SetFrequencyBands(std::vector< std::tuple<std::string, double, double> > fbands){fFreqBands = fbands;}
        void SetFreqGroups(std::vector< std::string > fgroups){fFreqGroups = fgroups;}
        void SetOnlyBandwidth(double bw){fOnlyBandwidth = bw; fSelectByBandwidth = true;}

        //use json representation of vex-scan information to return epoch string of frt
        std::string get_fourfit_reftime_for_scan(mho_json scan_obj);

        //given a mjd date and number of seconds, compute the vex string representation
        std::string get_vexdate_from_mjd_sec(double mjd, double sec);

        std::string get_correlation_vexdate() const {return fCorrDate;} ;

    private:

        void apply_delay_model_clock_correction(const mho_json& ant, const mho_json& ant_poly, station_coord_type* st_coord);
        int local_getDifxAntennaShiftedClock(const mho_json& da, double dt, int outputClockSize, double *clockOut);
        void calculateZerothOrderParallacticAngle(station_coord_type* st_coord, double X, double Y, double Z, double src_dec, double dt);

        //the station 2-char to 1-char code map (user specified)
        MHO_StationCodeMap* fStationCodeMap;

        ////////////////////////////////////////////////////////////////////////
        //members for dealing with a single (current) scan of data /////////////
        MHO_DiFXScanFileSet* fFileSet;

        bool CreateScanOutputDirectory();
        void LoadInputFile();
        void CreateRootFileObject(std::string vexfile);
        void ConvertVisibilityFileObjects();
        void NormalizeVisibilities();
        void ConvertStationFileObjects();

        void ExtractPCalData();
        void ExtractStationCoords();
        void CleanUp();

        //the DiFX input file structure
        mho_json fInput;

        //the root code assigned to this scan
        std::string fRootCode;
        std::string fCorrDate; //the correlation data as a vex-formatted string

        //integer experiment number
        int fExperNum;
        bool fNormalize;

        //the output directory for this scan
        std::string fOutputDirectory;

        std::map< int, MHO_DiFXBaselineProcessor > fAllBaselineVisibilities;
        MHO_DiFXPCalProcessor fPCalProcessor;

        std::map< std::string, multitone_pcal_type* > fStationCode2PCal;
        std::map< std::string, station_coord_type* > fStationCode2Coords;

        //generates the channel names (zoom bands not yet supported)
        //TODO -- allow band <-> freq range to be set externally
        MHO_DiFXChannelNameConstructor fChanNameConstructor;

        bool fPreserveDiFXScanNames;

        double MICROSEC_TO_SEC; //needed to match difx2mark4 convention

        //frequency band labelling/selection
        std::vector< std::tuple<std::string, double, double> > fFreqBands; //frequency band/group labels and ranges
        std::vector< std::string > fFreqGroups; //limit output to matching frequency groups
        bool fSelectByBandwidth;
        double fOnlyBandwidth; //limit output to only channels of this bandwidth


};

}//end of hops namespace

#endif /* end of include guard: MHO_DiFXScanProcessor */
