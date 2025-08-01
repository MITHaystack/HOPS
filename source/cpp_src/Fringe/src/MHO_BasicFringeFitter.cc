#include "MHO_BasicFringeFitter.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//control
#include "MHO_ControlConditionEvaluator.hh"
#include "MHO_ControlFileParser.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

//fringe finding library helper functions
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_BasicFringeInfo.hh"
#include "MHO_BasicFringeUtilities.hh"
#include "MHO_FringePlotInfo.hh"
#include "MHO_InitialFringeInfo.hh"
#include "MHO_InterpolateFringePeak.hh"
#include "MHO_VexInfoExtractor.hh"

//TODO FIXME -- remove this
#include "MHO_EstimatePCManual.hh"

#ifdef HOPS_USE_CUDA
    #include "MHO_MBDelaySearchCUDA.hh"
    #define MBD_SEARCH_TYPE MHO_MBDelaySearchCUDA
#else
    #define MBD_SEARCH_TYPE MHO_MBDelaySearch
#endif

//#define DUMP_PARAMS_ON_ERROR

namespace hops
{

MHO_BasicFringeFitter::MHO_BasicFringeFitter(MHO_FringeData* data): MHO_FringeFitter(data)
{
    fEnableCaching = false;
    vis_data = nullptr;
    wt_data = nullptr;
    sbd_data = nullptr;
    fNormFXOp = nullptr; //does not need to be deleted
    fMBDSearch = new MBD_SEARCH_TYPE();

    //must build the operator build manager
    fOperatorBuildManager = new MHO_OperatorBuilderManager(&fOperatorToolbox, fFringeData, fFringeData->GetControlFormat());
};

MHO_BasicFringeFitter::~MHO_BasicFringeFitter()
{
    delete fMBDSearch;
};

void MHO_BasicFringeFitter::Configure()
{
    profiler_start();

    //load root file and keep around (eventually eliminate this in favor of param store only)
    fVexInfo = fScanStore->GetRootFileData();

    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!skipped)
    {
        std::string baseline = fParameterStore->GetAs< std::string >("/config/baseline");
        std::string polprod = fParameterStore->GetAs< std::string >("/config/polprod");

        ////////////////////////////////////////////////////////////////////////////
        //LOAD DATA AND ASSEMBLE THE DATA STORE
        ////////////////////////////////////////////////////////////////////////////

        //load baseline data
        fScanStore->LoadBaseline(baseline, fContainerStore);
        fParameterStore->Set("/files/baseline_input_file", fScanStore->GetBaselineFilename(baseline));

        //loads visibility data and performs float -> double cast
        MHO_BasicFringeDataConfiguration::configure_visibility_data(fContainerStore);

        vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));
        if(vis_data == nullptr || wt_data == nullptr)
        {
            msg_error("fringe", "could not find visibility or weight objects with names (vis, weight)." << eom);
            fParameterStore->Set("/status/skipped", true); //skipped
            return;
        }

        //safety check
        if(vis_data->GetSize() == 0)
        {
            msg_error("fringe", "visibility data has size 0, skipping." << eom);
            fParameterStore->Set("/status/skipped", true);
            fParameterStore->Set("/status/is_finished", true);
        }

        if(wt_data->GetSize() == 0)
        {
            msg_error("fringe", "weight data has size 0, skipping." << eom);
            fParameterStore->Set("/status/skipped", true);
            fParameterStore->Set("/status/is_finished", true);
        }

        std::string vis_uuid = vis_data->GetObjectUUID().as_string();
        std::string wt_uuid = wt_data->GetObjectUUID().as_string();
        fParameterStore->Set("/uuid/visibilities", vis_uuid);
        fParameterStore->Set("/uuid/weights", wt_uuid);

        //load and rename station data according to reference/remote
        //also load pcal data if it is present
        std::string ref_station_mk4id = std::string(1, baseline[0]);
        std::string rem_station_mk4id = std::string(1, baseline[1]);
        MHO_BasicFringeDataConfiguration::configure_station_data(fScanStore, fContainerStore, ref_station_mk4id,
                                                                 rem_station_mk4id);
        fParameterStore->Set("/files/ref_station_input_file", fScanStore->GetStationFilename(ref_station_mk4id));
        fParameterStore->Set("/files/rem_station_input_file", fScanStore->GetStationFilename(rem_station_mk4id));

        station_coord_type* ref_data = fContainerStore->GetObject< station_coord_type >(std::string("ref_sta"));
        station_coord_type* rem_data = fContainerStore->GetObject< station_coord_type >(std::string("rem_sta"));
        if(ref_data == nullptr || rem_data == nullptr)
        {
            msg_error("fringe", "could not find station coordinate data with names (ref_sta, rem_sta)." << eom);
            fParameterStore->Set("/status/skipped", true); //skipped
            return;
        }
        std::string ref_uuid = ref_data->GetObjectUUID().as_string();
        std::string rem_uuid = rem_data->GetObjectUUID().as_string();
        fParameterStore->Set("/uuid/ref_coord", ref_uuid);
        fParameterStore->Set("/uuid/rem_coord", rem_uuid);

        multitone_pcal_type* ref_pcal_data = fContainerStore->GetObject< multitone_pcal_type >(std::string("ref_pcal"));
        multitone_pcal_type* rem_pcal_data = fContainerStore->GetObject< multitone_pcal_type >(std::string("rem_pcal"));
        if(ref_pcal_data != nullptr)
        {
            std::string ref_pcal_uuid = ref_pcal_data->GetObjectUUID().as_string();
            fParameterStore->Set("/uuid/ref_pcal", ref_pcal_uuid);
        }
        if(rem_pcal_data != nullptr)
        {
            std::string rem_pcal_uuid = rem_pcal_data->GetObjectUUID().as_string();
            fParameterStore->Set("/uuid/rem_pcal", rem_pcal_uuid);
        }

        ////////////////////////////////////////////////////////////////////////////
        //PARAMETER SETTING
        ////////////////////////////////////////////////////////////////////////////
        MHO_InitialFringeInfo::configure_reference_frequency(fContainerStore, fParameterStore);

        ////////////////////////////////////////////////////////////////////////////
        //CONFIGURE THE OPERATOR BUILD MANAGER
        ////////////////////////////////////////////////////////////////////////////
        fOperatorBuildManager->CreateDefaultBuilders();
        fOperatorBuildManager->SetControlStatements(&(fFringeData->GetControlStatements()));
        fOperatorBuildManager->BuildOperatorCategory("default");

        //take a snapshot if enabled
        take_snapshot_here("test", "visib", __FILE__, __LINE__, vis_data);
        take_snapshot_here("test", "weights", __FILE__, __LINE__, wt_data);
        
        ////////////////////////////////////////////////////////////////////////////
        //OPERATOR CONSTRUCTION
        ////////////////////////////////////////////////////////////////////////////
        fOperatorBuildManager->BuildOperatorCategory("labeling");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "labeling");
        fOperatorBuildManager->BuildOperatorCategory("selection");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "selection");

        //safety check
        if(vis_data->GetSize() == 0)
        {
            msg_warn("fringe", "no visibility data left after cuts, skipping." << eom);
            fParameterStore->Set("/status/skipped", true);
            fParameterStore->Set("/status/is_finished", true);
        }

        if(wt_data->GetSize() == 0)
        {
            msg_warn("fringe", "no weight data left after cuts, skipping." << eom);
            fParameterStore->Set("/status/skipped", true);
            fParameterStore->Set("/status/is_finished", true);
        }
        
        //calculate useful quantities to stash in the parameter store
        MHO_InitialFringeInfo::precalculate_quantities(fContainerStore, fParameterStore);
        

        
        //build the rest of the operator categories
        fOperatorBuildManager->BuildOperatorCategory("flagging");
        fOperatorBuildManager->BuildOperatorCategory("calibration");
        fOperatorBuildManager->BuildOperatorCategory("prefit");
        fOperatorBuildManager->BuildOperatorCategory("postfit");
        fOperatorBuildManager->BuildOperatorCategory("finalize");


        //if we have any additional prefit and postfit operators there is a possibility 
        //that more than one fitting loop is run, in that case we will
        //cache the configured visibilities and weights
        if(fOperatorToolbox.GetNOperatorsInCategory("prefit") > 0 && 
           fOperatorToolbox.GetNOperatorsInCategory("postfit") > 0)
        {
            msg_debug("fringe", "enabling visibility/weight caching due to presence of prefit/postfit operators (" <<
                fOperatorToolbox.GetNOperatorsInCategory("prefit") << ", " <<
                fOperatorToolbox.GetNOperatorsInCategory("postfit") << ")" << eom);
            fEnableCaching = true;
        }

        Cache();
        

    }

    profiler_stop();
}

