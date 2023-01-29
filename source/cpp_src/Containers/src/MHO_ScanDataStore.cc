#include "MHO_ScanDataStore.hh"

namespace hops
{

MHO_ScanDataStore::MHO_ScanDataStore(){};

MHO_ScanDataStore::~MHO_ScanDataStore()
{
    Clear();
};

void
MHO_ScanDataStore::Initialize()
{
    Clear();
    //read the directory file list
    fDirInterface.SetCurrentDirectory(fDirectory);
    fDirInterface.ReadCurrentDirectory();

    fDirInterface.GetFileList(fAllFiles);
    fDirInterface.GetFilesMatchingExtention(fCorFiles, "cor");
    fDirInterface.GetFilesMatchingExtention(fStaFiles, "sta");
    fDirInterface.GetFilesMatchingExtention(fJSONFiles, "json");

    //determine the root file (.json)
    DetermineRootFile();

    //map the stations and baselines to 1-char and 2-char (mk4 id) look-up codes
    MapBaselines();
    MapStations();
}

bool
MHO_ScanDataStore::IsValid()
{
    if(fRootFileName == ""){msg_warn("containers", "no root file found." << eom); return false;}
    if(fBaselineCodes.size() == 0){msg_warn("containers", "no baseline files found." << eom);return false;}
    if(fStationCodes.size() == 0){msg_warn("containers", "no station files found." << eom);return false;}
    return true;
}

void
MHO_ScanDataStore::DetermineRootFile()
{
    //heuristic for a root file is that it is a json file which contains two '.'
    //and the mid-section is the 6-character root code.
    //there should only be one file like this, but if there is more than one
    //we return the last one found and issue a warning.

    fRootFileName = "";
    std::size_t n_candidate_files = 0;
    for(auto it = fJSONFiles.begin(); it != fJSONFiles.end(); it++)
    {
        std::string root_code = "";
        if( std::count(it->begin(), it->end(), '.') == 2)
        {
            auto first = it->find(".");
            auto second = it->find(".", first + 1);
            root_code = it->substr(first + 1, second - first - 1);
            if(root_code.size() == 6)
            {
                n_candidate_files++;
                fRootFileName = *it;
            }
        }
    }

    if(n_candidate_files == 0)
    {
        msg_fatal("containers", "no root file found in directory: " << fDirectory << eom);
    }

    if(n_candidate_files > 1)
    {
        msg_warn("containers", "duplicate root files found in directory, using last one: "<< fRootFileName << eom );
    }

}

void
MHO_ScanDataStore::MapBaselines()
{
    //map all baseline files to 2-char codes
    std::string basename, bl_code;
    for(auto it = fCorFiles.begin(); it != fCorFiles.end(); it++)
    {
        basename = fDirInterface.GetBasename(*it);
        std::size_t index = basename.find_first_of(".");
        if(index != std::string::npos)
        {
            bl_code = basename.substr(index + 1);
            if(bl_code.size() == 2)
            {
                fBaselineFileMap[bl_code] = *it;
                fBaselineCodes.push_back(bl_code);
            }
        }
    }
}

void
MHO_ScanDataStore::MapStations()
{
    //map all station files to 1-char codes
    std::string basename, sta_code;
    for(auto it = fStaFiles.begin(); it != fStaFiles.end(); it++)
    {
        basename = fDirInterface.GetBasename(*it);
        std::size_t index = basename.find_first_of(".");
        if(index != std::string::npos)
        {
            sta_code = basename.substr(index + 1);
            if(sta_code.size() == 1)
            {
                fStationFileMap[sta_code] = *it;
                fStationCodes.push_back(sta_code);
            }
        }
    }
}


mho_json
MHO_ScanDataStore::GetRootFileData()
{
    mho_json vex_info;
    if(fRootFileName != "" )
    {
        std::ifstream ifs(fRootFileName);
        vex_info = json::parse(ifs);
    }
    else
    {
        msg_warn("containers", "no root file found, returning empty data." << eom);
    }
    return vex_info;
}

MHO_ContainerStore*
MHO_ScanDataStore::LoadBaseline(std::string baseline)
{
    auto it = fBaselineFileMap.find(baseline);
    if(it != fBaselineFileMap.end() )
    {

        //read the entire file into memory (obviously we will want to optimize this in the future)
        MHO_ContainerStore* conStore = new MHO_ContainerStore();
        MHO_ContainerFileInterface conInter;
        conInter.SetFilename(it->second);
        conInter.PopulateStoreFromFile(*conStore); //reads in ALL the objects in the file

        fActiveBaselineContainers[baseline] = conStore;
        return conStore;
    }
    msg_warn("containers", "attempted to load baseline: "<< baseline <<" which does not exist." << eom);
    return nullptr;
}

MHO_ContainerStore*
MHO_ScanDataStore::LoadStation(std::string station)
{
    auto it = fStationFileMap.find(station);
    if(it != fStationFileMap.end() )
    {

        //read the entire file into memory (obviously we will want to optimize this in the future)
        MHO_ContainerStore* conStore = new MHO_ContainerStore();
        MHO_ContainerFileInterface conInter;
        conInter.SetFilename(it->second);
        conInter.PopulateStoreFromFile(*conStore); //reads in ALL the objects in the file

        fActiveStationContainers[station] = conStore;
        return conStore;
    }
    msg_warn("containers", "attempted to load station: "<< station <<" which does not exist." << eom);
    return nullptr;
}


void
 MHO_ScanDataStore::Clear()
 {
     //delete open station containers
     for(auto it = fActiveStationContainers.begin(); it != fActiveStationContainers.end(); it++){ delete it->second;}
     fActiveStationContainers.clear();
     fStationCodes.clear();
     fStationFileMap.clear();

     //delete open baseline containers
     for(auto it = fActiveBaselineContainers.begin(); it != fActiveBaselineContainers.end(); it++){ delete it->second;}
     fActiveBaselineContainers.clear();
     fBaselineCodes.clear();
     fBaselineFileMap.clear();

     fAllFiles.clear();
     fCorFiles.clear();
     fStaFiles.clear();
     fJSONFiles.clear();
     fRootFileName = "";
 }




}
