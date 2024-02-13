#include "MHO_ScanDataStore.hh"

namespace hops
{

MHO_ScanDataStore::MHO_ScanDataStore(){};

MHO_ScanDataStore::~MHO_ScanDataStore()
{
    Clear();
};

bool
MHO_ScanDataStore::Initialize()
{
    Clear();
    //read the directory file list
    fDirInterface.SetCurrentDirectory(fDirectory);
    bool ok = fDirInterface.ReadCurrentDirectory();

    if(ok)
    {
        fDirInterface.GetFileList(fAllFiles);
        fDirInterface.GetFilesMatchingExtention(fCorFiles, "cor");
        fDirInterface.GetFilesMatchingExtention(fStaFiles, "sta");
        fDirInterface.GetFilesMatchingExtention(fJSONFiles, "json");
        fDirInterface.GetFilesMatchingExtention(fFringeFiles, "frng");

        //determine the root file (.json)
        DetermineRootFile();

        //map the stations and baselines to 1-char and 2-char (mk4 id) look-up codes
        MapBaselines();
        MapStations();

        //map fringes to basename strings (could have many more than one fringe per-baseline)
        MapFringes();

        return true;
    }
    return false;
}

bool
MHO_ScanDataStore::IsValid()
{
    if(fRootFileName == ""){msg_warn("containers", "no root file found." << eom); return false;}
    if(fBaselineCodes.size() == 0){msg_warn("containers", "no baseline files found." << eom);return false;}
    if(fStationCodes.size() == 0){msg_warn("containers", "no station files found." << eom);return false;}
    return true;
}

bool 
MHO_ScanDataStore::IsFringePresent(std::string basename) const
{
    for(auto it = fFringeCodes.begin(); it != fFringeCodes.end(); it++)
    {
        if(basename == *it){return true;}
    }
    return false;
}


bool 
MHO_ScanDataStore::IsBaselinePresent(std::string bl) const
{
    for(auto it = fBaselineCodes.begin(); it != fBaselineCodes.end(); it++)
    {
        if( bl == *it){return true;}
    }
    return false;
}

bool 
MHO_ScanDataStore::IsStationPresent(std::string st) const
{
    for(auto it = fStationCodes.begin(); it != fStationCodes.end(); it++)
    {
        if( st == *it){return true;}
    }
    return false;
}

void
MHO_ScanDataStore::DetermineRootFile()
{
    //the root file ought to have the extension ".root.json"
    //there should only be one per scan directory
    fRootFileName = "";
    std::size_t n_candidate_files = 0;

    for(auto it = fJSONFiles.begin(); it != fJSONFiles.end(); it++)
    {
        std::size_t ext_pos = it->find(".root.json"); //must have this extension
        if(ext_pos != std::string::npos)
        {
            n_candidate_files++;
            fRootFileName = *it;
        }
    }

    if(n_candidate_files == 0 || fRootFileName == "")
    {
        msg_fatal("containers", "no root file found in directory: " << fDirectory << eom);
        std::exit(1);
    }

    if(n_candidate_files > 1)
    {
        fRootFileName = fJSONFiles[0];
        msg_error("containers", "duplicate root files found in directory, using last one found: "<< fRootFileName << eom );
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
            bl_code = basename.substr(0,index);
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
            sta_code = basename.substr(0,index);
            if(sta_code.size() == 1)
            {
                fStationFileMap[sta_code] = *it;
                fStationCodes.push_back(sta_code);
            }
        }
    }
}


void
MHO_ScanDataStore::MapFringes()
{
    //map all fringes to filename basename
    std::string basename, bl_code;
    for(auto it = fFringeFiles.begin(); it != fFringeFiles.end(); it++)
    {
        basename = fDirInterface.GetBasename(*it);
        fFringeFileMap[basename] = *it;
        fFringeCodes.push_back(basename);
    }
}


mho_json
MHO_ScanDataStore::GetRootFileData()
{
    mho_json vex_info;
    if(fRootFileName != "" )
    {
        std::ifstream ifs(fRootFileName);
        vex_info = mho_json::parse(ifs);
    }
    else
    {
        msg_warn("containers", "no root file found, returning empty data." << eom);
    }
    return vex_info;
}

bool
MHO_ScanDataStore::LoadBaseline(std::string baseline, MHO_ContainerStore* store)
{
    auto it = fBaselineFileMap.find(baseline);
    if(it != fBaselineFileMap.end() )
    {
        //read the entire file into memory (obviously we will want to optimize this in the future)
        MHO_ContainerFileInterface conInter;
        conInter.SetFilename(it->second);
        msg_debug("containers", "loading baseline data from: "<< it->second << eom );
        conInter.PopulateStoreFromFile(*store); //reads in ALL the objects in the file
        return true;
    }
    else 
    {
        return false;
        msg_warn("containers", "could not find data for baseline: "<< baseline <<"." << eom);
    }
}

std::string
MHO_ScanDataStore::GetBaselineFilename(std::string baseline) const
{
    auto it = fBaselineFileMap.find(baseline);
    if(it != fBaselineFileMap.end() ){return it->second;}
    else{return std::string("");}
}

bool
MHO_ScanDataStore::LoadStation(std::string station, MHO_ContainerStore* store)
{
    auto it = fStationFileMap.find(station);
    if(it != fStationFileMap.end() )
    {
        //read the entire file into memory (obviously we will want to optimize this in the future)
        MHO_ContainerFileInterface conInter;
        conInter.SetFilename(it->second);
        msg_debug("containers", "loading station data from: "<< it->second << eom );
        conInter.PopulateStoreFromFile(*store); //reads in ALL the objects in the file
        return true;
    }
    else 
    {
        msg_warn("containers", "could not find data for station: "<< station <<"." << eom);
        return false;
    }
}

std::string
MHO_ScanDataStore::GetStationFilename(std::string station) const
{
    auto it = fStationFileMap.find(station);
    if(it != fStationFileMap.end() ){return it->second;}
    else{return std::string("");}
}


//true if loaded, false if unsuccessful
bool 
MHO_ScanDataStore::LoadFringe(std::string fringe_basename, MHO_ContainerStore* store)
{
    auto it = fFringeFileMap.find(fringe_basename);
    if(it != fFringeFileMap.end() )
    {
        //read the entire file into memory (obviously we will want to optimize this in the future)
        MHO_ContainerFileInterface conInter;
        conInter.SetFilename(it->second);
        msg_debug("containers", "loading fringe data from: "<< it->second << eom );
        conInter.PopulateStoreFromFile(*store); //reads in ALL the objects in the file
        return true;
    }
    else 
    {
        msg_warn("containers", "could not find data for fringe: "<< fringe_basename <<"." << eom);
        return false;
    }
}

std::string 
MHO_ScanDataStore::GetFringeFilename(std::string fringe_basename) const
{
    auto it = fFringeFileMap.find(fringe_basename);
    if(it != fFringeFileMap.end() ){return it->second;}
    else{return std::string("");}
}



void
MHO_ScanDataStore::Clear()
{
    fStationCodes.clear();
    fStationFileMap.clear();
    fBaselineCodes.clear();
    fBaselineFileMap.clear();
    fFringeCodes.clear();
    fFringeFileMap.clear();

    fAllFiles.clear();
    fCorFiles.clear();
    fStaFiles.clear();
    fJSONFiles.clear();
    fFringeFiles.clear();
    fRootFileName = "";
}




}
