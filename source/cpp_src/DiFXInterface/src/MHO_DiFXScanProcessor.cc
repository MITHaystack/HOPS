#include "MHO_DiFXScanProcessor.hh"
#include "MHO_Clock.hh"
#include "MHO_MK4ChanId.hh"
#include "MHO_SkyFreqGrid.hh"
#include "MHO_VexGenerator.hh"
#include "MHO_VexParser.hh"

#include "MHO_Reducer.hh"

#include "MHO_MK4StationInterfaceReversed.hh"

#include <ctime>
#include <iomanip>
#include <math.h>
#include <sys/stat.h>
#include <time.h>

#include <algorithm>
#include <cctype> //toupper

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
    fAttachDiFXInput = false;
    fNormalize = false;
    fExportAsMark4 = false;
    fFreqBands.clear();
    fFreqGroups.clear();
    fSelectByBandwidth = false;
    fOnlyBandwidth = 0;
};

MHO_DiFXScanProcessor::~MHO_DiFXScanProcessor(){};

void MHO_DiFXScanProcessor::SetStationCodes(MHO_StationCodeMap* code_map)
{
    fStationCodeMap = code_map;
}

void MHO_DiFXScanProcessor::ProcessScan(MHO_DiFXScanFileSet& fileSet)
{
    fFileSet = &fileSet;
    LoadInputFile(); //read .input file
    if(MHO_Message::GetInstance().GetMessageLevel() == eDebugLevel)
    {
        fileSet.PrintSummary();
    }

    //configure the zoom-band rebuilder for this scan. fDiFX2VexStationCodes
    //gets set later (inside PatchOvexStructures) once $SITE has been parsed.
    fZoomBandRebuilder.SetDiFXInputData(&fInput);
    fZoomBandRebuilder.SetFrequencyBands(fFreqBands);
    if(fSelectByBandwidth)
        fZoomBandRebuilder.SetSelectionByBandwidth(fOnlyBandwidth);
    else
        fZoomBandRebuilder.ClearSelectionByBandwidth();

    bool ok = CreateScanOutputDirectory();
    if(ok)
    {
        //prepare fRootJSON (parse VEX, rip non-current scan/source, run structural patches),
        //but defer channel naming + file emission until after visibilities have been read so
        //both sides number chan_ids against the same scan-wide global sky-freq grid.
        CreateRootFileObject(fileSet.fVexFile);
        ConvertVisibilityFileObjects(); //read+organize, compute global grid, construct, write
        FinalizeAndWriteRootFile();     //AddChannelNames (using grid) + emit ovex/root.json
        ConvertStationFileObjects();    //convert the station splines, and pcal data
    }
    else
    {
        msg_error("difx_interface",
                  "could not locate or create scan output directory: " << fOutputDirectory << ", skipping." << eom);
    }
    CleanUp(); //delete workspace and prep for next scan
}

bool MHO_DiFXScanProcessor::CreateScanOutputDirectory()
{
    //currently we just use the DiFX scan name, but we should add the option
    //to use the scan time e.g. (031-1020)

    MHO_DirectoryInterface dirInterface;

    std::string output_dir = fFileSet->fOutputBaseDirectory + "/";
    if(fPreserveDiFXScanNames)
    {
        output_dir += fFileSet->fScanName;
    }
    else
    {
        std::string scan_id = fInput["scan"][fFileSet->fLocalIndex]["identifier"];
        output_dir += scan_id;
    }
    fOutputDirectory = dirInterface.GetDirectoryFullPath(output_dir);

    bool ok = dirInterface.DoesDirectoryExist(fOutputDirectory);
    if(!ok)
    {
        ok = dirInterface.CreateDirectory(fOutputDirectory);
    }
    return ok;
}

void MHO_DiFXScanProcessor::CreateRootFileObject(std::string vexfile)
{
    if(vexfile != "")
    {
        MHO_VexParser vparser;
        vparser.SetVexFile(vexfile);
        fRootJSON = vparser.ParseVex();

        //now convert the 'vex' to 'ovex' (using only subset of information)
        fRootJSON[MHO_VexDefinitions::VexRevisionFlag()] = "ovex";

        std::string scan_id = fInput["scan"][fFileSet->fLocalIndex]["identifier"];
        std::vector< std::string > source_ids;

        //rip out all scans but the one we are processing
        mho_json sched;
        mho_json sched_copy = fRootJSON["$SCHED"];
        std::string mode_name; //grab this in this loop, we'll need it for the $TRACKS info later
        for(auto it = sched_copy.begin(); it != sched_copy.end(); ++it)
        {
            if(it.key() == scan_id)
            {
                for(std::size_t n = 0; n < (*it)["source"][0].size(); n++)
                {
                    source_ids.push_back((*it)["source"][n]["source"]);
                }
                (*it)["fourfit_reftime"] = MHO_DiFXOvexPatcher::ComputeFourfitReftime(*it);
                sched[it.key()] = it.value(); //add this scan to the schedule section
                mode_name = it.value()["mode"].get< std::string >();
                break;
            }
        }
        fRootJSON.erase("$SCHED");
        fRootJSON["$SCHED"] = sched;

        //rip out all sources but the one specified for this scan
        std::string src_name = "unknown";
        mho_json src;
        mho_json src_copy = fRootJSON["$SOURCE"];
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
        fRootJSON.erase("$SOURCE");
        fRootJSON["$SOURCE"] = src;

        //patch up the vex info to make it conform to the HOPS3 ovex parser. Note that
        //AddChannelNames is NOT run here -- it has to wait until visibility records have
        //been read so the chan_def chidx can be drawn from the same scan-wide global grid
        //the mark4 t101 chan_ids will use. FinalizeAndWriteRootFile() picks that up after
        //ConvertVisibilityFileObjects().
        fOvexPatcher.SetDiFXInputData(&fInput);
        fOvexPatcher.SetStationCodeMap(fStationCodeMap);
        fOvexPatcher.SetExperimentNumber(fExperNum);
        fOvexPatcher.SetZoomBandRebuilder(&fZoomBandRebuilder);
        fOvexPatcher.Patch(fRootJSON, mode_name);
        //pull back the station-code maps the patcher populated from $SITE; downstream
        //(MHO_DiFXBaselineProcessor) needs them.
        fDiFX2VexStationCodes = fOvexPatcher.GetDiFX2VexStationCodes();
        fDiFX2VexStationNames = fOvexPatcher.GetDiFX2VexStationNames();

        //cache for FinalizeAndWriteRootFile (which actually emits the ovex/root.json)
        fSrcName = src_name;
        fDiFXInputFilename = fInput["difx_input_filename"].get< std::string >();
    }
    else
    {
        msg_error("difx_interface",
                  "could not determine vex file, will not be able to generate root (ovex) information." << eom);
    }
}

void MHO_DiFXScanProcessor::ConvertVisibilityFileObjects()
{
    //load the visibilities
    if(fFileSet->fVisibilityFileList.size() > 1)
    {
        msg_warn("difx_interface", "more than one Swinburne file present, will only process the first one: "
                                       << fFileSet->fVisibilityFileList[0] << "." << eom);
    }

    //expectation here is that there is only a single file containing visibility
    //records from every baseline in the scan
    MHO_DiFXVisibilityProcessor visProcessor;

    //pass these parameters if we need to select on bandwidth or freq group
    if(fFreqBands.size() != 0)
    {
        visProcessor.SetFrequencyBands(fFreqBands);
    }
    if(fFreqGroups.size() != 0)
    {
        visProcessor.SetFreqGroups(fFreqGroups);
    }
    if(fSelectByBandwidth)
    {
        visProcessor.SetOnlyBandwidth(fOnlyBandwidth);
    }
    visProcessor.SetDiFXInputData(&fInput);
    visProcessor.SetFilename(fFileSet->fVisibilityFileList[0]);

    //when zoom-band VEX reconstruction is active (and user has not selected a different native bandwidth),
    //collect zoom global freq indices so AddRecord can discard native-band records and keep the
    //visibility data consistent with the reconstructed root file
    if(fZoomBandRebuilder.HasZoomBands() && fZoomBandRebuilder.WantZoomChannels())
    {
        visProcessor.SetZoomFreqIndices(fZoomBandRebuilder.CollectZoomFreqIndices());
    }

    visProcessor.ReadDIFXFile(fAllBaselineVisibilities);

    //Pass 1: configure each baseline and Organize() it. Organize() populates fBaselineFreqs
    //(the ordered sky_freq list this baseline will export) but does NOT yet build the
    //channelized visibility container. We need this state before computing the scan-wide
    //global sky-freq grid that drives mark4 chan_id numbering.
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        //it->second is a MHO_DiFXBaselineProcessor
        it->second.SetScanIndex(fFileSet->fLocalIndex);
        it->second.SetRescaleTrue(); //default is to always apply VanVleck and x10000 scaling

        if(fAttachDiFXInput)
        {
            it->second.SetAttachDiFXInputTrue();
        }
        else
        {
            it->second.SetAttachDiFXInputFalse();
        }

        if(fExportAsMark4)
        {
            it->second.SetExportAsMark4True();
        }
        else
        {
            it->second.SetExportAsMark4False();
        }

        it->second.SetRootCode(fRootCode);
        it->second.SetCorrelationDate(fCorrDate);
        it->second.SetStationCodes(fStationCodeMap);
        it->second.SetDiFXCodes2VexCodes(fDiFX2VexStationCodes);
        it->second.SetDiFXCodes2VexNames(fDiFX2VexStationNames);
        it->second.SetDiFXInputData(&fInput);
        it->second.Organize();
    }

    //Pass 2: compute the scan-wide global sky-freq grid and inject it into every consumer
    //(both the baseline processors that write t101 chan_ids and the channel-name constructor
    //that writes chan_def.channel_name).
    std::vector< double > global_grid = ComputeGlobalSkyFreqGrid();
    fChanNameConstructor.SetGlobalSkyFreqGrid(global_grid);
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        it->second.SetGlobalSkyFreqGrid(global_grid);
    }

    //Pass 3: build the channelized visibility container (uses the grid for mark4 chan_ids).
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
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