void MHO_BasicFringeFitter::Cache()
{
    if(fEnableCaching)
    {
        vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));
        if(vis_data != nullptr)
        {
            auto cached_vis_data = vis_data->Clone();
            msg_debug("fringe", "caching visibility data to object: " << cached_vis_data->GetObjectUUID().as_string() << eom);
            fContainerStore->AddObject(cached_vis_data);
            std::string shortname = "cached_v";
            fContainerStore->SetShortName(cached_vis_data->GetObjectUUID(), shortname);
        }

        if(wt_data != nullptr)
        {
            auto cached_wt_data = wt_data->Clone();
            msg_debug("fringe", "caching weight data to object: " << cached_wt_data->GetObjectUUID().as_string() << eom);
            fContainerStore->AddObject(cached_wt_data);
            std::string shortname = "cached_w";
            fContainerStore->SetShortName(cached_wt_data->GetObjectUUID(), shortname);
        }
    }
};

void MHO_BasicFringeFitter::Refresh()
{
    if(fEnableCaching)
    {
        auto vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        auto wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));
        auto cached_vis_data = fContainerStore->GetObject< visibility_type >(std::string("cached_v"));
        auto cached_wt_data = fContainerStore->GetObject< weight_type >(std::string("cached_w"));
        if(vis_data != nullptr && cached_vis_data != nullptr)
        {
            //deep copy
            msg_debug("fringe", "refreshing visibility data from cache" << eom);
            vis_data->Copy(*cached_vis_data);
        }

        if(wt_data != nullptr && cached_wt_data != nullptr)
        {
            //deep copy
            msg_debug("fringe", "refreshing weight data from cache" << eom);
            wt_data->Copy(*cached_wt_data);
        }
    }
};

