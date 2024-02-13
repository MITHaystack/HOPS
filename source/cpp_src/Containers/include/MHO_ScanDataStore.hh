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
        bool Initialize(); //load the directory
        bool IsValid(); //scan dir contains root file, and data
        bool IsBaselinePresent(std::string bl) const; //check if a particular baseline is present in this scan
        bool IsStationPresent(std::string st) const; //check if a particular station is present
        bool IsFringePresent(std::string basename) const;//check if a fringe file is present

        std::size_t GetNBaselines(){return fBaselineCodes.size();};
        std::size_t GetNStations(){return fStationCodes.size();};

        std::vector< std::string > GetBaselinesPresent() const {return fBaselineCodes;} 
        std::vector< std::string > GetStationsPresent() const { return fStationCodes;}
        std::vector< std::string > GetFringesPresent() const {return fFringeCodes;}

        //retieve file data (root, baseline, station)
        mho_json GetRootFileData();
        std::string GetRootFileBasename(){return fDirInterface.GetBasename(fRootFileName);}

        //true if loaded, false if unsuccessful
        bool LoadBaseline(std::string baseline, MHO_ContainerStore* store);
        std::string GetBaselineFilename(std::string baseline) const;

        //true if loaded, false if unsuccessful
        bool LoadStation(std::string station, MHO_ContainerStore* store);
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


}//end namespace


#endif /* end of include guard: MHO_ScanDataStore_HH__ */
