#ifndef MHO_ScanDataStore_HH__
#define MHO_ScanDataStore_HH__

/*
*File: MHO_ScanDataStore.hh
*Class: MHO_ScanDataStore
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 27-01-2023 3:15
*Description: Class to catalog and organize data files
 associated with a single scan, and handle retrieval for specific baselilnes
*/

//global messaging util
#include "MHO_Message.hh"

//handles reading directories, listing files etc.
#include "MHO_DirectoryInterface.hh"

//needed to read hops files and extract objects
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"


namespace hops
{

class MHO_ScanDataStore
{
    public:
        MHO_ScanDataStore();
        virtual ~MHO_ScanDataStore();

        void SetDirectory(std::string dir){fDirectory = dir;};
        void Initialize(); //load the directory
        bool IsValid(); //scan dir contains root file, and data

        std::size_t GetNBaselines(){return fBaselineCodes.size();};
        std::size_t GetNStations(){return fStationCodes.size();};

        // //return the json object containing the root file information
        // mho_json GetRootFileData();
        //
        // bool LoadBaseline(std::string baseline);
        // bool LoadStation(std::string station);
        //
        // void ClearBaselineFiles();
        // void ClearStationFiles();

    private:

        void DetermineRootFile();
        void MapBaselines();
        void MapStations();


        std::string fDirectory;

        //directory file lists
        MHO_DirectoryInterface fDirInterface;
        std::vector< std::string > fAllFiles;
        std::vector< std::string > fCorFiles;
        std::vector< std::string > fStaFiles;
        std::vector< std::string > fJSONFiles;

        std::string fRootFileName;
        mho_json fRootFileData;

        //map baseline 2-char code to filename (cor file)
        std::vector< std::string > fBaselineCodes;
        std::map< std::string, std::string > fBaselineFileMap;

        //map station 1-char (mk4 id) code to filename
        std::vector< std::string > fStationCodes;
        std::map< std::string, std::string > fStationFileMap;

        // //one active container for each '.cor' file which is open, mapped by baseline 2-char code
        // std::map<std::string, MHO_ContainerStore* > fBaselineContainers;
        //
        // //one active container for each '.sta' file which is open, mapped by station mk4 id
        // std::map< std::string, MHO_ContainerStore* > fStationContainers;

};


}//end namespace


#endif /* end of include guard: MHO_ScanDataStore_HH__ */