void MHO_BasicFringeFitter::Initialize()
{
    profiler_start();
    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!skipped)
    {
        //refresh the visibility/weight data from the cache
        //this mechanism is necessary if we want to be able to provide that ability for a user-specified
        //outer layer of iteration, where they are able to modify operator parameters
        //until some convergence criteria is met
        Refresh();

        //compute the sum of all weights and stash in the parameter store
        //(before any other operations (e.g. passband, notches) modify them)
        MHO_InitialFringeInfo::compute_total_summed_weights(fContainerStore, fParameterStore);
        //figure out the number of channels which have data with weights >0 in at least 1 AP
        MHO_InitialFringeInfo::determine_n_active_channels(fContainerStore, fParameterStore);

        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "flagging");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "calibration");
    }

    // std::cout<<"PARAMETERS = "<<std::endl;
    // fParameterStore->Dump();
    profiler_stop();
}

void MHO_BasicFringeFitter::PreRun()
{
    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!skipped) //execute if we are not finished and are not skipping
    {
        //create space for the visibilities transformed into single-band-delay space
        sbd_data = fContainerStore->GetObject< visibility_type >(std::string("sbd"));
        if(sbd_data == nullptr) //doesn't yet exist so create and cache it in the store
        {
            sbd_data = new sbd_type();
            fContainerStore->AddObject(sbd_data);
            fContainerStore->SetShortName(sbd_data->GetObjectUUID(), std::string("sbd"));
        }

        //user specified python scripts that are in the 'prefit' category are run here
        //as well as the 'pol-product' summation operator (which is applied last if applicable)
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "prefit");

        //initialize the fringe search operators ///////////////////////////////
        //determine the type of NormFX operator we need (either mixed sideband or single-sideband)
        if(ContainsMixedSideband(vis_data))
        {
            fNormFXOp = &fMSBNormFXOp;
        }
        else
        {
            fNormFXOp = &fSSBNormFXOp;
        }

        //initialize norm-fx (x-form to SBD space)
        fNormFXOp->SetArgs(vis_data, sbd_data);
        fNormFXOp->SetWeights(wt_data);
        bool ok = fNormFXOp->Initialize(); //initialize takes care of properly re-sizing SBD data
        check_step_fatal(ok, "fringe", "normfx initialization." << eom);

        //configure the coarse SBD/DR/MBD search
        double ref_freq = fParameterStore->GetAs< double >("/control/config/ref_freq");
        fMBDSearch->SetWeights(wt_data);
        fMBDSearch->SetReferenceFrequency(ref_freq);
        fMBDSearch->SetArgs(sbd_data);
        ok = fMBDSearch->Initialize();
        check_step_fatal(ok, "fringe", "mbd initialization." << eom);

        //configure the fringe-peak interpolator
        bool optimize_closure_flag = false;
        bool is_oc_set = fParameterStore->Get(std::string("/control/fit/optimize_closure"), optimize_closure_flag);
        double frt_offset = fParameterStore->GetAs< double >("/config/frt_offset");
        //NOTE: the optimize_closure_flag has no effect on fringe-phase when
        //using the 'simul' algorithm, which is currently the only one implemented
        //This is also true of the legacy code 'simul' implementation.
        if(optimize_closure_flag)
        {
            fPeakInterpolator.EnableOptimizeClosure();
        }
        fPeakInterpolator.SetReferenceFrequency(ref_freq);
        fPeakInterpolator.SetReferenceTimeOffset(frt_offset);
        fPeakInterpolator.SetSBDArray(sbd_data);
        fPeakInterpolator.SetWeights(wt_data);
    }
}

