#ifndef MHO_DiFXInterface_HH__
#define MHO_DiFXInterface_HH__

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "MHO_DirectoryInterface.hh"
#include "MHO_LegacyRootCodeGenerator.hh"
#include "MHO_Message.hh"
#include "MHO_StationCodeMap.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_DiFXScanFileSet.hh"
#include "MHO_DiFXScanProcessor.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXInterface.hh
 *@class  MHO_DiFXInterface
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri Feb 4 13:56:11 2022 -0500
 *@brief
 */

/**
 * @brief Class MHO_DiFXInterface
 */
class MHO_DiFXInterface
{
    public:
        MHO_DiFXInterface();
        virtual ~MHO_DiFXInterface();

        /**
         * @brief Setter for (data) input directory
         * 
         * @param dir Input directory path as std::string.
         */
        void SetInputDirectory(std::string dir);
        /**
         * @brief Setter for output directory
         * 
         * @param dir Input directory path as a string.
         */
        void SetOutputDirectory(std::string dir);

        /**
         * @brief Setter for station codes map
         * 
         * @param code_map Input pointer to an MHO_StationCodeMap containing station codes
         */
        void SetStationCodes(MHO_StationCodeMap* code_map);

        /**
         * @brief Setter for experiment number
         * 
         * @param num New experiment number to set
         */
        void SetExperimentNumber(int num) { fExperNum = num; }

        /**
         * @brief Setter for normalize false - Do NOT apply mk4 style visibility normalization
         */
        void SetNormalizeFalse() { fNormalize = false; }

        /**
         * @brief Setter for normalize true - apply mk4 style visibility normalization
         */
        void SetNormalizeTrue() { fNormalize = true; }
        
        /**
         * @brief Setter for attach difx .input true
         */
        void SetAttachDiFXInputTrue() {fAttachDiFXInput = true; }
        
        /**
         * @brief Setter for attach difx .input false
         */
        void SetAttachDiFXInputFalse() {fAttachDiFXInput = false; }

        /**
         * @brief Setter for preserve difx scan names true
         */
        void SetPreserveDiFXScanNamesTrue() { fPreserveDiFXScanNames = true; }

        /**
         * @brief Setter for preserve difx scan names false
         */
        void SetPreserveDiFXScanNamesFalse() { fPreserveDiFXScanNames = false; };

        void SetExportAsMark4True(){fExportAsMark4 = true;}
        void SetExportAsMark4False(){fExportAsMark4 = false;}

        /**
         * @brief Setter for frequency bands (label, frequency limits)
         * 
         * @param fbands Vector of tuples containing band name, start freq, and end freq
         */
        void SetFrequencyBands(std::vector< std::tuple< std::string, double, double > > fbands) { fFreqBands = fbands; }

        /**
         * @brief Setter for freq groups - only consider data from these frequency groups
         * 
         * @param fgroups Vector of strings representing frequency groups
         */
        void SetFreqGroups(std::vector< std::string > fgroups) { fFreqGroups = fgroups; }

        /**
         * @brief Setter for only bandwidth - only consider channels with matching bandwidth
         * 
         * @param bw The allowed channel bandwidth value.
         */
        void SetOnlyBandwidth(double bw)
        {
            fOnlyBandwidth = bw;
            fSelectByBandwidth = true;
        }

        /**
         * @brief Initializes MHO_DiFXInterface based on input directory type (single scan or whole experiment).
         */
        void Initialize();   //read the directory and construct the scan file-lists
        
        /**
         * @brief Generates root codes and processes scans using MHO_LegacyRootCodeGenerator and MHO_ScanProcessor.
         */
        void ProcessScans(); //convert the scans

    private:

        /**
         * @brief Initializes MHO_DiFXInterface from experiment directory files (batch mode)
         * 
         * @param input_dir Input directory path containing experiment data
         */
        void InitializeFromExperimentDir(const std::string& input_dir); //for when we are processing a whole experiment in batch mode
        
        /**
         * @brief Initializes DiFX interface from scan directory and filters specific scans by input directory.
         * 
         * @param input_dir Input directory path to initialize from.
         */
        void InitializeFromScanDir(const std::string& input_dir); //for when we are processing a single scan

        /**
         * @brief Checks if input directory contains a single scan file (.difx).
         * 
         * @param input_dir Input directory path to check for single scan file
         * @return True if single scan file found, false otherwise
         */
        bool IsSingleScan(const std::string& input_dir) const;

        std::string fInputDirectory;
        std::string fOutputDirectory;
        MHO_DirectoryInterface fDirInterface;
        std::string fVexFile;
        std::string fV2DFile;
        std::vector< MHO_DiFXScanFileSet > fScanFileSetList;
        bool fNormalize;
        bool fPreserveDiFXScanNames;
        bool fAttachDiFXInput;
        bool fExportAsMark4;

        std::vector< std::tuple< std::string, double, double > > fFreqBands; //frequency band/group labels and ranges
        std::vector< std::string > fFreqGroups;                              //limit output to matching frequency groups
        bool fSelectByBandwidth;
        double fOnlyBandwidth; //limit output to only channels of this bandwidth

        int fExperNum;
        MHO_DiFXScanProcessor fScanProcessor;
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXInterface */
