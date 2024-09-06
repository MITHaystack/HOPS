#include "MHO_DiFXScanProcessor.hh"
#include "MHO_VexParser.hh"
#include "MHO_VexGenerator.hh"
#include "MHO_Clock.hh"

#include "MHO_Reducer.hh"

#include <math.h>
#include <sys/stat.h>
#include <time.h>
#include <ctime>
#include <iomanip>

#include <cctype> //toupper
#include <algorithm>


#define EPS 1e-15

//this is the nominal DiFX MJD epoch start...however, it will be off by however
//many leap seconds have been inserted between this time and the time point of
//interest...so when UTC times are calculated from DiFX MJD values, this epoch
//start must be corrected by the number of leap seconds inserted (total of 5 as of 2023)
#define DIFX_J2000_MJD_EPOCH_UTC_ISO8601 "2000-01-01T12:00:00.000000000Z"
#define DIFX_J2000_MJD_EPOCH_OFFSET 51544.50000

namespace hops
{

MHO_DiFXScanProcessor::MHO_DiFXScanProcessor()
{
    fRootCode = "unknown";
    fStationCodeMap = nullptr;
    fPreserveDiFXScanNames = false;
    fNormalize = false;
    MICROSEC_TO_SEC = 1e-6; //needed to match difx2mark4 convention
    fFreqBands.clear();
    fFreqGroups.clear();
    fSelectByBandwidth = false;
    fOnlyBandwidth = 0;
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
        CreateRootFileObject(fileSet.fVexFile); //create the equivalent to the Mk4 'ovex' root file
        ConvertVisibilityFileObjects(); //convert visibilities and data weights
        ConvertStationFileObjects(); //convert the station splines, and pcal data
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
    if(vexfile != "")
    {
        MHO_VexParser vparser;
        vparser.SetVexFile(vexfile);
        mho_json vex_root = vparser.ParseVex();

        //now convert the 'vex' to 'ovex' (using only subset of information)
        vex_root[ MHO_VexDefinitions::VexRevisionFlag() ] = "ovex";

        //add the experiment number
        (*(vex_root["$EXPER"].begin()))["exper_num"] = fExperNum;

        //always replace the target correlator with difx
        //(so we are compatible with HOP3 vex parser)
        (*(vex_root["$EXPER"].begin()))["target_correlator"] = "difx";

        std::string scan_id = fInput["scan"][fFileSet->fIndex]["identifier"];
        std::vector< std::string > source_ids;

        //rip out all scans but the one we are processing
        mho_json sched;
        mho_json sched_copy = vex_root["$SCHED"];
        std::string mode_name; //grab this in this loop, we'll need it for the $TRACKS info later
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
                mode_name = it.value()["mode"].get<std::string>();
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
        //also create a map of DiFX input file station codes (all upper-case) to vex station codes (any-case)
        fDiFX2VexStationCodes.clear(); //station code map
        fDiFX2VexStationNames.clear(); //station name map
        for(auto it = vex_root["$SITE"].begin(); it != vex_root["$SITE"].end(); ++it)
        {
            std::string vex_station_code = (*it)["site_ID"];
            std::string vex_station_name = (*it)["site_name"];
            (*it)["mk4_site_ID"] = fStationCodeMap->GetMk4IdFromStationCode(vex_station_code);
            //Careful!! this will break if there are two stations that only differ by case
            //needed because while the DiFX input-file makes all station codes upper-case
            //the vex file does not (and we want to preserve the vex codes) - jpb 02/15/24
            std::string difx_station_code = vex_station_code;
            std::transform(difx_station_code.begin(), difx_station_code.end(), difx_station_code.begin(), ::toupper);
            fDiFX2VexStationCodes[difx_station_code] = vex_station_code;
            fDiFX2VexStationNames[difx_station_code] = vex_station_name;
        }

        //This assumes all datastreams at a antenna are all sampled with the same bit depth
        //this is probably relatively safe,
        //but otherwise we would need to track this on a per-channel or per-band basis
        std::map< std::string, int >  code2bits;
        std::map< int, std::string >  id2code;
        std::set< int > bits_present;
        if( fInput.contains("datastream") && fInput.contains("antenna") )
        {
            for(auto it : fInput["datastream"].items() )
            {
                if( it.value().contains("antennaId") && it.value().contains("quantBits") )
                {
                    int antennaId = it.value()["antennaId"].get<int>();
                    int bits = it.value()["quantBits"].get<int>();
                    int antId = 0;
                    for(auto it = fInput["antenna"].begin(); it != fInput["antenna"].end(); it++)
                    {
                        if(antId == antennaId && it.value().contains("name"))
                        {
                            std::string antCode = it.value()["name"].get<std::string>();
                            code2bits[antCode] = bits;
                            id2code[antennaId] = antCode;
                            break;
                        }
                        antId++;
                    }
                }
            }
        }

        //fake it 'til we make it (just like difx2mark4)
        //add an OVEX $TRACKS section for each site just to keep track of # bits/sample
        //we'll create a tracks "trax" entry for each possible bits/sample that we've encountered
        //we will then re-link the stations to this OVEX tracks section in the MODE section
        mho_json tracks_section;
        std::set< int > added_trax;
        std::map< std::string, std::vector< std::string > > trax2codes;
        for(auto it = code2bits.begin(); it != code2bits.end(); it++)
        {
            std::stringstream ss;
            ss << "trax_" << it->second << "bits";
            std::string trax_name = ss.str();

            auto iter_bool_pair = added_trax.insert(it->second);
            if(iter_bool_pair.second)
            {
                //first time we are seeing this #bits/sample
                mho_json trax;
                trax["bits/sample"] = it->second;

                tracks_section[trax_name] = trax;
            }
            //add the (vex) station code to the list of stations attached to this trax specification
            trax2codes[trax_name].push_back( fDiFX2VexStationCodes[ it->first ] );
        }

        //insert our new trax info
        if( vex_root.contains("$TRACKS") ){ vex_root["$TRACKS"].update(tracks_section); }
        else{ vex_root["$TRACKS"] = tracks_section; }

        //clear all existing tracks in the $MODE section, and re-link to only our 'trax'
        vex_root["$MODE"][mode_name]["$TRACKS"].clear();
        for(auto it = trax2codes.begin(); it != trax2codes.end(); it++)
        {
            mho_json trax_obj;
            trax_obj["keyword"] = it->first;
            trax_obj["qualifiers"] = it->second;
            vex_root["$MODE"][mode_name]["$TRACKS"].push_back(trax_obj);
        }

        //make sure we link to an $EOP object (only necessary for pedantic HOPS3 ovex parser)
        // vex_root["$MODE"][mode_name]["$EOP"].clear();
        // for(auto it = trax2codes.begin(); it != trax2codes.end(); it++)
        // {
        //     mho_json trax_obj;
        //     trax_obj["keyword"] = it->first;
        //     trax_obj["qualifiers"] = it->second;
        //     vex_root["$MODE"][mode_name]["$TRACKS"].push_back(trax_obj);
        // }



        //lastly we need to insert the traditional mk4 channel names for each frequency
        //TODO FIXME -- need to support zoom bands (requires difx .input data)
        //fChanNameConstructor.AddChannelNames(vex_root);
        //and/or adapt the channel defintions to deal with zoom bands
        //std::string tmp = vex_root["$FREQ"]["VGOS_std"]["chan_def"][0]["channel_name"].get<std::string>();

        //generate a traditional 'ovex' file too...for backwards compatibility
        MHO_VexGenerator gen;
        std::string ovex_output_file = fOutputDirectory + "/" + src_name + "." + fRootCode;
        gen.SetFilename(ovex_output_file);
        gen.GenerateVex(vex_root);

        //write out the 'ovex'/'vex' json object as a json file

        std::string output_file = ConstructRootFileName(fOutputDirectory, fRootCode, src_name);
        //open and dump to file
        std::ofstream outFile(output_file.c_str(), std::ofstream::out);
        outFile << vex_root.dump(2);
        outFile.close();
    }
    else
    {
        msg_error("difx_interface", "could not determine vex file, will not be able to generate root (ovex) information." << eom );
    }
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

    //pass these parameters if we need to select on bandwidth or freq group
    if(fFreqBands.size() != 0){ visProcessor.SetFrequencyBands(fFreqBands); }
    if(fFreqGroups.size() != 0){ visProcessor.SetFreqGroups(fFreqGroups); }
    if(fSelectByBandwidth){  visProcessor.SetOnlyBandwidth(fOnlyBandwidth); }
    visProcessor.SetDiFXInputData(&fInput);
    visProcessor.SetFilename(fFileSet->fVisibilityFileList[0]);
    visProcessor.ReadDIFXFile(fAllBaselineVisibilities);

    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        //it->second is a MHO_DiFXBaselineProcessor
        it->second.SetRescaleTrue(); //default is to always apply VanVleck and x10000 scaling
        it->second.SetRootCode(fRootCode);
        it->second.SetCorrelationDate(fCorrDate);
        it->second.SetStationCodes(fStationCodeMap);
        it->second.SetDiFXCodes2VexCodes(fDiFX2VexStationCodes);
        it->second.SetDiFXCodes2VexNames(fDiFX2VexStationNames);
        it->second.SetDiFXInputData(&fInput);
        it->second.ConstructVisibilityFileObjects();
    }