void MHO_BasicFringeFitter::Run()
{
    profiler_start();
    bool is_finished = fParameterStore->GetAs< bool >("/status/is_finished");
    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!is_finished && !skipped) //execute if we are not finished and are not skipping
    {
        //execute the coarse fringe search algorithm
        coarse_fringe_search();
    }

    //check again since, if there is an error during the fringe search we should skip this pass
    is_finished = fParameterStore->GetAs< bool >("/status/is_finished");
    skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!is_finished && !skipped) //execute if we are not finished and are not skipping
    {
        interpolate_peak();
        // MHO_BasicFringeUtilities::basic_fringe_search(fContainerStore, fParameterStore);
        fParameterStore->Set("/status/is_finished", true);
        //have sampled all grid points, find the solution and finalize
        //calculate the fringe properties
        MHO_BasicFringeUtilities::calculate_fringe_solution_info(fContainerStore, fParameterStore, fVexInfo);
    }
    profiler_stop();
}

void MHO_BasicFringeFitter::PostRun()
{
    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!skipped) //execute if we are not finished and are not skipping
    {
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "postfit");
    }
}

bool MHO_BasicFringeFitter::IsFinished()
{
    bool is_finished = fParameterStore->GetAs< bool >("/status/is_finished");
    return is_finished;
}

void MHO_BasicFringeFitter::Finalize()
{
    profiler_start();
    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO may want to reorg the way this is done

    bool status_is_finished = fParameterStore->GetAs< bool >("/status/is_finished");
    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(status_is_finished && !skipped) //have to be finished and not-skipped
    {
        //get the actual search windows that were used
        double low, high;
        std::vector< double > win;
        win.resize(2);
        fMBDSearch->GetSBDWindow(low, high);
        win[0] = low;
        win[1] = high;
        fParameterStore->Set("/fringe/sb_win", win);

        fMBDSearch->GetDRWindow(low, high);
        win[0] = low;
        win[1] = high;
        fParameterStore->Set("/fringe/dr_win", win);

        fMBDSearch->GetMBDWindow(low, high);
        win[0] = low;
        win[1] = high;
        fParameterStore->Set("/fringe/mb_win", win);

        mho_json& plot_data = fFringeData->GetPlotData();
        plot_data = MHO_FringePlotInfo::construct_plot_data(fContainerStore, fParameterStore, &fOperatorToolbox, fVexInfo);
        MHO_FringePlotInfo::fill_plot_data(fParameterStore, plot_data);

        //TODO FIXME...remove this (we should build this as an operator in the 'finalize category'), just for testing
        MHO_EstimatePCManual est_pc_man;
        auto vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        auto wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));
        auto phasor_data = fContainerStore->GetObject< phasor_type >(std::string("phasors"));
        est_pc_man.SetArgs(vis_data);
        est_pc_man.SetWeights(wt_data);
        est_pc_man.SetPlotData(plot_data);
        est_pc_man.SetParameterStore(fParameterStore);
        est_pc_man.SetPhasors(phasor_data);
        est_pc_man.Initialize();
        est_pc_man.Execute();

        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "finalize");
    }

    profiler_stop();
}

void MHO_BasicFringeFitter::coarse_fringe_search(bool set_windows)
{
    profiler_start();
    ////////////////////////////////////////////////////////////////////////////
    //COARSE SBD, DR, MBD SEARCH ALGO
    ////////////////////////////////////////////////////////////////////////////
    sbd_data->ZeroArray();

    //run norm_fx (takes visibilities to (single band) delay space)
    bool ok = fNormFXOp->Execute();
    check_step_fatal(ok, "fringe", "normfx execution." << eom);

    //take snapshot of sbd data after normfx
    take_snapshot_here("test", "sbd", __FILE__, __LINE__, sbd_data);

    //set the coarse SBD/MBD/DR search windows here
    if(fParameterStore->IsPresent("/control/fit/sb_win") && set_windows)
    {
        std::vector< double > sbwin = fParameterStore->GetAs< std::vector< double > >("/control/fit/sb_win");
        fMBDSearch->SetSBDWindow(sbwin[0], sbwin[1]); //units are microsec
    }

    if(fParameterStore->IsPresent("/control/fit/mb_win") && set_windows)
    {
        std::vector< double > mbwin = fParameterStore->GetAs< std::vector< double > >("/control/fit/mb_win");
        fMBDSearch->SetMBDWindow(mbwin[0], mbwin[1]); //units are microsec
    }

    if(fParameterStore->IsPresent("/control/fit/dr_win") && set_windows)
    {
        std::vector< double > drwin = fParameterStore->GetAs< std::vector< double > >("/control/fit/dr_win");
        fMBDSearch->SetDRWindow(drwin[0], drwin[1]);
    }

    ok = fMBDSearch->Execute();

    check_step_fatal(ok, "fringe", "mbd execution." << eom);

    int n_mbd_pts = fMBDSearch->GetNMBDBins();
    int n_dr_pts = fMBDSearch->GetNDRBins();
    int n_sbd_pts = fMBDSearch->GetNSBDBins();
    int n_drsp_pts = fMBDSearch->GetNDRSPBins();
    double n_pts_searched = fMBDSearch->GetNPointsSearched();

    fParameterStore->Set("/fringe/n_mbd_points", n_mbd_pts);
    fParameterStore->Set("/fringe/n_sbd_points", n_sbd_pts);
    fParameterStore->Set("/fringe/n_dr_points", n_dr_pts);
    fParameterStore->Set("/fringe/n_drsp_points", n_drsp_pts);
    fParameterStore->Set("/fringe/n_pts_searched", n_pts_searched);

    int c_mbdmax = fMBDSearch->GetMBDMaxBin();
    int c_sbdmax = fMBDSearch->GetSBDMaxBin();
    int c_drmax = fMBDSearch->GetDRMaxBin();
    double freq_spacing = fMBDSearch->GetFrequencySpacing();
    double ave_freq = fMBDSearch->GetAverageFrequency();

    if(c_mbdmax < 0 || c_sbdmax < 0 || c_drmax < 0)
    {
        msg_fatal("fringe", "coarse fringe search could not locate peak, bin (sbd, mbd, dr) = ("
                                << c_sbdmax << ", " << c_mbdmax << "," << c_drmax << "), skipping this pass" << eom);
#ifdef HOPS_ENABLE_DEBUG_MSG
    #ifdef DUMP_PARAMS_ON_ERROR
        msg_fatal("fringe", "dumping parameter store for debugging" << eom);
        fParameterStore->Dump();
    #endif
#endif
        fParameterStore->Set("/status/skipped", true);
        fParameterStore->Set("/status/is_finished", true);
    }
    else
    {
        //get the coarse maximum and re-scale by the total weights
        double search_max_amp = fMBDSearch->GetSearchMaximumAmplitude();
        double total_summed_weights = fParameterStore->GetAs< double >("/fringe/total_summed_weights");

        fParameterStore->Set("/fringe/coarse_search_max_amp", search_max_amp / total_summed_weights);
        fParameterStore->Set("/fringe/max_mbd_bin", c_mbdmax);
        fParameterStore->Set("/fringe/max_sbd_bin", c_sbdmax);
        fParameterStore->Set("/fringe/max_dr_bin", c_drmax);

        double coarse_sbdelay = fMBDSearch->GetCoarseSBD();
        fParameterStore->Set("/fringe/sbdelay", coarse_sbdelay);
    }

    profiler_stop();
}

void MHO_BasicFringeFitter::interpolate_peak()
{
    profiler_start();
    ////////////////////////////////////////////////////////////////////////////
    //FINE INTERPOLATION STEP (search over 5x5x5 grid around peak)
    ////////////////////////////////////////////////////////////////////////////
    int c_mbdmax = fParameterStore->GetAs< int >("/fringe/max_mbd_bin");
    int c_sbdmax = fParameterStore->GetAs< int >("/fringe/max_sbd_bin");
    int c_drmax = fParameterStore->GetAs< int >("/fringe/max_dr_bin");

    fPeakInterpolator.SetMaxBins(c_sbdmax, c_mbdmax, c_drmax);
    fPeakInterpolator.SetMBDAxis(fMBDSearch->GetMBDAxis());
    fPeakInterpolator.SetDRAxis(fMBDSearch->GetDRAxis());

    fPeakInterpolator.Initialize();
    fPeakInterpolator.Execute();

    double sbdelay = fPeakInterpolator.GetSBDelay();
    double mbdelay = fPeakInterpolator.GetMBDelay();
    double drate = fPeakInterpolator.GetDelayRate();
    double frate = fPeakInterpolator.GetFringeRate();
    double famp = fPeakInterpolator.GetFringeAmplitude();

    //if there is only one channel, original/default fourfit behavior is to set MBD = SBD
    int n_active = fParameterStore->GetAs< int >("/fringe/n_active_channels");
    if(n_active == 1)
    {
        msg_info("fringe", "only one active data channel, setting MBD(" << mbdelay << ") to SBD(" << sbdelay << ")" << eom);
        //see original fourfit code: interp.c, line 238
        mbdelay = sbdelay;
    }

    fParameterStore->Set("/fringe/sbdelay", sbdelay);
    fParameterStore->Set("/fringe/mbdelay", mbdelay);
    fParameterStore->Set("/fringe/drate", drate);
    fParameterStore->Set("/fringe/frate", frate);
    fParameterStore->Set("/fringe/famp", famp);

    profiler_stop();
}

bool MHO_BasicFringeFitter::ContainsMixedSideband(visibility_type* vis)
{
    //figure out if we have USB or LSB data, or a mixture, or double-sideband data
    auto channel_axis = &(std::get< CHANNEL_AXIS >(*(vis)));

    //first check for double-sideband channels
    std::vector< mho_json > dsb_labels = channel_axis->GetMatchingIntervalLabels("double_sideband");
    std::size_t n_dsb_chan = dsb_labels.size();
    if(n_dsb_chan != 0)
    {
        return true;
    }

    //now check for the number of other sidebands
    std::string sb_key = "net_sideband";
    std::string usb_flag = "U";
    std::string lsb_flag = "L";
    auto usb_chan = channel_axis->GetMatchingIndexes(sb_key, usb_flag);
    auto lsb_chan = channel_axis->GetMatchingIndexes(sb_key, lsb_flag);
    std::size_t n_usb_chan = usb_chan.size();
    std::size_t n_lsb_chan = lsb_chan.size();

    //mixed sideband data should be ok, but warn user since it is not well tested
    if(n_usb_chan != 0 && n_lsb_chan != 0)
    {
        return true; //mixed set of channels with USB or LSB
    }

    if(n_lsb_chan != 0 && n_usb_chan == 0)
    {
        return false; //single sideband (LSB)
    }

    if(n_usb_chan != 0 && n_lsb_chan == 0)
    {
        return false; //single side band (USB)
    }

    msg_warn("fringe", "could not determine type of sidebands present in visibility data" << eom);
    return false;
}

} // namespace hops
