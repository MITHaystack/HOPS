#include "MHO_ScanDataStore.hh"

namespace hops
{

MHO_ScanDataStore::MHO_ScanDataStore(){};
MHO_ScanDataStore::~MHO_ScanDataStore(){};

void
MHO_ScanDataStore::Initialize()
{
    //read the directory file list
    fAllFiles.clear();
    fCorFiles.clear();
    fStaFiles.clear();
    fJSONFiles.clear();
    fBaselineFileMap.clear();
    fBaselineCodes.clear();
    fStationFileMap.clear();
    fStationCodes.clear();
    fRootFileName = "";

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
    if(fRootFileName == ""){return false;}
    if(fBaselineCodes.size() == 0){return false;}
    if(fStationCodes.size() == 0){return false;}
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


    // //read the entire file into memory (obviously we will want to optimize this in the future)
    // MHO_ContainerStore conStore;
    // MHO_ContainerFileInterface conInter;
    // conInter.SetFilename(corel_file);
    // conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file
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


// bool
// MHO_ScanDataStore::LoadBaseline(std::string baseline)
// {
//     //
//     // if(fBaselineContainers[baseline])
//     //
//     // //read the entire file into memory (obviously we will want to optimize this in the future)
//     // MHO_ContainerStore conStore;
//     // MHO_ContainerFileInterface conInter;
//     // conInter.SetFilename(corel_file);
//     // conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file
//
// }

// bool
// MHO_ScanDataStore::LoadStation(std::string station);
//
// void
// MHO_ScanDataStore::ClearBaselineFiles();
//
// void
// MHO_ScanDataStore::ClearStationFiles();
//


}
