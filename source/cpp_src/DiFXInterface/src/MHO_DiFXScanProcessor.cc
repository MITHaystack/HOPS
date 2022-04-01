#include "MHO_DiFXScanProcessor.hh"

namespace hops 
{

MHO_DiFXScanProcessor::MHO_DiFXScanProcessor()
{};

MHO_DiFXScanProcessor::~MHO_DiFXScanProcessor()
{
    if(fDInput){deleteDifxInput(fDInput);}
};


void 
MHO_DiFXScanProcessor::ProcessScan(MHO_DiFXScanFileSet& fileSet)
{
    fFileSet = &fileSet;

    LoadInputFile(fileSet.fInputFile); //read .input file and build freq table

    //load the visibilities
    MHO_DiFXVisibilityProcessor visProcessor;
    visProcessor.SetFilename(fileSet.fVisibilityFileList[0]);
    visProcessor.ReadDIFXFile(fAllBaselineVisibilities);

    ConstructRootFileObject();

    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        it->second.SetDiFXInputData(&fInput);
        it->second.ConstructVisibilityFileObjects();
        it->second.WriteVisibilityObjects(fFileSet->fOutputBaseDirectory);
    }

    //process pcal files (if they exist)
    for(auto it = fileSet.fPCALFileList.begin(); it != fileSet.fPCALFileList.end(); it++)
    {
        fPCalProcessor.SetFilename(*it);
        fPCalProcessor.ReadPCalFile();
        fPCalProcessor.Organize();
    }

    ConstructStationFileObjects();

    //clear up and reset for next scan
    deleteDifxInput(fDInput);
    fDInput = nullptr;
}


// void 
// MHO_DiFXScanProcessor::ReadPCALFile(std::string filename)
// {
//     //TODO 
// }
// 
// void 
// MHO_DiFXScanProcessor::ReadIMFile(std::string filename)
// {
//     //TODO
// }


void 
MHO_DiFXScanProcessor::LoadInputFile(std::string filename)
{
    //TODO FIXME - Why does this sometimes fail for DiFX versions <2.6 
    //when the .threads file is missing??
    fDInput = loadDifxInput(filename.c_str());

    //convert the input to json 
    MHO_DiFXInputProcessor input_proc;
    input_proc.LoadDiFXInputFile(filename);
    input_proc.ConvertToJSON(fInput);

    //debug -- remove me
    std::cout<< fInput.dump(2)<<std::endl;
}


void 
MHO_DiFXScanProcessor::ConstructRootFileObject()
{

};

void 
MHO_DiFXScanProcessor::ConstructStationFileObjects()
{

};



}