    //need to normalize each baseline by the auto-corrs
    if(fNormalize)
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
    fAllBaselineVisibilities.clear();


}

void
MHO_DiFXScanProcessor::NormalizeVisibilities()
{
    msg_debug("difx_interface", "normalizing visibilities"<< eom);

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
            auto vis = it->second.GetVisibilities();
            if(vis != nullptr)
            {
                raw_auto_corrs[station_id] = vis;
            }
        }
        //add all here (including autocorrs)
        std::string baseline = it->second.GetBaselineShortName();
        auto vis = it->second.GetVisibilities();
        if(vis != nullptr)
        {
            raw_visibilities[baseline] = vis;
        }
    }

    //for the auto-corrs compute the sum/average of all the values for each pol/ap
    MHO_Reducer<visibility_store_type, MHO_CompoundSum> reducer;
    reducer.ReduceAxis(FREQ_AXIS);
    for(auto it = raw_auto_corrs.begin(); it != raw_auto_corrs.end(); it++)
    {
        std::string station_id = it->first;
        //std::cout<<"REDUCING AUTOCORR = "<<station_id<<std::endl;
        visibility_store_type* auto_corrs = it->second;
        visibility_store_type* reduced = new visibility_store_type();
        std::size_t npp = auto_corrs->GetDimension(POLPROD_AXIS);
        std::size_t nch = auto_corrs->GetDimension(CHANNEL_AXIS);
        std::size_t nap = auto_corrs->GetDimension(TIME_AXIS);
        std::size_t n_spectral_pts = auto_corrs->GetDimension(FREQ_AXIS);
        reduced->Resize(npp, nch, nap, 1);
        reduced->ZeroArray();
        reducer.SetArgs(auto_corrs, reduced);
        reducer.Initialize();
        reducer.Execute();
        (*reduced) *= 1.0/( (double) n_spectral_pts ); //divide by number of spectral points
        reduced_auto_corrs[station_id] = reduced;
    }

    //now apply normalization to visibilities
    for(auto it = raw_visibilities.begin(); it != raw_visibilities.end(); it++)
    {
        std::string baseline = it->first;
        auto vis = it->second;
        std::size_t npp = vis->GetDimension(POLPROD_AXIS);
        std::size_t nap = vis->GetDimension(TIME_AXIS);
        std::size_t nch = vis->GetDimension(CHANNEL_AXIS);
        std::string ref_st = std::string() + (char) baseline[0];
        std::string rem_st = std::string() + (char) baseline[1];

        //do all baselines (including autocorrs)
        bool have_both = true;
        if( reduced_auto_corrs.find(ref_st) == reduced_auto_corrs.end() ){have_both = false;}
        if( reduced_auto_corrs.find(rem_st) == reduced_auto_corrs.end() ){have_both = false;}
        if(have_both)
        {
            auto ref_ac = reduced_auto_corrs[ref_st];
            auto rem_ac = reduced_auto_corrs[rem_st];
            for(std::size_t pp=0; pp<npp; pp++)
            {
                //figure out the pol-mapping to the right autocorrs
                std::string polprod = std::get<POLPROD_AXIS>(*vis)(pp);
                std::string ref_polprod = std::string() + (char)polprod[0] + (char)polprod[0];
                std::string rem_polprod = std::string() + (char)polprod[1] + (char)polprod[1];
                std::size_t ref_pp_idx, rem_pp_idx;
                bool ref_ok = std::get<POLPROD_AXIS>(*ref_ac).SelectFirstMatchingIndex(ref_polprod, ref_pp_idx);
                bool rem_ok = std::get<POLPROD_AXIS>(*rem_ac).SelectFirstMatchingIndex(rem_polprod, rem_pp_idx);

                bool issue_once = true;
                if(!ref_ok || !rem_ok)
                {
                    msg_error("difx_interface",
                        "error missing data in autocorrs needed to normalize: "
                        <<baseline<<":"<<polprod<<" "<<pp<<"."<<eom);
                }
                else
                {
                    for(std::size_t ap=0;ap<nap;ap++)
                    {
                        for(std::size_t ch=0; ch<nch; ch++)
                        {
                            double ref_val = std::sqrt( std::real( (*ref_ac)(ref_pp_idx,ch,ap,0) ) );
                            double rem_val = std::sqrt( std::real( (*rem_ac)(rem_pp_idx,ch,ap,0) ) );
                            double factor = 1.0;
                            if( std::fabs(ref_val) == 0.0 || std::fabs(rem_val) == 0.0)
                            {
                                if(issue_once)
                                {
                                    std::string st_code = "";
                                    if( std::fabs(ref_val) == 0.0 ){st_code = ref_st;}
                                    if( std::fabs(rem_val) == 0.0 ){st_code = rem_st;}
                                    msg_error("difx_interface", "zero value in auto-corrs of station: "<< st_code << ", normalization may not be correct."<<eom);
                                    issue_once = false;
                                }
                            }
                            else{factor = 1.0/(ref_val*rem_val);}
                            vis->SliceView(pp,ch,ap,":") *= factor;
                        }
                    }
                }
            }
        }
        else
        {
            msg_error("difx_interface", "cannot locate autocorrs for both stations of baseline: " << baseline <<", will not normalize." << eom);
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
        std::string difx_station_code = it->first;
        station_coord_type* station_coord_data_ptr = it->second;

        //figure out the 2-char vex code (if it exists)
        std::string vex_station_code = difx_station_code;
        if(fDiFX2VexStationCodes.find(difx_station_code) != fDiFX2VexStationCodes.end()){vex_station_code = fDiFX2VexStationCodes[difx_station_code];}

        std::string vex_station_name = difx_station_code;
        if(fDiFX2VexStationNames.find(difx_station_code) != fDiFX2VexStationNames.end()){vex_station_name = fDiFX2VexStationNames[difx_station_code];}

        std::string station_mk4id = fStationCodeMap->GetMk4IdFromStationCode(difx_station_code);

        //if there is pcal data, make sure we grab it too, and add some tags to it
        multitone_pcal_type* pcal_data_ptr = nullptr;
        auto pcal_it = fStationCode2PCal.find(difx_station_code);
        if(pcal_it != fStationCode2PCal.end())
        {
            pcal_data_ptr = pcal_it->second;
            pcal_data_ptr->Insert(std::string("difx_station_code"), difx_station_code);
            pcal_data_ptr->Insert(std::string("station_code"), vex_station_code);
            pcal_data_ptr->Insert(std::string("station_mk4id"), station_mk4id);
            pcal_data_ptr->Insert(std::string("station_name"), vex_station_name);
            pcal_data_ptr->Insert(std::string("name"), std::string("pcal"));
        }

        //add some tags to the station coord data
        station_coord_data_ptr->Insert(std::string("difx_station_code"), difx_station_code);
        station_coord_data_ptr->Insert(std::string("station_code"), vex_station_code);
        station_coord_data_ptr->Insert(std::string("station_mk4id"), station_mk4id);
        station_coord_data_ptr->Insert(std::string("station_name"), vex_station_name);
        station_coord_data_ptr->Insert(std::string("name"), std::string("station_data"));


        //figure out the output file name
        std::string root_code = fRootCode;
        std::string output_file = ConstructStaFileName(fOutputDirectory, root_code, vex_station_code, station_mk4id);

        MHO_BinaryFileInterface inter;
        bool status = inter.OpenToWrite(output_file);
        MHO_ObjectTags tags;
        tags.AddObjectUUID(station_coord_data_ptr->GetObjectUUID());
        if(pcal_data_ptr){tags.AddObjectUUID(pcal_data_ptr->GetObjectUUID());}

        if(status)
        {
            inter.Write(tags, "tags");
            inter.Write( *station_coord_data_ptr, "sta");
            if(pcal_data_ptr){ inter.Write( *pcal_data_ptr, "pcal"); }
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

    //grab the date when the fInputFile was last modified as the correlation time
    struct tm *mod_time;
    struct stat attrib;
    fCorrDate = "";
    int err = stat(fFileSet->fInputFile.c_str(), &attrib);
    if (err)
    {
        msg_debug("difx_interface", "could not stat input file: " << fFileSet->fInputFile <<", correlation date will be set to start of J2000 epoch." << eom);
        fCorrDate = hops_clock::to_vex_format(hops_clock::get_hops_epoch() );
    }
    else
    {
        mod_time = gmtime(&(attrib.st_mtime));
        std::time_t timeval = std::mktime(mod_time);
        auto mod_datetime = std::chrono::system_clock::from_time_t(timeval);
        auto hops_datetime = hops_clock::from_sys(mod_datetime);
        fCorrDate = hops_clock::to_vex_format(hops_datetime);
    }

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

    std::size_t phaseCenterSrcId = fInput["scan"][scan_index]["phsCentreSrcs"][phase_center];
    mho_json src = fInput["source"][phaseCenterSrcId];
    double src_dec = src["dec"]; //needed for parallatic angle calculation

    for(std::size_t n=0; n<nAntenna; n++)
    {
        //first get antenna name for an ID (later we need to map this to the 2 char code)
        std::string difx_station_code = fInput["antenna"][n]["name"];
        station_coord_type* st_coord = new station_coord_type();
        fStationCode2Coords[difx_station_code] = st_coord;

        //get the spline model for the stations quantities
        mho_json antenna_poly = fInput["scan"][scan_index]["DifxPolyModel"][n][phase_center];

        //get the antenna info
        mho_json ant = fInput["antenna"][n];
        std::string station_name = ant["name"];
        std::string station_mount = ant["mount"];
        std::vector<double> position = ant["position"];
        double clockrefmjd = ant["clockrefmjd"];

        //figure out the start time of this polynomial
        //and convert this date information to a cannonical date/time-stamp class
        int mjd = antenna_poly[0]["mjd"];//start mjd
        int sec = antenna_poly[0]["sec"];//start second

        std::string model_start = get_vexdate_from_mjd_sec(mjd,sec);

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
            mho_json poly_interval = antenna_poly[i];
            for(std::size_t p=0; p<=n_order; p++)
            {
                double delay, az, el, par, u, v, w;
                delay = poly_interval["delay"][p];
                az = poly_interval["az"][p];
                el = poly_interval["elgeom"][p];
                #ifdef CALC_SUPPORTS_PARANGLE //not defined! CALC does not yet support par. angle spline
                par = poly_interval["parangle"][p];
                #else
                //just fill in dummy values for now
                par = 0.0;
                #endif
                u = poly_interval["u"][p];
                v = poly_interval["v"][p];
                w = poly_interval["w"][p];

                st_coord->at(0,i,p) = -1.0 * MICROSEC_TO_SEC * delay; //negative sign needed to match difx2mar4 convention
                st_coord->at(1,i,p) = az;
                st_coord->at(2,i,p) = el;
                st_coord->at(3,i,p) = par;
                st_coord->at(4,i,p) = u;
                st_coord->at(5,i,p) = v;
                st_coord->at(6,i,p) = w;
            }
        }

        //correct delay mode with antenna clock model
        apply_delay_model_clock_correction(ant, antenna_poly, st_coord);

        //tag the station data structure with all the meta data from the type_300
        // st_coord->Insert(std::string("difx_station_code"), station_code);
        // st_coord->Insert(std::string("difx_station_name"), station_name);
        st_coord->Insert(std::string("model_interval"), duration);
        st_coord->Insert(std::string("model_start"), model_start);

        //do we really need to add these parameters here (available from vex)
        st_coord->Insert(std::string("mount"), station_mount);
        st_coord->Insert(std::string("X"), position[0]);
        st_coord->Insert(std::string("Y"), position[1]);
        st_coord->Insert(std::string("Z"), position[2]);

        //calculate zero-th order parallactic_angle values
        calculateZerothOrderParallacticAngle(st_coord, position[0], position[1], position[2], src_dec, duration);

        //store n_poly as int
        int nsplines = n_poly;
        st_coord->Insert(std::string("nsplines"), nsplines);
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

std::string
MHO_DiFXScanProcessor::get_vexdate_from_mjd_sec(double mjd, double sec)
{
    double total_mjd = (double)mjd + (double)sec /86400.0;
    auto difx_mjd_epoch = hops_clock::from_iso8601_format(DIFX_J2000_MJD_EPOCH_UTC_ISO8601);
    //figure out the approximate time of this mjd (ignoring leap seconds)
    auto approx_tp = hops_clock::from_mjd(difx_mjd_epoch, DIFX_J2000_MJD_EPOCH_OFFSET, total_mjd);
    //calculate the number of leap seconds
    auto n_leaps = hops_clock::get_leap_seconds_between(difx_mjd_epoch, approx_tp);
    //now correct the nominal difx start epoch to deal with the number of leap seconds
    auto actual_epoch_start = difx_mjd_epoch + n_leaps;
    msg_debug("difx_interface", "the leap-second corrected difx mjd epoch start is: "<<  hops_clock::to_iso8601_format( actual_epoch_start ) << eom);
    auto mjd_tp = hops_clock::from_mjd(actual_epoch_start, DIFX_J2000_MJD_EPOCH_OFFSET, total_mjd );
    msg_debug("difx_interface", "the MJD value: "<< total_mjd <<" as a vex timestamp is: "<< hops_clock::to_vex_format(mjd_tp) << eom);
    return hops_clock::to_vex_format(mjd_tp);
}


void
MHO_DiFXScanProcessor::apply_delay_model_clock_correction(const mho_json& ant, const mho_json& ant_poly, station_coord_type* st_coord)
{
    //see difx2mar4 createType3s.c line 269
    double clock[MAX_MODEL_ORDER+1];
    // units of difx are usec, ff uses sec
    // shift clock polynomial to start of model interval

    double clockrefmjd = ant["clockrefmjd"];;
    double modelrefmjd = ant_poly[0]["mjd"];//start mjd
    double modelrefsec = ant_poly[0]["sec"];//start second
    double deltat = 86400.0 * (modelrefmjd - clockrefmjd) + modelrefsec;
    //loop over each interval
    for(std::size_t i=0; i< std::get<INTERVAL_AXIS>(*st_coord).GetSize(); i++)
    {
        double sec_offset = std::get<INTERVAL_AXIS>(*st_coord)[i];
        double dt = deltat + sec_offset;
        int nclock = local_getDifxAntennaShiftedClock(ant, dt, 6, clock);

        // difx delay doesn't have clock added in, so we must do it here
        //loop over poly coeff
        for(int p=0; p<MAX_MODEL_ORDER+1; p++)
        {
            if(p < nclock) // add in those clock coefficients that are valid
            {
                //negative sign to match difx2mar4 convention
                st_coord->at(0,i,p) -= MICROSEC_TO_SEC * clock[p];
            }
        }
    }

}


//lifted from difx_antenna.c line 288 with minor changes
//(copied here so we can avoid introducing additional dependencies to difxio lib)
int
MHO_DiFXScanProcessor::local_getDifxAntennaShiftedClock(const mho_json& da, double dt, int outputClockSize, double *clockOut)
{
    if( !(da.contains("clockorder")) || !(da.contains("clockcoeff")) )
    {
            return -1;
    }
    int clockorder = da["clockorder"];

    if(outputClockSize < clockorder+1)
    {
        return -2;
    }

    double a[MAX_MODEL_ORDER+1]; //MAX_MODEL_ORDER defined in difx_input.h
    for(int i = 0; i < MAX_MODEL_ORDER+1; ++i)             // pad out input array to full order with 0's
    {
        a[i] = 0.0;
        if(i <= clockorder)
        {
            double value = da["clockcoeff"][i];
            a[i] = value;
        }
    }

    double t2, t3, t4, t5;  /* units: sec^n, n = 2, 3, 4, 5 */
    t2 = dt * dt;
    t3 = t2 * dt;
    t4 = t2 * t2;
    t5 = t3 * t2;

    switch(clockorder)
    {
        case 5: clockOut[5] = a[5];
        case 4: clockOut[4] = a[4] + 5 * a[5] * dt;
        case 3: clockOut[3] = a[3] + 4 * a[4] * dt + 10 * a[5] * t2;
        case 2: clockOut[2] = a[2] + 3 * a[3] * dt +  6 * a[4] * t2 + 10 * a[5] * t3;
        case 1: clockOut[1] = a[1] + 2 * a[2] * dt +  3 * a[3] * t2 +  4 * a[4] * t3 + 5 * a[5] * t4;
        case 0: clockOut[0] = a[0] +     a[1] * dt +      a[2] * t2 +      a[3] * t3 +     a[4] * t4 + a[5] * t5;
    }

    return clockorder + 1;
}

//adapted from difx2mark createType3s.c
void MHO_DiFXScanProcessor::calculateZerothOrderParallacticAngle(station_coord_type* st_coord, double X, double Y, double Z, double dec, double dt)
{
    //station coordinates: X, Y, Z;
    //source declination: dec;
    //poly model interval: dt;

    std::size_t n_poly = std::get<INTERVAL_AXIS>(*st_coord).GetSize();
    for(std::size_t i=0; i<n_poly; i++)
    {
        // par. angle from calc program is NYI, so add zeroth order approx here
        for(std::size_t p=0; p<6; p++)
        {
            if(p == 0) // for now, only constant term is non-zero
            {
                double az0 = st_coord->at(1,i,0); //1 is az
                double az1 = st_coord->at(1,i,1);
                double el0 = st_coord->at(2,i,0); //2 is el
                double el1 = st_coord->at(2,i,1);
                // calculate geocentric latitude (rad)
                double geoc_lat = std::atan2(Z, std::sqrt(X*X + Y*Y) );
                // evaluate az & el at midpoint of spline interval
                double el = ( M_PI / 180.0 ) * (el0 + 0.5*dt*el1);
                double az = ( M_PI / 180.0 ) * (az0 + 0.5*dt*az1);
                // evaluate sin and cos of the local hour angle
                double sha = -1.0 * std::cos(el) * std::sin(az) / std::cos(dec);
                double cha = ( std::sin(el) - std::sin(geoc_lat) * std::sin(dec) ) / ( std::cos(geoc_lat) * std::cos(dec) );
                // approximate (first order in f) conversion (where does the magic number 1.00674 come from?)
                double geod_lat = std::atan(1.00674 * std::tan(geoc_lat));
                // finally ready for par. angle
                double par_angle = (180.0 / M_PI) * std::atan2(sha, ( std::cos(dec) * std::tan(geod_lat) - std::sin(dec) * cha) );
                st_coord->at(3,i,p) = par_angle; //3 is par. angle
            }
            else
            {
                st_coord->at(3,i,p) = 0.0; //all other coeff are set to zero
            }
        }
    }
}

std::string
MHO_DiFXScanProcessor::ConstructRootFileName(const std::string& output_dir,
                                             const std::string& root_code,
                                             const std::string& src_name)
{
    std::string output_file = output_dir + "/" + src_name + "." + root_code + ".root.json";
    return output_file;
}


std::string
MHO_DiFXScanProcessor::ConstructStaFileName(const std::string& output_dir,
                                            const std::string& root_code,
                                            const std::string& station_code,
                                            const std::string& station_mk4id)
{
    std::string output_file = output_dir + "/" + station_mk4id + "." + station_code + "."  + root_code + ".sta";
    return output_file;
}

}//end of namespace
