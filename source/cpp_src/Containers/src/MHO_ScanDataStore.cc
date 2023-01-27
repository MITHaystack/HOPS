#include "MHO_ScanDataStore.hh"

namespace hops 
{

MHO_ScanDataStore::MHO_ScanDataStore(){};
MHO_ScanDataStore::~MHO_ScanDataStore(){};

MHO_ScanDataStore::Initialize()
{
    //read the directory file list
    fAllFiles.clear();
    fCorFiles.clear();
    fStaFiles.clear();
    fJSONFiles.clear();
    fBaselineFileMap.clear(); 
    fStationFileMap.clear();

    fDirInterface.SetCurrentDirectory(fDirectory);
    fDirInterface.ReadCurrentDirectory();

    fDirInterface.GetFileList(fAllFiles);
    fDirInterface.GetFilesMatchingExtention(fCorFiles, "cor");
    fDirInterface.GetFilesMatchingExtention(fStaFiles, "sta");
    fDirInterface.GetFilesMatchingExtention(fJSONFiles, "json");

    DetermineRootFile();
    fNBaselines = MapBaselines();
    fStations = MapStations();

    //determine the root file (.json)

    //check that there is only one json file
    std::string root_file = "";
    if(fJSONFiles.size() != 1)
    {
        msg_fatal("main", "There are "<<fJSONFiles.size()<<" root files." << eom);
        std::exit(1);
    }
    else
    {
        root_file = fJSONFiles[0];
    }

    // std::ifstream ifs(root_file);
    // mho_json vexInfo = json::parse(ifs);

    //map 2-character baseline codes to cor files 


    //map 1-character station codes to sta files 




}


void 
MHO_ScanDataStore::DetermineRootFile()
{

}

void 
MHO_ScanDataStore::MapBaselines()
{

}

void 
MHO_ScanDataStore::MapStations()
{


}



bool
MHO_ScanDataStore::LoadBaseline(std::string baseline)
{
    //locate the corel file that contains the baseline of interest
    std::string corel_file = "";
    bool found_baseline = false;
    std::string basename, bl_code;
    for(auto it = fCorFiles.begin(); it != fCorFiles.end(); it++)
    {
        std::size_t index = it->find(baseline);
        if(index != std::string::npos)
        {
            //now verify this is actually the baseline string (e.g. HV in HV.2A5SV6.cor)
            //and not accidentaly a matching 2-char section of the root code 
            basename = fDirInterface.GetBasename(*it);
            std::size_t index = basename.find_first_of(".");
            if(index != std::string::npos)
            {
                bl_code = basename.substr(index + 1);
            }

            if(bl_code == baseline)
            {
                corel_file = *it;
                found_baseline = true;
                break;
            }
        }
    }

    if(!found_baseline)
    {
        msg_fatal("container", "could not find a file for baseline: "<< baseline <<" in directory: "<< fDirectory << eom);
        return false;
    }


    if(fBaselineContainers[baseline])

    //read the entire file into memory (obviously we will want to optimize this in the future)
    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(corel_file);
    conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file

}

bool 
MHO_ScanDataStore::LoadStation(std::string station);

void 
MHO_ScanDataStore::ClearBaselineFiles();

void 
MHO_ScanDataStore::ClearStationFiles();



}