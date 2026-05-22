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
#include "MHO_DiFXOvexPatcher.hh"
#include "MHO_DiFXPCalProcessor.hh"
#include "MHO_DiFXScanFileSet.hh"
#include "MHO_DiFXStationCoordBuilder.hh"
#include "MHO_DiFXVisibilityProcessor.hh"
#include "MHO_DiFXVisibilityRecord.hh"
#include "MHO_DiFXZoomBandRebuilder.hh"
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
        void SetAttachDiFXInputTrue() { fAttachDiFXInput = true; }

        /**
         * @brief Setter for attach difx .input data false
         */
        void SetAttachDiFXInputFalse() { fAttachDiFXInput = false; }

        void SetExportAsMark4True() { fExportAsMark4 = true; }

        void SetExportAsMark4False() { fExportAsMark4 = false; }

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

        /**
         * @brief Retrieves the correlation date as a string.
         *
         * @return std::string representing the correlation date.
         */
        std::string get_correlation_vexdate() const { return fCorrDate; };

    private:
        //the station 2-char to 1-char code map (user specified)
        MHO_StationCodeMap* fStationCodeMap;

        ////////////////////////////////////////////////////////////////////////
        //members for dealing with a single (current) scan of data /////////////
        MHO_DiFXScanFileSet* fFileSet;

        bool CreateScanOutputDirectory();
        void LoadInputFile();
        void CreateRootFileObject(std::string vexfile);

        //finalize the OVEX/root.json after visibilities have been processed: assigns
        //chan_def.channel_name using the scan-wide global sky-freq grid and writes the
        //ovex + root.json files. Split out of CreateRootFileObject so channel naming can
        //see the union of actually-exported sky_freqs (only known after visibility records
        //have been read and each baseline organized).
        void FinalizeAndWriteRootFile();

        void ConvertVisibilityFileObjects();
        void NormalizeVisibilities();
        void ConvertStationFileObjects();

        //compute the union (deduped, sorted ascending) of sky_freqs across every channel
        //that any baseline in fAllBaselineVisibilities will export. Each baseline must have
        //been Organize()'d first so its fBaselineFreqs is populated.
        std::vector< double > ComputeGlobalSkyFreqGrid(double tol = 0.001) const;

        void ExtractPCalData();
        void ExtractStationCoords();
        void CleanUp();

        std::string ConstructRootFileName(const std::string& output_dir, const std::string& root_code,
                                          const std::string& src_name);

        std::string ConstructStaFileName(const std::string& output_dir, const std::string& root_code,
                                         const std::string& station_code, const std::string& station_mk4id);

        //the DiFX input file structure
        mho_json fInput;

        //the root code assigned to this scan
        std::string fRootCode;
        std::string fCorrDate; //the correlation data as a vex-formatted string

        //integer experiment number
        int fExperNum;
        bool fNormalize;
        bool fExportAsMark4;

        //place to cache the OVEX/ROOT file data
        //this object only needs to be cached if we are exporting to mark4 types
        mho_json fRootJSON;

        //the output directory for this scan
        std::string fOutputDirectory;

        //cached so FinalizeAndWriteRootFile can emit the ovex/root.json after visibilities
        //have been processed. Populated by CreateRootFileObject.
        std::string fSrcName;
        std::string fDiFXInputFilename;

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

        //builds station_coord_type entries from the DiFX .input json
        MHO_DiFXStationCoordBuilder fStationCoordBuilder;

        //synthesizes $FREQ/$BBC/$IF for zoom-band stations
        MHO_DiFXZoomBandRebuilder fZoomBandRebuilder;

        //in-place OVEX structural fix-ups (run inside CreateRootFileObject)
        MHO_DiFXOvexPatcher fOvexPatcher;

        bool fPreserveDiFXScanNames;
        bool fAttachDiFXInput;

        //frequency band labelling/selection
        std::vector< std::tuple< std::string, double, double > > fFreqBands; //frequency band/group labels and ranges
        std::vector< std::string > fFreqGroups;                              //limit output to matching frequency groups
        bool fSelectByBandwidth;
        double fOnlyBandwidth; //limit output to only channels of this bandwidth
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXScanProcessor */
