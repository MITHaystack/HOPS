#include "MHO_BasicFringeFitter.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//control
#include "MHO_ControlFileParser.hh"
#include "MHO_ControlConditionEvaluator.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

//fringe finding library helper functions
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_BasicFringeInfo.hh"
#include "MHO_InitialFringeInfo.hh"
#include "MHO_BasicFringeUtilities.hh"
#include "MHO_FringePlotInfo.hh"
#include "MHO_VexInfoExtractor.hh"
#include "MHO_InterpolateFringePeak.hh"

namespace hops
{

MHO_BasicFringeFitter::MHO_BasicFringeFitter(MHO_FringeData* data):MHO_FringeFitter(data)
{
    vis_data = nullptr;
    wt_data = nullptr;
    sbd_data = nullptr;
};

MHO_BasicFringeFitter::~MHO_BasicFringeFitter(){};

void MHO_BasicFringeFitter::Configure()
{
    profiler_start();

    //load root file and keep around (eventually eliminate this in favor of param store only)
    fVexInfo = fScanStore->GetRootFileData();

    //now build the operator build manager
    fOperatorBuildManager = new MHO_OperatorBuilderManager(&fOperatorToolbox, fContainerStore, fParameterStore, fFringeData->GetControlFormat() );

    profiler_stop();
}

void MHO_BasicFringeFitter::Initialize()
{
    profiler_start();
    bool skipped = fParameterStore->GetAs<bool>("/status/skipped");
    if( !skipped )
    {
        std::string baseline = fParameterStore->GetAs<std::string>("/config/baseline");
        std::string polprod = fParameterStore->GetAs<std::string>("/config/polprod");

        ////////////////////////////////////////////////////////////////////////////
        //LOAD DATA AND ASSEMBLE THE DATA STORE
        ////////////////////////////////////////////////////////////////////////////

        //load baseline data
        fScanStore->LoadBaseline(baseline, fContainerStore);
        fParameterStore->Set("/files/baseline_input_file", fScanStore->GetBaselineFilename(baseline));

        //loads visibility data and performs float -> double cast
        MHO_BasicFringeDataConfiguration::configure_visibility_data(fContainerStore);

        vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
        wt_data = fContainerStore->GetObject<weight_type>(std::string("weight"));
        if( vis_data == nullptr || wt_data == nullptr )
        {
            msg_fatal("fringe", "could not find visibility or weight objects with names (vis, weight)." << eom);
            std::exit(1);
        }

        //safety check
        if(vis_data->GetSize() == 0){msg_fatal("fringe", "visibility data has size zero." << eom); std::exit(1);}
        if(wt_data->GetSize() == 0){msg_fatal("fringe", "weight data has size zero." << eom); std::exit(1);}

        std::string vis_uuid = vis_data->GetObjectUUID().as_string();
        std::string wt_uuid = wt_data->GetObjectUUID().as_string();
        fParameterStore->Set("/uuid/visibilities", vis_uuid);
        fParameterStore->Set("/uuid/weights", wt_uuid);

        //load and rename station data according to reference/remote
        //also load pcal data if it is present
        std::string ref_station_mk4id = std::string(1,baseline[0]);
        std::string rem_station_mk4id = std::string(1,baseline[1]);
        MHO_BasicFringeDataConfiguration::configure_station_data(fScanStore, fContainerStore, ref_station_mk4id, rem_station_mk4id);
        fParameterStore->Set("/files/ref_station_input_file", fScanStore->GetStationFilename(ref_station_mk4id));
        fParameterStore->Set("/files/rem_station_input_file", fScanStore->GetStationFilename(rem_station_mk4id));

        station_coord_type* ref_data = fContainerStore->GetObject<station_coord_type>(std::string("ref_sta"));
        station_coord_type* rem_data = fContainerStore->GetObject<station_coord_type>(std::string("rem_sta"));
        if( ref_data == nullptr || rem_data == nullptr )
        {
            msg_fatal("fringe", "could not find station coordinate data with names (ref_sta, rem_sta)." << eom);
            std::exit(1);
        }
        std::string ref_uuid = ref_data->GetObjectUUID().as_string();
        std::string rem_uuid = rem_data->GetObjectUUID().as_string();
        fParameterStore->Set("/uuid/ref_coord", ref_uuid);
        fParameterStore->Set("/uuid/rem_coord", rem_uuid);

        multitone_pcal_type* ref_pcal_data = fContainerStore->GetObject<multitone_pcal_type>(std::string("ref_pcal"));
        multitone_pcal_type* rem_pcal_data = fContainerStore->GetObject<multitone_pcal_type>(std::string("rem_pcal"));
        if( ref_pcal_data != nullptr)
        {
            std::string ref_pcal_uuid = ref_pcal_data->GetObjectUUID().as_string();
            fParameterStore->Set("/uuid/ref_pcal", ref_pcal_uuid);
        }
        if( rem_pcal_data != nullptr )
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
        fOperatorBuildManager->SetControlStatements( &(fFringeData->GetControlStatements()) );

        //take a snapshot if enabled
        take_snapshot_here("test", "visib", __FILE__, __LINE__, vis_data);
        take_snapshot_here("test", "weights", __FILE__, __LINE__,  wt_data);

        ////////////////////////////////////////////////////////////////////////////
        //OPERATOR CONSTRUCTION
        ////////////////////////////////////////////////////////////////////////////

        fOperatorBuildManager->BuildOperatorCategory("default");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "labeling");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "selection");

        //calculate useful quantities to stash in the parameter store
        MHO_InitialFringeInfo::precalculate_quantities(fContainerStore, fParameterStore);
        //safety check
        if(vis_data->GetSize() == 0){msg_fatal("fringe", "no visibility data left after cuts." << eom); std::exit(1);}
        if(wt_data->GetSize() == 0){msg_fatal("fringe", "no weight data left after cuts." << eom); std::exit(1);}

        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "flagging");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "calibration");

        //compute the sum of all weights and stash in the parameter store
        MHO_InitialFringeInfo::compute_total_summed_weights(fContainerStore, fParameterStore);

        //initialize the fringe search operators ///////////////////////////////
        //create space for the visibilities transformed into single-band-delay space
        std::size_t bl_dim[visibility_type::rank::value];
        vis_data->GetDimensions(bl_dim);
        sbd_data = fContainerStore->GetObject<visibility_type>(std::string("sbd"));
        if(sbd_data == nullptr) //doesn't yet exist so create and cache it in the store
        {
            sbd_data = vis_data->Clone();
            fContainerStore->AddObject(sbd_data);
            fContainerStore->SetShortName(sbd_data->GetObjectUUID(), std::string("sbd"));
            bl_dim[FREQ_AXIS] *= 4; //normfx implementation demands this
            sbd_data->Resize(bl_dim);
            sbd_data->ZeroArray();
        }

        //initialize norm-fx (x-form to SBD space)
        fNormFXOp.SetArgs(vis_data, wt_data, sbd_data);
        bool ok = fNormFXOp.Initialize();
        check_step_fatal(ok, "fringe", "normfx initialization." << eom );

        //configure the coarse SBD/DR/MBD search
        double ref_freq = fParameterStore->GetAs<double>("/control/config/ref_freq");
        fMBDSearch.SetWeights(wt_data);
        fMBDSearch.SetReferenceFrequency(ref_freq);
        fMBDSearch.SetArgs(sbd_data);
        ok = fMBDSearch.Initialize();
        check_step_fatal(ok, "fringe", "mbd initialization." << eom );

        //configure the fringe-peak interpolator
        bool optimize_closure_flag = false;
        bool is_oc_set = fParameterStore->Get(std::string("/control/fit/optimize_closure"), optimize_closure_flag );
        double frt_offset = fParameterStore->GetAs<double>("/config/frt_offset");
        //NOTE: the optimize_closure_flag has no effect on fringe-phase when
        //using the 'simul' algorithm, which is currently the only one implemented
        //This is also true of the legacy code 'simul' implementation.
        if(optimize_closure_flag){fPeakInterpolator.EnableOptimizeClosure();}
        fPeakInterpolator.SetReferenceFrequency(ref_freq);
        fPeakInterpolator.SetReferenceTimeOffset(frt_offset);
        fPeakInterpolator.SetSBDArray(sbd_data);
        fPeakInterpolator.SetWeights(wt_data);

    }

    // std::cout<<"PARAMETERS = "<<std::endl;
    // fParameterStore->Dump();
    profiler_stop();
}

void MHO_BasicFringeFitter::PreRun()
{
    bool skipped = fParameterStore->GetAs<bool>("/status/skipped");
    if( !skipped) //execute if we are not finished and are not skipping
    {
        //TODO FILL ME IN -- need to call specified user-scripts here
    }
}

void MHO_BasicFringeFitter::Run()
{
    profiler_start();
    bool is_finished = fParameterStore->GetAs<bool>("/status/is_finished");
    bool skipped = fParameterStore->GetAs<bool>("/status/skipped");
    if( !is_finished  && !skipped) //execute if we are not finished and are not skipping
    {
        //execute the basic fringe search algorithm
        //basic_fringe_search();
        coarse_fringe_search();
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
    bool skipped = fParameterStore->GetAs<bool>("/status/skipped");
    if( !skipped) //execute if we are not finished and are not skipping
    {
        //TODO FILL ME IN -- need to call specified user-scripts here
    }
}


bool MHO_BasicFringeFitter::IsFinished()
{
    bool is_finished = fParameterStore->GetAs<bool>("/status/is_finished");
    return is_finished;
}


void MHO_BasicFringeFitter::Finalize()
{
    profiler_start();
    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO may want to reorg the way this is done

    bool status_is_finished = fParameterStore->GetAs<bool>("/status/is_finished");
    bool skipped = fParameterStore->GetAs<bool>("/status/skipped");
    if( status_is_finished  && !skipped ) //have to be finished and not-skipped
    {
        //get the actual search windows that were used
        double low, high;
        std::vector< double > win; win.resize(2);
        fMBDSearch.GetSBDWindow(low,high);
        win[0] = low; win[1] = high;
        fParameterStore->Set("/fringe/sb_win", win);

        fMBDSearch.GetDRWindow(low,high);
        win[0] = low; win[1] = high;
        fParameterStore->Set("/fringe/dr_win", win);

        fMBDSearch.GetMBDWindow(low,high);
        win[0] = low; win[1] = high;
        fParameterStore->Set("/fringe/mb_win", win);

        mho_json& plot_data = fFringeData->GetPlotData();
        plot_data = MHO_FringePlotInfo::construct_plot_data(fContainerStore, fParameterStore, fVexInfo);
        MHO_FringePlotInfo::fill_plot_data(fParameterStore, plot_data);
    }

    profiler_stop();
}


void
MHO_BasicFringeFitter::coarse_fringe_search()
{
    profiler_start();
    ////////////////////////////////////////////////////////////////////////////
    //COARSE SBD, DR, MBD SEARCH ALGO
    ////////////////////////////////////////////////////////////////////////////
    sbd_data->ZeroArray();

    //run norm_fx (takes visibilities to (single band) delay space)
    bool ok = fNormFXOp.Execute();
    check_step_fatal(ok, "fringe", "normfx execution." << eom );

    //take snapshot of sbd data after normfx
    take_snapshot_here("test", "sbd", __FILE__, __LINE__, sbd_data);

    //set the coarse SBD/MBD/DR search windows here
    if(fParameterStore->IsPresent("/control/fit/sb_win"))
    {
        std::vector<double> sbwin = fParameterStore->GetAs< std::vector<double> >("/control/fit/sb_win");
        fMBDSearch.SetSBDWindow(sbwin[0], sbwin[1]); //units are microsec
    }

    if(fParameterStore->IsPresent("/control/fit/mb_win"))
    {
        std::vector<double> mbwin = fParameterStore->GetAs< std::vector<double> >("/control/fit/mb_win");
        fMBDSearch.SetMBDWindow(mbwin[0], mbwin[1]); //units are microsec
    }

    if(fParameterStore->IsPresent("/control/fit/dr_win"))
    {
        std::vector<double> drwin = fParameterStore->GetAs< std::vector<double> >("/control/fit/dr_win");
        fMBDSearch.SetDRWindow(drwin[0], drwin[1]);
    }

    ok = fMBDSearch.Execute();

    check_step_fatal(ok, "fringe", "mbd execution." << eom );

    int n_mbd_pts = fMBDSearch.GetNMBDBins();
    int n_dr_pts = fMBDSearch.GetNDRBins();
    int n_sbd_pts = fMBDSearch.GetNSBDBins();
    int n_drsp_pts = fMBDSearch.GetNDRSPBins();

    fParameterStore->Set("/fringe/n_mbd_points", n_mbd_pts);
    fParameterStore->Set("/fringe/n_sbd_points", n_sbd_pts);
    fParameterStore->Set("/fringe/n_dr_points", n_dr_pts);
    fParameterStore->Set("/fringe/n_drsp_points", n_drsp_pts);

    int c_mbdmax = fMBDSearch.GetMBDMaxBin();
    int c_sbdmax = fMBDSearch.GetSBDMaxBin();
    int c_drmax = fMBDSearch.GetDRMaxBin();
    double freq_spacing = fMBDSearch.GetFrequencySpacing();
    double ave_freq = fMBDSearch.GetAverageFrequency();

    if(c_mbdmax < 0 || c_sbdmax < 0 || c_drmax < 0)
    {
        fParameterStore->Dump();
        msg_fatal("fringe", "coarse fringe search could not locate peak, bin (sbd, mbd, dr) = (" <<c_sbdmax << ", " << c_mbdmax <<"," << c_drmax<< ")." << eom );
        std::exit(1);
    }

    //get the coarse maximum and re-scale by the total weights
    double search_max_amp = fMBDSearch.GetSearchMaximumAmplitude();
    double total_summed_weights = fParameterStore->GetAs<double>("/fringe/total_summed_weights");

    fParameterStore->Set("/fringe/coarse_search_max_amp", search_max_amp/total_summed_weights);
    fParameterStore->Set("/fringe/max_mbd_bin", c_mbdmax);
    fParameterStore->Set("/fringe/max_sbd_bin", c_sbdmax);
    fParameterStore->Set("/fringe/max_dr_bin", c_drmax);

    double coarse_sbdelay = fMBDSearch.GetCoarseSBD();
    fParameterStore->Set("/fringe/sbdelay", coarse_sbdelay);

    profiler_stop();
}

void
MHO_BasicFringeFitter::interpolate_peak()
{
    profiler_start();
    ////////////////////////////////////////////////////////////////////////////
    //FINE INTERPOLATION STEP (search over 5x5x5 grid around peak)
    ////////////////////////////////////////////////////////////////////////////
    int c_mbdmax = fParameterStore->GetAs<int>("/fringe/max_mbd_bin");
    int c_sbdmax = fParameterStore->GetAs<int>("/fringe/max_sbd_bin");
    int c_drmax = fParameterStore->GetAs<int>("/fringe/max_dr_bin");

    fPeakInterpolator.SetMaxBins(c_sbdmax, c_mbdmax, c_drmax);
    #pragma message("TODO FIXME -- we shouldn't be referencing internal members of the MHO_MBDelaySearch class workspace")

    //TODO FIXME: Figure out how best to present this axis data to the fine-interp function.
    fPeakInterpolator.SetMBDAxis( fMBDSearch.GetMBDAxis() );
    fPeakInterpolator.SetDRAxis( fMBDSearch.GetDRAxis() );

    fPeakInterpolator.Initialize();
    fPeakInterpolator.Execute();

    double sbdelay = fPeakInterpolator.GetSBDelay();
    double mbdelay = fPeakInterpolator.GetMBDelay();
    double drate = fPeakInterpolator.GetDelayRate();
    double frate = fPeakInterpolator.GetFringeRate();
    double famp = fPeakInterpolator.GetFringeAmplitude();

    fParameterStore->Set("/fringe/sbdelay", sbdelay);
    fParameterStore->Set("/fringe/mbdelay", mbdelay);
    fParameterStore->Set("/fringe/drate", drate);
    fParameterStore->Set("/fringe/frate", frate);
    fParameterStore->Set("/fringe/famp", famp);

    profiler_stop();
}


}//end namespace
