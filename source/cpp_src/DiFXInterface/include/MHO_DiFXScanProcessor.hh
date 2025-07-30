#ifndef MHO_DiFXScanProcessor_HH__
#define MHO_DiFXScanProcessor_HH__

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DiFXChannelNameConstructor.hh"
#include "MHO_DiFXInputProcessor.hh"
#include "MHO_DiFXPCalProcessor.hh"
#include "MHO_DiFXScanFileSet.hh"
#include "MHO_DiFXVisibilityProcessor.hh"
#include "MHO_DiFXVisibilityRecord.hh"
#include "MHO_DirectoryInterface.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_StationCodeMap.hh"

#include "MHO_DiFXTimeUtilities.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXScanProcessor.hh
 *@class  MHO_DiFXScanProcessor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Mon Jan 31 14:35:02 2022 -0500
 *@brief
 */

/**
 * @brief Class MHO_DiFXScanProcessor
 */
class MHO_DiFXScanProcessor
{
    public:
        MHO_DiFXScanProcessor();
        virtual ~MHO_DiFXScanProcessor();

        /**
         * @brief Setter for root code
         * 
         * @param rcode New root code as string
         */
        void SetRootCode(std::string rcode) { fRootCode = rcode; }

        /**
         * @brief Setter for experiment number
         * 
         * @param num New experiment number to set
         */
        void SetExperimentNumber(int num) { fExperNum = num; }

        /**
         * @brief Setter for station codes
         * 
         * @param code_map Pointer to an MHO_StationCodeMap object containing station codes.
         */
        void SetStationCodes(MHO_StationCodeMap* code_map);
        /**
         * @brief Processes a DiFX scan file set by loading input files, creating output directories, converting visibility and station files.
         * 
         * @param fileSet Reference to MHO_DiFXScanFileSet containing input files
         */
        void ProcessScan(MHO_DiFXScanFileSet& fileSet);

        /**
         * @brief Setter for normalize false - Do NOT apply mk4 style visibility normalization
         */
        void SetNormalizeFalse() { fNormalize = false; }

        /**
         * @brief Setter for normalize true - apply mk4 style visibility normalization
         */
        void SetNormalizeTrue() { fNormalize = true; }

        /**
         * @brief Setter for preserve difx scan names true
         */
        void SetPreserveDiFXScanNamesTrue() { fPreserveDiFXScanNames = true; }

        /**
         * @brief Setter for preserve difx scan names false
         */
        void SetPreserveDiFXScanNamesFalse() { fPreserveDiFXScanNames = false; };
        
        /**
         * @brief Setter for attach difx .input data true
         */
        void SetAttachDiFXInputTrue() {fAttachDiFXInput = true; }
        
        /**
         * @brief Setter for attach difx .input data false
         */
        void SetAttachDiFXInputFalse() {fAttachDiFXInput = false; }
        
        void SetExportAsMark4True(){fExportAsMark4 = true;}
        void SetExportAsMark4False(){fExportAsMark4 = false;}

        /**
         * @brief Setter for frequency bands (name, limits)
         * 
         * @param fbands Vector of tuples where each tuple contains a string (band name), double (lower freq), double (upper freq)
         */
        void SetFrequencyBands(std::vector< std::tuple< std::string, double, double > > fbands)
        {
            fFreqBands = fbands;
            for(std::size_t i = 0; i < fFreqBands.size(); i++)
            {
                auto btup = fFreqBands[i];
                fChanNameConstructor.AddBandLabel(std::get< 0 >(btup), std::get< 1 >(btup), std::get< 2 >(btup));
            }
        }

        /**
         * @brief Setter for (allowed) freq groups/bands
         * 
         * @param fgroups Vector of strings representing frequency groups
         */
        void SetFreqGroups(std::vector< std::string > fgroups) { fFreqGroups = fgroups; }

        /**
         * @brief Setter for allowed channel bandwidth - channels with other bandwidths will be discarded
         * 
         * @param bw allowed channel bandwidth value.
         */
        void SetOnlyBandwidth(double bw)
        {
            fOnlyBandwidth = bw;
            fSelectByBandwidth = true;
        }

