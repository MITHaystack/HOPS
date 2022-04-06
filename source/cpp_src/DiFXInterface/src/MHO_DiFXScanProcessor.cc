#include "MHO_DiFXScanProcessor.hh"

namespace hops 
{

MHO_DiFXScanProcessor::MHO_DiFXScanProcessor()
{};

MHO_DiFXScanProcessor::~MHO_DiFXScanProcessor()
{};


void 
MHO_DiFXScanProcessor::ProcessScan(MHO_DiFXScanFileSet& fileSet)
{
    fFileSet = &fileSet;
    LoadInputFile(); //read .input file and build freq table
    ConvertRootFileObject(); //create the equivalent to the Mk4 'ovex' root file
    ConvertVisibilityFileObjects(); //convert visibilities and data weights 
    ConvertStationFileObjects(); //convert the station splines, and pcal data 
    CleanUp(); //delete workspace and prep for next scan
}


void 
MHO_DiFXScanProcessor::ConvertRootFileObject()
{
    //TODO FILL ME IN ...need to populate the 'ovex' structure that we typically use 
    //then convert that to the json representation (as we do in the Mk4Inteface)
    //Is this strictly necessary? We've already converted the DiFX input information into json 
    //so we could probably skip the ovex step...but we do need to make sure we cover the same set of information
}

void 
MHO_DiFXScanProcessor::ConvertVisibilityFileObjects()
{
    //load the visibilities
    if(fFileSet->fVisibilityFileList.size() > 1)
    {
        msg_warn("difx_interface", "more than one Swinburne file present, will only process the first one: " <<fFileSet->fVisibilityFileList[0]<< "." << eom);
    }

    //expectation here is that there is only a single file containing visibility
    //records from every baseline in the scan 
    MHO_DiFXVisibilityProcessor visProcessor;
    visProcessor.SetFilename(fFileSet->fVisibilityFileList[0]);
    visProcessor.ReadDIFXFile(fAllBaselineVisibilities);

    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        it->second.SetDiFXInputData(&fInput);
        it->second.ConstructVisibilityFileObjects();
        it->second.WriteVisibilityObjects(fFileSet->fOutputBaseDirectory);
    }

}

void 
MHO_DiFXScanProcessor::ConvertStationFileObjects()
{
    //first process pcal files (if they exist)
    for(auto it = fFileSet->fPCALFileList.begin(); it != fFileSet->fPCALFileList.end(); it++)
    {
        std::cout<<"pcal name = "<<*it<<std::endl;
        fPCalProcessor.SetFilename(*it);
        double ap_length = fInput["config"][0]["tInt"]; //config is a list element, grab the first
        fPCalProcessor.SetAccumulationPeriod(ap_length);
        fPCalProcessor.ReadPCalFile();
        fPCalProcessor.Organize();

        std::string station_code = fPCalProcessor.GetStationCode();
        multitone_pcal_type* pcal = fPCalProcessor.GetPCalData()->Clone();
        fStationCode2PCal[station_code] = pcal;
    }

    //DEBUG, lets write out the PCAL stuff 
    for(auto it = fStationCode2PCal.begin(); it != fStationCode2PCal.end(); it++)
    {
        std::string station_code = it->first;
        //construct output file name (eventually figure out how to construct the baseline name)
        std::string root_code = "dummy"; //TODO replace with actual 'root' code
        std::string output_file = fFileSet->fOutputBaseDirectory + "/" + station_code + "." + root_code + ".pcal";

        MHO_BinaryFileInterface inter;
        bool status = inter.OpenToWrite(output_file);
        MHO_ObjectTags tags;

        if(status)
        {
            uint32_t label = 0xFFFFFFFF; //someday make this mean something
            tags.AddObjectUUID(it->second->GetObjectUUID());
            inter.Write(tags, "tags", label);
            inter.Write( *(it->second), "pcal", label);
            inter.Close();
        }
        else
        {
            msg_error("file", "error opening pcal output file: " << output_file << eom);
        }
        inter.Close();
    }

    //next we need to extract the station specific data from fInput
    //(e.g. the coordinates, and delay spline info, etc.)

}


void 
MHO_DiFXScanProcessor::CleanUp()
{
    //clear up and reset for next scan
    //now iterate through the pcal map and delete the objects we cloned 
    for(auto it = fStationCode2PCal.begin(); it != fStationCode2PCal.end(); it++)
    {
        multitone_pcal_type* ptr = it->second;
        delete ptr;
    }
    fStationCode2PCal.clear();
}


void 
MHO_DiFXScanProcessor::LoadInputFile()
{
    //convert the input to json 
    MHO_DiFXInputProcessor input_proc;
    input_proc.LoadDiFXInputFile(fFileSet->fInputFile);
    input_proc.ConvertToJSON(fInput);

    msg_debug("difx_interface", "difx .input file: " << fFileSet->fInputFile <<" converted to json." << eom);
    //debug -- remove me TODO FIXME
    //std::cout<< fInput.dump(2)<<std::endl;
}





}
