#ifndef MHO_ScanDataStore_HH__
#define MHO_ScanDataStore_HH__

//global messaging util
#include "MHO_Message.hh"

//handles reading directories, listing files etc.
#include "MHO_DirectoryInterface.hh"

//needed to read hops files and extract objects
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerStore.hh"

namespace hops
{

/*!
 *@file MHO_ScanDataStore.hh
 *@class MHO_ScanDataStore
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Jan 27 16:41:52 2023 -0500
 *@brief Class to catalog and organize data files that are associated with a single scan, and handle retrieval for specific baselilnes
 */

class MHO_ScanDataStore
{
    public:
        MHO_ScanDataStore();
        virtual ~MHO_ScanDataStore();

        /**
         * @brief Setter for (scan) directory
         *
         * @param dir New directory path as std::string
         */
        void SetDirectory(std::string dir) { fDirectory = dir; };

        /**
         * @brief Initializes data store by clearing, reading directory files, mapping stations and fringes, and determining root file.
         *
         * @return True if initialization is successful, false otherwise.
         */
        bool Initialize(); //load the directory

        /**
         * @brief Checks if root file and baseline/station files exist for valid data processing.
         *
         * @return True if all required files are present, false otherwise.
         */
        bool IsValid(); //scan dir contains root file, and data

        /**
         * @brief Checks if a baseline code is present in the internal list.
         *
         * @param bl The baseline code to search for.
         * @return True if the baseline code is found, false otherwise.
         */
        bool IsBaselinePresent(std::string bl) const; //check if a particular baseline is present in this scan

        /**
         * @brief Checks if a station is present in the list of station codes.
         *
         * @param st The station name to search for.
         * @return True if the station is found, false otherwise.
         */

        bool IsStationPresent(std::string st) const; //check if a particular station is present

        /**
         * @brief Checks if a fringe file is present for a given using its basename.
         *
         * @param basename Input basename string to search for in fringe codes
         * @return Boolean indicating whether the fringe with the given basename is present
         */
        bool IsFringePresent(std::string basename) const; //check if a fringe file is present

        /**
         * @brief Getter for number of baselines
         *
         * @return Size of fBaselineCodes vector
         */
        std::size_t GetNBaselines() { return fBaselineCodes.size(); };

        /**
         * @brief Getter for number of stations
         *
         * @return Number of stations as std::size_t.
         */
        std::size_t GetNStations() { return fStationCodes.size(); };

        /**
         * @brief Getter for number of fringes
         *
         * @return Size of fringe codes vector as std::size_t
         */
        std::size_t GetNFringes() { return fFringeCodes.size(); };

        /**
         * @brief Getter for baselines present
         *
         * @return std::string containing baseline codes
         */
        std::vector< std::string > GetBaselinesPresent() const { return fBaselineCodes; }

        /**
         * @brief Getter for stations present
         *
         * @return std::string containing station codes
         */
        std::vector< std::string > GetStationsPresent() const { return fStationCodes; }

        /**
         * @brief Getter for fringes present
         *
         * @return std::string containing the fringe codes.
         */
        std::vector< std::string > GetFringesPresent() const { return fFringeCodes; }

        /**
         * @brief Getter for root file data (as json)
         *
         * @return JSON object containing root file data
         */
        mho_json GetRootFileData() const;

        /**
         * @brief Getter for root file basename
         *
         * @return std::string containing the basename of the root file
         */
        std::string GetRootFileBasename() { return fDirInterface.GetBasename(fRootFileName); }

        //true if loaded, false if unsuccessful
        /**
         * @brief Loads baseline data into a store from a file mapped to the given baseline.
         *
         * @param baseline Input baseline string used to lookup the corresponding file
         * @param store (MHO_ContainerStore*)
         * @return True if loaded successfully, false otherwise
         */
        bool LoadBaseline(std::string baseline, MHO_ContainerStore* store);

        /**
         * @brief Getter for baseline filename
         *
         * @param baseline Input baseline string used to lookup its corresponding filename.
         * @return The filename associated with the input baseline or an empty string if not found.
         */
        std::string GetBaselineFilename(std::string baseline) const;

        //true if loaded, false if unsuccessful
        /**
         * @brief Loads station data into a container store if found in the map.
         *
         * @param station Station name to load data for.
         * @param store Container store to populate with loaded data.
         * @return True if loaded successfully, false otherwise.
         */
        bool LoadStation(std::string station, MHO_ContainerStore* store);

        /**
         * @brief Getter for station filename
         *
         * @param station The station name to search for in the internal map.
         * @return The corresponding filename if found, otherwise an empty string.
         */
        std::string GetStationFilename(std::string station) const;

        //true if loaded, false if unsuccessful
        bool LoadFringe(std::string fringe_basename, MHO_ContainerStore* store);
        std::string GetFringeFilename(std::string fringe_basename) const;

        //deletes all loaded containers and resets the state for another scan.
        void Clear();

    private:
        void DetermineRootFile();
        void MapBaselines();
        void MapStations();
        void MapFringes();

        std::string fDirectory;

        //directory file lists
        MHO_DirectoryInterface fDirInterface;
        std::vector< std::string > fAllFiles;
        std::vector< std::string > fCorFiles;
        std::vector< std::string > fStaFiles;
        std::vector< std::string > fJSONFiles;
        std::vector< std::string > fFringeFiles;

        std::string fRootFileName;
        mho_json fRootFileData;

        //map baseline 2-char code to filename (cor file)
        std::vector< std::string > fBaselineCodes;
        std::map< std::string, std::string > fBaselineFileMap;
        std::map< std::string, MHO_ContainerStore* > fActiveBaselineContainers;

        //map station 1-char (mk4 id) code to filename
        std::vector< std::string > fStationCodes;
        std::map< std::string, std::string > fStationFileMap;
        std::map< std::string, MHO_ContainerStore* > fActiveStationContainers;

        //map fringe file basename to filename
        std::vector< std::string > fFringeCodes;
        std::map< std::string, std::string > fFringeFileMap;
        std::map< std::string, MHO_ContainerStore* > fActiveFringeContainers;
};

} // namespace hops

#endif /*! end of include guard: MHO_ScanDataStore_HH__ */