std::vector< double > MHO_DiFXScanProcessor::ComputeGlobalSkyFreqGrid(double tol) const
{
    //collect every sky_freq that any baseline in fAllBaselineVisibilities will export.
    //Each baseline's fBaselineFreqs entry is (difx_freqidx, fInput["freq"][...]); the
    //sky_freq lives under "freq" (MHz) in that json object.
    MHO_SkyFreqGrid grid;
    grid.SetTolerance(tol);
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        for(const auto& pr : it->second.GetBaselineFreqs())
        {
            grid.Add(pr.second["freq"].get< double >());
        }
    }
    grid.Finalize();
    return grid.Frequencies();
}

void MHO_DiFXScanProcessor::FinalizeAndWriteRootFile()
{
    //emits the ovex + root.json that fourfit3 / fourfit4 will read. Channel naming runs
    //here (not in CreateRootFileObject / PatchOvexStructures) so chan_def.channel_name
    //chidx comes from the same scan-wide global sky-freq grid the mark4 t101 chan_ids
    //use; otherwise the two would disagree whenever stations export different channel
    //subsets (e.g. K2's 25-of-32 DBE channels in the mixed-mode VGOS autocorr test).
    if(fSrcName.empty())
    {
        //CreateRootFileObject failed to parse a vex file; nothing to write
        return;
    }

    fChanNameConstructor.AddChannelNames(fRootJSON);

    MHO_VexGenerator gen;
    std::string ovex_output_file = fOutputDirectory + "/" + fSrcName + "." + fRootCode;
    gen.SetFilename(ovex_output_file);
    gen.GenerateVex(fRootJSON);

    std::ofstream ovex_out(ovex_output_file, std::ios::app);
    if(ovex_out.is_open())
    {
        ovex_out << "*difx_input_filename:" << fDiFXInputFilename << "\n";
    }
    ovex_out.close();

    fRootJSON["difx_input_filename"] = fDiFXInputFilename;
    std::string output_file = ConstructRootFileName(fOutputDirectory, fRootCode, fSrcName);
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << fRootJSON.dump(2);
    outFile.close();
}