        //use json representation of vex-scan information to return epoch string of frt
        /**
         * @brief Calculates and returns the fourfit reference time epoch string for a given scan object.
         * 
         * @param scan_obj Input mho_json scan object containing station data
         * @return std::string representing the fourfit reference time in VEX format
         */
        std::string get_fourfit_reftime_for_scan(mho_json scan_obj);

        // //given a mjd date and number of seconds, compute the vex string representation
        // static std::string get_vexdate_from_mjd_sec(double mjd, double sec);

        /**
         * @brief Retrieves the correlation date as a string.
         * 
         * @return std::string representing the correlation date.
         */
        std::string get_correlation_vexdate() const { return fCorrDate; };

    private:
        /**
         * @brief Applies delay model clock correction to station coordinates using antenna and polynomial data.
         * 
         * @param ant Antenna information as mho_json object.
         * @param ant_poly Antenna polynomial data as mho_json object.
         * @param st_coord Station coordinate type pointer, modified in-place.
         */
        void apply_delay_model_clock_correction(const mho_json& ant, const mho_json& ant_poly, station_coord_type* st_coord);
        
        /**
         * @brief Calculates shifted clock values for a given antenna using polynomial coefficients and time difference.
         * 
         * @param da Input JSON object containing clock order and coefficients.
         * @param dt Time difference in seconds.
         * @param outputClockSize Size of the output clock array.
         * @param clockOut Output array to store shifted clock values.
         * @return Number of valid clock orders plus one, or an error code (-1 if 'clockorder' or 'clockcoeff' is missing, -2 if outputClockSize is insufficient).
         */
        int local_getDifxAntennaShiftedClock(const mho_json& da, double dt, int outputClockSize, double* clockOut);
        
        /**
         * @brief Calculates zeroth order parallactic angle using station coordinates and source declination.
         * 
         * @param st_coord Input: Station coordinate structure
         * @param X Input: X-coordinate of station
         * @param Y Input: Y-coordinate of station
         * @param Z Input: Z-coordinate of station
         * @param src_dec Input: Source declination in degrees
         * @param dt Input: Time interval for polynomial model
         */
        void calculateZerothOrderParallacticAngle(station_coord_type* st_coord, double X, double Y, double Z, double src_dec,
                                                  double dt);

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

        std::string ConstructRootFileName(const std::string& output_dir, const std::string& root_code,
                                          const std::string& src_name);

        std::string ConstructStaFileName(const std::string& output_dir, const std::string& root_code,
                                         const std::string& station_code, const std::string& station_mk4id);

        void PatchOvexStructures(mho_json& vex_root, std::string mode_name);

        //the DiFX input file structure
        mho_json fInput;

        //the root code assigned to this scan
        std::string fRootCode;
        std::string fCorrDate; //the correlation data as a vex-formatted string

        //integer experiment number
        int fExperNum;
        bool fNormalize;
        bool fExportAsMark4;

        //the output directory for this scan
        std::string fOutputDirectory;

        std::map< int, MHO_DiFXBaselineProcessor > fAllBaselineVisibilities;
        MHO_DiFXPCalProcessor fPCalProcessor;

        std::map< std::string, multitone_pcal_type* > fStationCode2PCal;
        std::map< std::string, station_coord_type* > fStationCode2Coords;

        //difx -> vex (2 char) station code map
        //this is needed because DiFX arbitrarily converts all 2-character
        //station codes to upper case
        std::map< std::string, std::string > fDiFX2VexStationCodes;
        std::map< std::string, std::string > fDiFX2VexStationNames;

        //generates the channel names (zoom bands not yet supported)
        //TODO -- allow band <-> freq range to be set externally
        MHO_DiFXChannelNameConstructor fChanNameConstructor;

        bool fPreserveDiFXScanNames;
        bool fAttachDiFXInput;

        double MICROSEC_TO_SEC; //needed to match difx2mark4 convention

        //frequency band labelling/selection
        std::vector< std::tuple< std::string, double, double > > fFreqBands; //frequency band/group labels and ranges
        std::vector< std::string > fFreqGroups;                              //limit output to matching frequency groups
        bool fSelectByBandwidth;
        double fOnlyBandwidth; //limit output to only channels of this bandwidth
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXScanProcessor */
