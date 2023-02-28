#include "MHO_DiFXScanProcessor.hh"
#include "MHO_VexParser.hh"
#include "MHO_VexGenerator.hh"
#include "MHO_Clock.hh"

#include "MHO_Reducer.hh"

#include <math.h> 

#define EPS 1e-15



namespace hops 
{

MHO_DiFXScanProcessor::MHO_DiFXScanProcessor()
{
    fRootCode = "unknown";
    fStationCodeMap = nullptr;
    fPreserveDiFXScanNames = false;
    fNormalize = false;
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

    //now convert the 'vex' to 'ovex' (using only subset of information)
    vex_root[ MHO_VexDefinitions::VexRevisionFlag() ] = "ovex";

    //add the experiment number 
    vex_root["$EXPER"]["exper_num"] = fExperNum;

    std::string scan_id = fInput["scan"][fFileSet->fIndex]["identifier"];
    std::vector< std::string > source_ids;

    //rip out all scans but the one we are processing
    mho_json sched;
    mho_json sched_copy = vex_root["$SCHED"];
    for(auto it = sched_copy.begin(); it != sched_copy.end(); ++it)
    {
        if(it.key() == scan_id)
        {
            for(std::size_t n = 0; n < (*it)["source"][0].size(); n++)
            {
                source_ids.push_back( (*it)["source"][n]["source"] );
            }
            (*it)["fourfit_reftime"] = get_fourfit_reftime_for_scan(*it); //add the fourfit reference time
            sched[it.key()] = it.value(); //add this scan to the schedule section
            break;
        }
    }
    vex_root.erase("$SCHED");
    vex_root["$SCHED"] = sched;

    //rip out all sources but the one specified for this scan
    std::string src_name = "unknown";
    mho_json src;
    mho_json src_copy = vex_root["$SOURCE"];
    for(auto it = src_copy.begin(); it != src_copy.end(); ++it)
    {
        for(std::size_t n = 0; n < source_ids.size(); n++)
        {
            if(it.key() == source_ids[n])
            {
                src[it.key()] = it.value();
                src_name = (it.value())["source_name"];
                break;
            }
        }
    }
    vex_root.erase("$SOURCE");
    vex_root["$SOURCE"] = src;

    //make sure the mk4_site_id single-character codes are specified for each site 
    for(auto it = vex_root["$SITE"].begin(); it != vex_root["$SITE"].end(); ++it)
    {
        std::string station_code = (*it)["site_ID"];
        (*it)["mk4_site_ID"] = fStationCodeMap->GetMk4IdFromStationCode(station_code);
    }

    //lastly we need to insert the traditional mk4 channel names for each frequency
    //TODO FIXME -- need to support zoom bands (requires difx .input data)
    fChanNameConstructor.AddChannelNames(vex_root);
    //and/or adapt the channel defintions to deal with zoom bands
    //std::string tmp = vex_root["$FREQ"]["VGOS_std"]["chan_def"][0]["channel_name"].get<std::string>();

    MHO_VexGenerator gen;
    std::string output_file = fOutputDirectory + "/" + src_name + "." + fRootCode;
    gen.SetFilename(output_file);
    gen.GenerateVex(vex_root);

    //we also write out the 'ovex'/'vex' json object as a json file
    output_file = fOutputDirectory + "/" + src_name + "." + fRootCode + ".json";
    //open and dump to file 
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << vex_root.dump(2);
    outFile.close();
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
        // it->second.SetNormalizeFalse();
        // if(fNormalize){it->second.SetNormalizeTrue();}
        it->second.SetRootCode(fRootCode);
        it->second.SetStationCodes(fStationCodeMap);
        it->second.SetDiFXInputData(&fInput);
        it->second.ConstructVisibilityFileObjects();
    }

    //need to normalize each baseline by the auto-corrs
    //if(fNormalize)
    {
        NormalizeVisibilities();
    }

    //finally write out the visibility files
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        it->second.WriteVisibilityObjects(fOutputDirectory);
    }
    
    //clear out the baseline visbility containers for the next scan
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        it->second.Clear();
    }

}

void 
MHO_DiFXScanProcessor::NormalizeVisibilities()
{
    msg_debug("difx_interface", "normalizing visibilities"<<eom;);

    //map station to autocorrs
    std::map<std::string, visibility_store_type*> raw_auto_corrs;
    //map station to norm coeffs
    std::map<std::string, visibility_store_type*> reduced_auto_corrs;
    //map baseline to visibilities
    std::map<std::string, visibility_store_type*> raw_visibilities;

    //first need to locate all of the auto-corrs, and visibilities
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        if( it->second.IsAutoCorr() )
        {
            std::string station_id = it->second.GetRefStationMk4Id();
            std::cout<<"found autocorr: "<<station_id<<std::endl;
            auto vis = it->second.GetVisibilities();
            raw_auto_corrs[station_id] = vis;
        }
        else 
        {
            std::string baseline = it->second.GetBaselineShortName();
            auto vis = it->second.GetVisibilities();
            raw_visibilities[baseline] = vis;
        }
    }
    

    //for the auto-corrs compute the sum/average of all the values for each pol/ap 
    MHO_Reducer<visibility_store_type, MHO_CompoundSum> reducer;
    reducer.ReduceAxis(FREQ_AXIS);
    reducer.ReduceAxis(TIME_AXIS);
    for(auto it = raw_auto_corrs.begin(); it != raw_auto_corrs.end(); it++)
    {
        std::string station_id = it->first;
        std::cout<<"REDUCING AUTOCORR = "<<station_id<<std::endl;
        visibility_store_type* auto_corrs = it->second;
        visibility_store_type* reduced = new visibility_store_type();
        std::size_t npp = auto_corrs->GetDimension(POLPROD_AXIS);
        std::size_t nch = auto_corrs->GetDimension(CHANNEL_AXIS);
        std::size_t nap = auto_corrs->GetDimension(TIME_AXIS);
        std::size_t n_spectral_pts = auto_corrs->GetDimension(FREQ_AXIS); //do we need to know naps too?
        reduced->Resize(npp, nch, 1, 1);
        reduced->ZeroArray();
        reducer.SetArgs(auto_corrs, reduced);
        reducer.Initialize();
        reducer.Execute();
        (*reduced) *= (1.0/( (double)n_spectral_pts * (double)nap) ); //divide by number of spectral points x num aps
        reduced_auto_corrs[station_id] = reduced;
    }

    //now apply normalizatioin to visibilities
    for(auto it = raw_visibilities.begin(); it != raw_visibilities.end(); it++)
    {
        std::string baseline = it->first;
        std::cout<<"WORKING ON BASELINE: "<<baseline<<std::endl;
        auto vis = it->second;
        std::size_t npp = vis->GetDimension(POLPROD_AXIS);
        std::size_t nch = vis->GetDimension(CHANNEL_AXIS);
        std::string ref_st = std::string() + (char) baseline[0];
        std::string rem_st = std::string() + (char) baseline[1];

        if(ref_st != rem_st) //only do cross corrs 
        {
            std::cout<<"CROSS CORR"<<std::endl;
            auto ref_ac = reduced_auto_corrs[ref_st];
            auto rem_ac = reduced_auto_corrs[rem_st];
            for(std::size_t pp=0; pp<npp; pp++)
            {
                //figure out the pol-mapping to the right autocorrs (we ignore cross-autos)
                std::string polprod = std::get<POLPROD_AXIS>(*vis)(pp);
                std::string ref_polprod = std::string() + (char)polprod[0] + (char)polprod[0];
                std::string rem_polprod = std::string() + (char)polprod[1] + (char)polprod[1];
                std::size_t ref_pp_idx, rem_pp_idx;
                bool ref_ok = std::get<POLPROD_AXIS>(*ref_ac).SelectFirstMatchingIndex(ref_polprod, ref_pp_idx);
                bool rem_ok = std::get<POLPROD_AXIS>(*rem_ac).SelectFirstMatchingIndex(rem_polprod, rem_pp_idx);

                if(!ref_ok || !rem_ok)
                {
                    msg_error("difx_interface", 
                        "error missing pol-product in autocorrs needed to normalize: "
                        <<baseline<<":"<<polprod<<"."<<eom);
                }
                else 
                {
                    for(std::size_t ch=0; ch<nch; ch++)
                    {
                        double ref_val = std::sqrt( std::real( (*ref_ac)(ref_pp_idx,ch,0,0) ) );
                        double rem_val = std::sqrt( std::real( (*rem_ac)(rem_pp_idx,ch,0,0) ) );
                        std::cout<<"pref = "<<ref_val<<std::endl;
                        std::cout<<"prem = "<<rem_val<<std::endl;

                        double factor = 1.0;
                        if( std::fabs(ref_val) == 0.0 || std::fabs(rem_val) == 0.0)
                        {
                            msg_error("difx_interface", "zero value in auto-corrs, normalization may not be correct."<<eom);
                        }
                        else{factor = 1.0/(ref_val*rem_val);}
                        vis->SliceView(pp,ch,":",":") *= factor;
                    }
                }
            }
        }
    }

    //finally delete the reduced auto_corr arrays
    for(auto it = reduced_auto_corrs.begin(); it != reduced_auto_corrs.end(); it++)
    {
        delete it->second;
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
    // input_proc.FillFrequencyTable();

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
    //time, but rather than using the DiFX MJD value, uses the vex-file
    //specified epoch along with hops_clock for the converion.

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

    //truncate midpoint to integer second -- this is how difx2mark4 does it
    int itime =  itime = (latest_start + earliest_stop) / 2;
    std::string start_epoch = scan_obj["start"].get<std::string>();
    auto start_tp = hops_clock::from_vex_format(start_epoch); 
    auto frt_tp = start_tp + std::chrono::seconds(itime);
    std::string frt = hops_clock::to_vex_format(frt_tp);

    return frt;
}


}//end of namespace
