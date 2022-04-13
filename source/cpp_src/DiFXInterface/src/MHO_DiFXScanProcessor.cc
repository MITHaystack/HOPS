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
    //so we could probably skip the ovex step...but we do need to make sure we cover the 
    //same set of information, so filling the ovex structures may be the simplest thing to do for now
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

    //clear out station coords
    for(auto it = fStationCode2Coords.begin(); it != fStationCode2Coords.end(); it++)
    {
        station_coord_type2* ptr = it->second;
        delete ptr;
    }
    fStationCode2Coords.clear();

}


void 
MHO_DiFXScanProcessor::LoadInputFile()
{
    //convert the input to json 
    MHO_DiFXInputProcessor input_proc;
    input_proc.LoadDiFXInputFile(fFileSet->fInputFile);
    input_proc.ConvertToJSON(fInput);

    msg_debug("difx_interface", "difx .input file: " << fFileSet->fInputFile <<" converted to json." << eom);
}


void 
MHO_DiFXScanProcessor::ExtractStationCoords()
{

    //populate fStationCode2Coords with each station present in fInput 
    //(e.g. the station name/codes, coordinates, and delay spline info, etc. for each station)
    //first thing we have to do is figure out the data dimensions
    //the items we what to store here are equivalent to what is stored in the following type_3XXs
    //(1) delay spline polynomial coeff (type_301)
    //(2) phase spline polynomial coeff (type_302) --- This doesn't appear to get used anywhere, so leave off for now
    //(3) parallatic angle spline coeff (type_303)
    //(4) uvw-coords spline coeff (type_303)
    //(5) phase-cal data (type_309)

    //Note: with the exception of the phase-spline polynomial, all of these other quantities 
    //do not depend on the channel/frequency.



    std::size_t nAntenna = fInput["scan"][0]["nAntenna"];
    std::size_t phase_center = 0; //TODO FIXME?, currently only one phase-center supported

    for(std::size_t n=0; n<nAntenna; n++)
    {
        //first get antenna name for an ID (later we need to map this to the 2 char code)
        std::string antenna_name = fInput["antenna"][n]["name"];
        station_coord_type2* st_coord = new station_coord_type2();
        fStationCode2Coords[antenna_name] = st_coord;

        json antenna_poly = fInput["scan"][0]["DifxPolyModel"][n][phase_center];

        std::size_t n_coord = NCOORD-1; //currently 1 less because we are not converting the phase-spline
        std::size_t n_poly = antenna_poly.size(); //aka nsplines in d2m4
        std::size_t n_order = antenna_poly["order"];
        
        st_coord->Resize(n_coord, n_poly, n_order);

        auto coord_ax = std::get<COORD_AXIS>(*st_coord);
        coord_ax(0) = "delay";
        coord_ax(1) = "az";
        coord_ax(2) = "el";
        coord_ax(3) = "parangle";
        coord_ax(4) = "u";
        coord_ax(5) = "v";
        coord_ax(6) = "w";

        for(std::size_t i=0; i<n_poly; i++)
        {
            json poly_interval = antenna_poly[i];
            for(std::size_t p=0; p<=n_order; p++)
            {
                st_coord->at(0,i,p) = poly_interval["delay"][p];
                st_coord->at(1,i,p) = poly_interval["az"][p];
                st_coord->at(2,i,p) = poly_interval["elcorr"][p];
                st_coord->at(3,i,p) = poly_interval["parangle"][p];
                st_coord->at(4,i,p) = poly_interval["u"][p];
                st_coord->at(5,i,p) = poly_interval["v"][p];
                st_coord->at(6,i,p) = poly_interval["w"][p];
            }
        }
    }
}


}//end of namespace