void MHO_DiFXScanProcessor::NormalizeVisibilities()
{
    msg_debug("difx_interface", "normalizing visibilities" << eom);

    //map station to autocorrs
    std::map< std::string, visibility_store_type* > raw_auto_corrs;
    //map station to norm coeffs
    std::map< std::string, visibility_store_type* > reduced_auto_corrs;
    //map baseline to visibilities
    std::map< std::string, visibility_store_type* > raw_visibilities;

    //first need to locate all of the auto-corrs, and visibilities
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        if(it->second.IsAutoCorr())
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
    MHO_Reducer< visibility_store_type, MHO_CompoundSum > reducer;
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
        (*reduced) *= 1.0 / ((double)n_spectral_pts); //divide by number of spectral points
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
        std::string ref_st = std::string() + (char)baseline[0];
        std::string rem_st = std::string() + (char)baseline[1];

        //do all baselines (including autocorrs)
        bool have_both = true;
        if(reduced_auto_corrs.find(ref_st) == reduced_auto_corrs.end())
        {
            have_both = false;
        }
        if(reduced_auto_corrs.find(rem_st) == reduced_auto_corrs.end())
        {
            have_both = false;
        }
        if(have_both)
        {
            auto ref_ac = reduced_auto_corrs[ref_st];
            auto rem_ac = reduced_auto_corrs[rem_st];
            for(std::size_t pp = 0; pp < npp; pp++)
            {
                //figure out the pol-mapping to the right autocorrs
                std::string polprod = std::get< POLPROD_AXIS >(*vis)(pp);
                std::string ref_polprod = std::string() + (char)polprod[0] + (char)polprod[0];
                std::string rem_polprod = std::string() + (char)polprod[1] + (char)polprod[1];
                std::size_t ref_pp_idx, rem_pp_idx;
                bool ref_ok = std::get< POLPROD_AXIS >(*ref_ac).SelectFirstMatchingIndex(ref_polprod, ref_pp_idx);
                bool rem_ok = std::get< POLPROD_AXIS >(*rem_ac).SelectFirstMatchingIndex(rem_polprod, rem_pp_idx);

                bool issue_once = true;
                if(!ref_ok || !rem_ok)
                {
                    msg_error("difx_interface", "error missing data in autocorrs needed to normalize baseline:pol-product: "
                                                    << baseline << ":" << polprod << " " << pp << "." << eom);
                    if(!ref_ok)
                    {
                        msg_error("difx_interface", "reference station: " << ref_st << " autocorr data is missing" << eom);
                    }
                    if(!rem_ok)
                    {
                        msg_error("difx_interface", "remote station: " << rem_st << " autocorr data is missing" << eom);
                    }
                }
                else
                {
                    for(std::size_t ap = 0; ap < nap; ap++)
                    {
                        for(std::size_t ch = 0; ch < nch; ch++)
                        {
                            double ref_val = std::sqrt(std::real((*ref_ac)(ref_pp_idx, ch, ap, 0)));
                            double rem_val = std::sqrt(std::real((*rem_ac)(rem_pp_idx, ch, ap, 0)));
                            double factor = 1.0;
                            if(std::fabs(ref_val) == 0.0 || std::fabs(rem_val) == 0.0)
                            {
                                if(issue_once)
                                {
                                    std::string st_code = "";
                                    if(std::fabs(ref_val) == 0.0)
                                    {
                                        st_code = ref_st;
                                    }
                                    if(std::fabs(rem_val) == 0.0)
                                    {
                                        st_code = rem_st;
                                    }
                                    msg_error("difx_interface", "zero value in auto-corrs of station: "
                                                                    << st_code << ", normalization may not be correct." << eom);
                                    issue_once = false;
                                }
                            }
                            else
                            {
                                factor = 1.0 / (ref_val * rem_val);
                            }
                            vis->SliceView(pp, ch, ap, ":") *= factor;
                        }
                    }
                }
            }
        }
        else
        {
            msg_error("difx_interface",
                      "cannot locate autocorrs for both stations of baseline: " << baseline << ", will not normalize." << eom);
        }
    }

    //finally delete the reduced auto_corr arrays
    for(auto it = reduced_auto_corrs.begin(); it != reduced_auto_corrs.end(); it++)
    {
        delete it->second;
    }
}

void MHO_DiFXScanProcessor::ConvertStationFileObjects()
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
        if(fDiFX2VexStationCodes.find(difx_station_code) != fDiFX2VexStationCodes.end())
        {
            vex_station_code = fDiFX2VexStationCodes[difx_station_code];
        }

        std::string vex_station_name = difx_station_code;
        if(fDiFX2VexStationNames.find(difx_station_code) != fDiFX2VexStationNames.end())
        {
            vex_station_name = fDiFX2VexStationNames[difx_station_code];
        }

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
            pcal_data_ptr->Insert(std::string("root_code"), fRootCode);
            pcal_data_ptr->Insert(std::string("name"), std::string("pcal"));
        }

        //add some tags to the station coord data
        station_coord_data_ptr->Insert(std::string("difx_station_code"), difx_station_code);
        station_coord_data_ptr->Insert(std::string("station_code"), vex_station_code);
        station_coord_data_ptr->Insert(std::string("station_mk4id"), station_mk4id);
        station_coord_data_ptr->Insert(std::string("station_name"), vex_station_name);
        station_coord_data_ptr->Insert(std::string("root_code"), fRootCode);
        station_coord_data_ptr->Insert(std::string("name"), std::string("station_data"));

        MHO_ObjectTags tags;
        tags.AddObjectUUID(station_coord_data_ptr->GetObjectUUID());
        if(pcal_data_ptr)
        {
            tags.AddObjectUUID(pcal_data_ptr->GetObjectUUID());
        }

        if(!fExportAsMark4)
        {
            MHO_BinaryFileInterface inter;
            //figure out the output file name
            std::string root_code = fRootCode;
            std::string output_file = ConstructStaFileName(fOutputDirectory, root_code, vex_station_code, station_mk4id);
            bool status = inter.OpenToWrite(output_file);
            if(status)
            {
                inter.Write(tags, "tags");
                inter.Write(*station_coord_data_ptr, "sta");
                if(pcal_data_ptr)
                {
                    inter.Write(*pcal_data_ptr, "pcal");
                }
                inter.Close();
            }
            else
            {
                msg_error("file", "error opening station data output file: " << output_file << eom);
            }
        }
        else
        {
            MHO_MK4StationInterfaceReversed converter;
            converter.SetVexData(fRootJSON);
            converter.SetOutputDirectory(fOutputDirectory);
            converter.SetStationCoordData(station_coord_data_ptr);
            converter.SetPCalData(pcal_data_ptr);
            converter.GenerateStationStructure();
            converter.WriteStationFile();
            converter.FreeAllocated();
        }
    }
}

void MHO_DiFXScanProcessor::CleanUp()
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
    fSrcName = "";
    fDiFXInputFilename = "";
}

void MHO_DiFXScanProcessor::LoadInputFile()
{
    //convert the input to json
    MHO_DiFXInputProcessor input_proc;
    input_proc.LoadDiFXInputFile(fFileSet->fInputFile);
    fInput.clear(); //clear all pre-existing input file data in this json object
    input_proc.ConvertToJSON(fInput);
    // input_proc.FillFrequencyTable();

    //grab the date when the fInputFile was last modified as the correlation time
    struct tm* mod_time;
    struct stat attrib;
    fCorrDate = "";
    int err = stat(fFileSet->fInputFile.c_str(), &attrib);
    if(err)
    {
        msg_debug("difx_interface", "could not stat input file: " << fFileSet->fInputFile
                                                                  << ", correlation date will be set to start of J2000 epoch."
                                                                  << eom);
        fCorrDate = hops_clock::to_vex_format(hops_clock::get_hops_epoch());
    }
    else
    {
        mod_time = gmtime(&(attrib.st_mtime));
        std::time_t timeval = std::mktime(mod_time);
        auto mod_datetime = std::chrono::system_clock::from_time_t(timeval);
        auto hops_datetime = hops_clock::from_sys(mod_datetime);
        fCorrDate = hops_clock::to_vex_format(hops_datetime);
    }

    msg_debug("difx_interface", "difx .input file: " << fFileSet->fInputFile << " converted to json." << eom);
}

void MHO_DiFXScanProcessor::ExtractPCalData()
{
    for(auto it = fFileSet->fPCALFileList.begin(); it != fFileSet->fPCALFileList.end(); it++)
    {
        msg_debug("difx_interface", "extracting phase-cal data from: " << *it << eom);
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

void MHO_DiFXScanProcessor::ExtractStationCoords()
{
    fStationCoordBuilder.SetDiFXInputData(&fInput);
    fStationCoordBuilder.SetScanIndex(fFileSet->fLocalIndex);
    fStationCoordBuilder.Extract(fStationCode2Coords);
}

std::string MHO_DiFXScanProcessor::ConstructRootFileName(const std::string& output_dir, const std::string& root_code,
                                                         const std::string& src_name)
{
    return output_dir + "/" + src_name + "." + root_code + ".root.json";
}

std::string MHO_DiFXScanProcessor::ConstructStaFileName(const std::string& output_dir, const std::string& root_code,
                                                        const std::string& station_code, const std::string& station_mk4id)
{
    return output_dir + "/" + station_mk4id + "." + station_code + "." + root_code + ".sta";
}

} // namespace hops
