#include "MHO_DiFXScanProcessor.hh"
#include "MHO_VexParser.hh"
#include "MHO_VexGenerator.hh"
#include "MHO_Clock.hh"

#include <math.h> 


namespace hops 
{

MHO_DiFXScanProcessor::MHO_DiFXScanProcessor()
{
    fRootCode = "unknown";
    fStationCodeMap = nullptr;
    fPreserveDiFXScanNames = false;
};

MHO_DiFXScanProcessor::~MHO_DiFXScanProcessor()
{};


void 
MHO_DiFXScanProcessor::SetStationCodes(MHO_StationCodeMap* code_map)
{
    fStationCodeMap = code_map;
}

void 
MHO_DiFXScanProcessor::ProcessScan(MHO_DiFXScanFileSet& fileSet)
{
    fFileSet = &fileSet;
    LoadInputFile(); //read .input file
    bool ok = CreateScanOutputDirectory();
    if(ok)
    {
        ConvertVisibilityFileObjects(); //convert visibilities and data weights 
        ConvertStationFileObjects(); //convert the station splines, and pcal data 
        CreateRootFileObject(fileSet.fVexFile); //create the equivalent to the Mk4 'ovex' root file
    }
    else 
    {
        msg_error("difx_interface", "could not locate or create scan output directory: "<< fOutputDirectory <<", skipping." << eom);
    }
    CleanUp(); //delete workspace and prep for next scan
}


bool
MHO_DiFXScanProcessor::CreateScanOutputDirectory()
{
    //currently we just use the DiFX scan name, but we should add the option 
    //to use the scan time e.g. (031-1020)

    MHO_DirectoryInterface dirInterface;

    std::string output_dir  = fFileSet->fOutputBaseDirectory + "/";
    if(fPreserveDiFXScanNames)
    {
        output_dir += fFileSet->fScanName;
    }
    else 
    {
        std::string scan_id = fInput["scan"][fFileSet->fIndex]["identifier"];
        output_dir += scan_id;
    }
    fOutputDirectory = dirInterface.GetDirectoryFullPath(output_dir);

    bool ok = dirInterface.DoesDirectoryExist(fOutputDirectory);
    if(!ok){ ok = dirInterface.CreateDirectory(fOutputDirectory);}
    return ok;
}

void 
MHO_DiFXScanProcessor::CreateRootFileObject(std::string vexfile)
{
    MHO_VexParser vparser;
    vparser.SetVexFile(vexfile);
    mho_json vex_root = vparser.ParseVex();

    //now convert to 'ovex' (with subset of information)
    vex_root[ MHO_VexDefinitions::VexRevisionFlag() ] = "ovex";
    std::string scan_id = fInput["scan"][fFileSet->fIndex]["identifier"];
    std::vector< std::string > source_ids;

    //first rip out all scans but the one we are processing
    mho_json sched;
    mho_json sched_copy = vex_root["$SCHED"];
    for(auto it = sched_copy.begin(); it != sched_copy.end(); ++it)
    {
        if(it.key() == scan_id)
        {
            std::cout<<*it<<std::endl;
            for(std::size_t n = 0; n < (*it)["source"][0].size(); n++)
            {
                source_ids.push_back( (*it)["source"][n]["source"] );
            }
            std::string fourfit_reftime_epoch = get_fourfit_reftime_for_scan(*it);
            std::cout<<"frt!! = "<<fourfit_reftime_epoch<<std::endl;
            (*it)["fourfit_reftime"] = fourfit_reftime_epoch; //add the fourfit reference time
            sched[it.key()] = it.value(); //add this scan to the schedule section
            break;
        }
    }
    vex_root.erase("$SCHED");
    vex_root["$SCHED"] = sched;

    //rip out all sources but the one specified for this scan
    mho_json src;
    mho_json src_copy = vex_root["$SOURCE"];
    for(auto it = src_copy.begin(); it != src_copy.end(); ++it)
    {
        for(std::size_t n = 0; n < source_ids.size(); n++)
        {
            if(it.key() == source_ids[n])
            {
                std::cout<<*it<<std::endl;
                src[it.key()] = it.value();
                break;
            }
        }
    }
    vex_root.erase("$SOURCE");
    vex_root["$SOURCE"] = src;


    //std::string scan_id = fInput["scan"][fFileSet->fIndex]["identifier"];
    std::cout<<"difx scan name = "<<fFileSet->fScanName<<" = "<<scan_id<<std::endl;

    //remove all sources but the current scan's source

    MHO_VexGenerator gen;
    std::string output_file = fOutputDirectory + "/" + "test.vex";
    gen.SetFilename(output_file);
    gen.GenerateVex(vex_root);




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
        it->second.SetRootCode(fRootCode);
        it->second.SetStationCodes(fStationCodeMap);
        it->second.SetDiFXInputData(&fInput);
        it->second.ConstructVisibilityFileObjects();
        it->second.WriteVisibilityObjects(fOutputDirectory);
    }
    
    //clear out the baseline visbility containers for the next scan
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        it->second.Clear();
    }

}

void 
MHO_DiFXScanProcessor::ConvertStationFileObjects()
{
    //first extract the station coordinate quantities from the difx input
    ExtractStationCoords();

    //then process pcal files (if they exist)
    ExtractPCalData();

    //loop over all stations and write out coords and pcal data
    for(auto it = fStationCode2Coords.begin(); it != fStationCode2Coords.end(); it++)
    {
        //grab the station coordinate data
        std::string station_code = it->first;
        station_coord_type* station_coord_data_ptr = it->second;

        //if there is pcal data, make sure we grab it too
        multitone_pcal_type* pcal_data_ptr = nullptr;
        auto pcal_it = fStationCode2PCal.find(station_code);
        if(pcal_it != fStationCode2PCal.end()){pcal_data_ptr = pcal_it->second;}

        //figure out the output file name
        std::string station_mk4id = fStationCodeMap->GetMk4IdFromStationCode(station_code);
        std::string root_code = fRootCode;
        std::string output_file = fOutputDirectory + "/" + station_mk4id + "." + root_code + ".sta";

        MHO_BinaryFileInterface inter;
        bool status = inter.OpenToWrite(output_file);
        MHO_ObjectTags tags;
        tags.AddObjectUUID(station_coord_data_ptr->GetObjectUUID());
        if(pcal_data_ptr){tags.AddObjectUUID(pcal_data_ptr->GetObjectUUID());}

        if(status)
        {
            uint32_t label = 0xFFFFFFFF; //someday make this mean something
            inter.Write(tags, "tags", label);
            inter.Write( *station_coord_data_ptr, "coords", label);
            if(pcal_data_ptr){ inter.Write( *pcal_data_ptr, "pcal", label); }
            inter.Close();
        }
        else
        {
            msg_error("file", "error opening station data output file: " << output_file << eom);
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
        station_coord_type* ptr = it->second;
        delete ptr;
    }
    fStationCode2Coords.clear();
    fOutputDirectory = "";
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
MHO_DiFXScanProcessor::ExtractPCalData()
{
    for(auto it = fFileSet->fPCALFileList.begin(); it != fFileSet->fPCALFileList.end(); it++)
    {
        fPCalProcessor.SetFilename(*it);
        double ap_length = fInput["config"][0]["tInt"]; //config is a list element, grab the first item
        fPCalProcessor.SetAccumulationPeriod(ap_length);
        fPCalProcessor.ReadPCalFile();
        fPCalProcessor.Organize();

        std::string station_code = fPCalProcessor.GetStationCode();
        multitone_pcal_type* pcal = fPCalProcessor.GetPCalData()->Clone();

        fStationCode2PCal[station_code] = pcal;
    }
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
    //Note: with the exception of the phase-spline polynomial (type_302), all of these other quantities 
    //do not depend on the channel/frequency.


    std::size_t scan_index = 0;
    std::size_t nAntenna = fInput["scan"][scan_index]["nAntenna"];

    std::size_t nPhaseCenters = fInput["scan"][scan_index]["nPhaseCentres"];
    std::size_t phase_center = 0; // currently only one phase-center supported
    if(nPhaseCenters > 1)
    {
        msg_warn("difx_interface", "more than one phase center is not supported, using the first. " << eom);
    }

    for(std::size_t n=0; n<nAntenna; n++)
    {
        //first get antenna name for an ID (later we need to map this to the 2 char code)
        std::string station_code = fInput["antenna"][n]["name"];
        station_coord_type* st_coord = new station_coord_type();
        fStationCode2Coords[station_code] = st_coord;

        //get the spline model for the stations quantities 
        json antenna_poly = fInput["scan"][scan_index]["DifxPolyModel"][n][phase_center];

        //figure out the start time of this polynomial 
        //TODO FIXME! we need to convert this date information to a cannonical date/time-stamp class
        int mjd = antenna_poly[0]["mjd"];//start mjd 
        int sec = antenna_poly[0]["sec"];//start second 

        //length of time each spline is valid
        double duration = antenna_poly[0]["validDuration"]; 

        //figure out the data dimensions
        std::size_t n_order = antenna_poly[0]["order"];
        std::size_t n_coord = NCOORD; //note we do not manufacture a phase-spline (e.g. type_302)
        std::size_t n_poly = antenna_poly.size(); //aka nspline intervals in d2m4

        st_coord->Resize(n_coord, n_poly, n_order+1); //p = n_order is included!

        //label the coordinate axis
        std::get<COORD_AXIS>(*st_coord)[0] = "delay";
        std::get<COORD_AXIS>(*st_coord)[1] = "azimuth";
        std::get<COORD_AXIS>(*st_coord)[2] = "elevation";
        std::get<COORD_AXIS>(*st_coord)[3] = "parallactic_angle";
        std::get<COORD_AXIS>(*st_coord)[4] = "u";
        std::get<COORD_AXIS>(*st_coord)[5] = "v";
        std::get<COORD_AXIS>(*st_coord)[6] = "w";

        //label the spline interval axis
        for(std::size_t i=0; i<n_poly; i++)
        {
            std::get<INTERVAL_AXIS>(*st_coord)[i] = i*duration;
        }

        //label polynomial order axis
        for(std::size_t i=0; i<=n_order; i++)
        {
            std::get<COEFF_AXIS>(*st_coord)[i] = i;
        }

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





std::string 
MHO_DiFXScanProcessor::get_fourfit_reftime_for_scan(mho_json scan_obj)
{
    //this function tries to follow d2m4 method of computing fourfit reference
    //time, but rather than using the DiFX MJD, uses the vex-file epoch with hops_clock 

    //loop over all the stations in this scan and determine the latest start 
    //and earliest stop times   
    double latest_start = -1.0;
    double earliest_stop = 1e30;
    for(std::size_t n = 0; n < scan_obj["station"].size(); n++)
    {
        //assuming for the time being the units are seconds
        double start = scan_obj["station"][n]["data_good"]["value"].get<double>();
        double stop = scan_obj["station"][n]["data_stop"]["value"].get<double>();
        if(start > latest_start){latest_start = start;};
        if(stop < earliest_stop){earliest_stop = stop;}
    }

    //truncate midpoint to integer second
    int itime =  itime = (latest_start + earliest_stop) / 2;
    std::string start_epoch = scan_obj["start"].get<std::string>();
    std::cout<<"start epoch = "<<start_epoch<<std::endl;
    auto start_tp = hops_clock::from_vex_format(start_epoch); 
    auto frt_tp = start_tp + std::chrono::seconds(itime);

    std::string frt = hops_clock::to_vex_format(frt_tp);

    return frt;
}


}//end of namespace
