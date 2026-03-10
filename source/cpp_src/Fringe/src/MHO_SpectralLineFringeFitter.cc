#include "MHO_SpectralLineFringeFitter.hh"

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
#include "MHO_VexInfoExtractor.hh"

namespace hops
{

MHO_SpectralLineFringeFitter::MHO_SpectralLineFringeFitter(MHO_FringeData* data): MHO_FringeFitter(data)
{
    fEnableCaching = false;
    vis_data = nullptr;
    wt_data = nullptr;

    fOperatorBuildManager =
        new MHO_OperatorBuilderManager(&fOperatorToolbox, fFringeData, fFringeData->GetControlFormat());
}

MHO_SpectralLineFringeFitter::~MHO_SpectralLineFringeFitter()
{
    if(fOperatorBuildManager)
    {
        delete fOperatorBuildManager;
        fOperatorBuildManager = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Configure - identical bookkeeping to MHO_BasicFringeFitter::Configure()
////////////////////////////////////////////////////////////////////////////////

void MHO_SpectralLineFringeFitter::Configure()
{
    profiler_scope();

    fVexInfo = fScanStore->GetRootFileData();

    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!skipped)
    {
        std::string baseline = fParameterStore->GetAs< std::string >("/config/baseline");
        std::string polprod  = fParameterStore->GetAs< std::string >("/config/polprod");

        //-------------------------------------------------------------------
        // Load data and assemble the container store
        //-------------------------------------------------------------------
        fScanStore->LoadBaseline(baseline, fContainerStore);
        fParameterStore->Set("/files/baseline_input_file", fScanStore->GetBaselineFilename(baseline));

        MHO_BasicFringeDataConfiguration::configure_visibility_data(fContainerStore);

        vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        wt_data  = fContainerStore->GetObject< weight_type >(std::string("weight"));
        if(vis_data == nullptr || wt_data == nullptr)
        {
            msg_error("fringe", "could not find visibility or weight objects (vis, weight)." << eom);
            fParameterStore->Set("/status/skipped", true);
            return;
        }

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

        fParameterStore->Set("/uuid/visibilities", vis_data->GetObjectUUID().as_string());
        fParameterStore->Set("/uuid/weights",      wt_data->GetObjectUUID().as_string());

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
            msg_error("fringe", "could not find station coordinate data (ref_sta, rem_sta)." << eom);
            fParameterStore->Set("/status/skipped", true);
            return;
        }
        fParameterStore->Set("/uuid/ref_coord", ref_data->GetObjectUUID().as_string());
        fParameterStore->Set("/uuid/rem_coord", rem_data->GetObjectUUID().as_string());

        multitone_pcal_type* ref_pcal = fContainerStore->GetObject< multitone_pcal_type >(std::string("ref_pcal"));
        multitone_pcal_type* rem_pcal = fContainerStore->GetObject< multitone_pcal_type >(std::string("rem_pcal"));
        if(ref_pcal != nullptr) { fParameterStore->Set("/uuid/ref_pcal", ref_pcal->GetObjectUUID().as_string()); }
        if(rem_pcal != nullptr) { fParameterStore->Set("/uuid/rem_pcal", rem_pcal->GetObjectUUID().as_string()); }

        //-------------------------------------------------------------------
        // Parameter setup
        //-------------------------------------------------------------------
        MHO_InitialFringeInfo::configure_reference_frequency(fContainerStore, fParameterStore);

        //-------------------------------------------------------------------
        // Operator build manager
        //-------------------------------------------------------------------
        fOperatorBuildManager->CreateDefaultBuilders();
        fOperatorBuildManager->SetControlStatements(&(fFringeData->GetControlStatements()));
        fOperatorBuildManager->BuildOperatorCategory("default");

        take_snapshot_here("test", "visib",   __FILE__, __LINE__, vis_data);
        take_snapshot_here("test", "weights", __FILE__, __LINE__, wt_data);

        fOperatorBuildManager->BuildOperatorCategory("labeling");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "labeling");
        fOperatorBuildManager->BuildOperatorCategory("selection");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "selection");

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

        MHO_InitialFringeInfo::precalculate_quantities(fContainerStore, fParameterStore);

        fOperatorBuildManager->BuildOperatorCategory("flagging");
        fOperatorBuildManager->BuildOperatorCategory("calibration");
        fOperatorBuildManager->BuildOperatorCategory("prefit");
        fOperatorBuildManager->BuildOperatorCategory("postfit");
        fOperatorBuildManager->BuildOperatorCategory("finalize");

        if(fOperatorToolbox.GetNOperatorsInCategory("prefit") > 0 &&
           fOperatorToolbox.GetNOperatorsInCategory("postfit") > 0)
        {
            msg_debug("fringe", "enabling visibility/weight caching due to prefit/postfit operators." << eom);
            fEnableCaching = true;
        }
        Cache();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Cache / Refresh
////////////////////////////////////////////////////////////////////////////////

void MHO_SpectralLineFringeFitter::Cache()
{
    if(fEnableCaching)
    {
        vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        wt_data  = fContainerStore->GetObject< weight_type >(std::string("weight"));
        if(vis_data != nullptr)
        {
            auto cached = vis_data->Clone();
            fContainerStore->AddObject(cached);
            fContainerStore->SetShortName(cached->GetObjectUUID(), std::string("cached_v"));
        }
        if(wt_data != nullptr)
        {
            auto cached = wt_data->Clone();
            fContainerStore->AddObject(cached);
            fContainerStore->SetShortName(cached->GetObjectUUID(), std::string("cached_w"));
        }
    }
}

void MHO_SpectralLineFringeFitter::Refresh()
{
    if(fEnableCaching)
    {
        auto vd  = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        auto wd  = fContainerStore->GetObject< weight_type >(std::string("weight"));
        auto cvd = fContainerStore->GetObject< visibility_type >(std::string("cached_v"));
        auto cwd = fContainerStore->GetObject< weight_type >(std::string("cached_w"));
        if(vd != nullptr && cvd != nullptr) { vd->Copy(*cvd); }
        if(wd != nullptr && cwd != nullptr) { wd->Copy(*cwd); }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Initialize
////////////////////////////////////////////////////////////////////////////////

void MHO_SpectralLineFringeFitter::Initialize()
{
    profiler_scope();
    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!skipped)
    {
        Refresh();
        MHO_InitialFringeInfo::compute_total_summed_weights(fContainerStore, fParameterStore);
        MHO_InitialFringeInfo::determine_n_active_channels(fContainerStore, fParameterStore);
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "flagging");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "calibration");
    }
}

////////////////////////////////////////////////////////////////////////////////
// PreRun - set up spectral-line search operators
////////////////////////////////////////////////////////////////////////////////

void MHO_SpectralLineFringeFitter::PreRun()
{
    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!skipped)
    {
        // User-specified prefit operators (pol-product summation, passband, etc.).
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "prefit");

        // Re-fetch pointers in case prefit operators modified the containers.
        vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        wt_data  = fContainerStore->GetObject< weight_type >(std::string("weight"));
        if(vis_data == nullptr || wt_data == nullptr)
        {
            msg_error("fringe", "spectral line fitter: visibility/weight data missing after prefit." << eom);
            fParameterStore->Set("/status/skipped", true);
            return;
        }

        // Validate the polprod assumption.
        if(vis_data->GetDimension(POLPROD_AXIS) != 1)
        {
            msg_error("fringe",
                      "MHO_SpectralLineFringeFitter requires polprod axis size = 1, got "
                          << vis_data->GetDimension(POLPROD_AXIS)
                          << ". Apply pol-product summation in the prefit stage." << eom);
            fParameterStore->Set("/status/skipped", true);
            return;
        }

        double ref_freq_mhz = fParameterStore->GetAs< double >("/control/config/ref_freq");

        // Configure spectral-line fringe search.
        fSLFringeSearch.SetWeights(wt_data);
        fSLFringeSearch.SetReferenceFrequency(ref_freq_mhz);
        fSLFringeSearch.SetArgs(vis_data);

        // Apply optional user-specified DR window.
        if(fParameterStore->IsPresent("/control/fit/dr_win"))
        {
            std::vector< double > drwin = fParameterStore->GetAs< std::vector< double > >("/control/fit/dr_win");
            fSLFringeSearch.SetDRWindow(drwin[0], drwin[1]);
        }

        // Apply optional user-specified frequency search window (MHz).
        if(fParameterStore->IsPresent("/control/fit/spectral_line_freq_win"))
        {
            std::vector< double > fwin =
                fParameterStore->GetAs< std::vector< double > >("/control/fit/spectral_line_freq_win");
            fSLFringeSearch.SetFrequencyWindow(fwin[0], fwin[1]);
        }

        bool ok = fSLFringeSearch.Initialize();
        if(!ok)
        {
            msg_error("fringe", "spectral line fringe search initialization failed." << eom);
            fParameterStore->Set("/status/skipped", true);
            return;
        }

        // Configure peak interpolator.
        fSLPeakInterpolator.SetSpecDRData(fSLFringeSearch.GetSpecDRData());
        fSLPeakInterpolator.SetWeights(wt_data);
        fSLPeakInterpolator.SetReferenceFrequency(ref_freq_mhz);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Run - coarse search then fine interpolation
////////////////////////////////////////////////////////////////////////////////

void MHO_SpectralLineFringeFitter::Run()
{
    profiler_scope();

    bool is_finished = fParameterStore->GetAs< bool >("/status/is_finished");
    bool skipped     = fParameterStore->GetAs< bool >("/status/skipped");
    if(!is_finished && !skipped)
    {
        coarse_spectral_line_search();
    }

    // Re-check: coarse search may have set skipped on failure.
    is_finished = fParameterStore->GetAs< bool >("/status/is_finished");
    skipped     = fParameterStore->GetAs< bool >("/status/skipped");
    if(!is_finished && !skipped)
    {
        interpolate_spectral_line_peak();
        fParameterStore->Set("/status/is_finished", true);
        MHO_BasicFringeUtilities::calculate_fringe_solution_info(fContainerStore, fParameterStore, fVexInfo);
    }
}

////////////////////////////////////////////////////////////////////////////////
// PostRun
////////////////////////////////////////////////////////////////////////////////

void MHO_SpectralLineFringeFitter::PostRun()
{
    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!skipped)
    {
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "postfit");
    }
}

////////////////////////////////////////////////////////////////////////////////
// IsFinished
////////////////////////////////////////////////////////////////////////////////

bool MHO_SpectralLineFringeFitter::IsFinished()
{
    return fParameterStore->GetAs< bool >("/status/is_finished");
}

////////////////////////////////////////////////////////////////////////////////
// Finalize - store search windows; plot data is deferred to a later step
////////////////////////////////////////////////////////////////////////////////

void MHO_SpectralLineFringeFitter::Finalize()
{
    profiler_scope();

    bool status_is_finished = fParameterStore->GetAs< bool >("/status/is_finished");
    bool skipped             = fParameterStore->GetAs< bool >("/status/skipped");
    if(status_is_finished && !skipped)
    {
        // Store the DR window that was actually used.
        double dr_low, dr_high;
        fSLFringeSearch.GetDRWindow(dr_low, dr_high);
        std::vector< double > dr_win = {dr_low, dr_high};
        fParameterStore->Set("/fringe/dr_win", dr_win);

        // For compatibility with downstream consumers that expect sb_win and mb_win,
        // set them to a sentinel range indicating they are not meaningful here.
        std::vector< double > zero_win = {0.0, 0.0};
        fParameterStore->Set("/fringe/sb_win", zero_win);
        fParameterStore->Set("/fringe/mb_win", zero_win);

        // Run finalize-category operators (e.g. output writers).
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "finalize");
    }
}

////////////////////////////////////////////////////////////////////////////////
// Private helpers
////////////////////////////////////////////////////////////////////////////////

void MHO_SpectralLineFringeFitter::coarse_spectral_line_search()
{
    profiler_scope();

    bool ok = fSLFringeSearch.Execute();
    if(!ok)
    {
        msg_fatal("fringe", "spectral line fringe search execution failed." << eom);
        fParameterStore->Set("/status/skipped", true);
        fParameterStore->Set("/status/is_finished", true);
        return;
    }

    int peak_chan = fSLFringeSearch.GetChanMaxBin();
    int peak_dr   = fSLFringeSearch.GetDRMaxBin();
    int peak_freq = fSLFringeSearch.GetFreqMaxBin();

    if(peak_chan < 0 || peak_dr < 0 || peak_freq < 0)
    {
        msg_fatal("fringe", "spectral line search could not locate peak, skipping." << eom);
        fParameterStore->Set("/status/skipped", true);
        fParameterStore->Set("/status/is_finished", true);
        return;
    }

    double total_summed_weights = fParameterStore->GetAs< double >("/fringe/total_summed_weights");
    double search_max_amp       = fSLFringeSearch.GetSearchMaximumAmplitude();

    fParameterStore->Set("/fringe/coarse_search_max_amp", search_max_amp / total_summed_weights);
    fParameterStore->Set("/fringe/peak_channel_idx",  peak_chan);
    fParameterStore->Set("/fringe/peak_freq_bin",     peak_freq);
    fParameterStore->Set("/fringe/max_dr_bin",        peak_dr);
    fParameterStore->Set("/fringe/peak_spectral_freq", fSLFringeSearch.GetCoarsePeakSkyFrequencyMHz());
    fParameterStore->Set("/fringe/n_dr_points",       fSLFringeSearch.GetNDRBins());
    fParameterStore->Set("/fringe/n_pts_searched",    (double)fSLFringeSearch.GetNDRBins());
    fParameterStore->Set("/fringe/is_spectral_line",  true);

    msg_debug("fringe", "coarse spectral line search: peak (chan=" << peak_chan
                                                                   << " dr=" << peak_dr
                                                                   << " freq=" << peak_freq << ")"
                                                                   << " amp=" << search_max_amp / total_summed_weights
                                                                   << eom);
}

void MHO_SpectralLineFringeFitter::interpolate_spectral_line_peak()
{
    profiler_scope();

    int peak_chan = fParameterStore->GetAs< int >("/fringe/peak_channel_idx");
    int peak_dr   = fParameterStore->GetAs< int >("/fringe/max_dr_bin");
    int peak_freq = fParameterStore->GetAs< int >("/fringe/peak_freq_bin");

    fSLPeakInterpolator.SetMaxBins(peak_chan, peak_dr, peak_freq);
    fSLPeakInterpolator.SetDRAxis(fSLFringeSearch.GetDRAxis());

    bool ok = fSLPeakInterpolator.Initialize();
    if(!ok)
    {
        msg_error("fringe", "spectral line peak interpolator initialization failed." << eom);
        fParameterStore->Set("/status/skipped", true);
        return;
    }

    ok = fSLPeakInterpolator.Execute();
    if(!ok)
    {
        msg_error("fringe", "spectral line peak interpolator execution failed." << eom);
        fParameterStore->Set("/status/skipped", true);
        return;
    }

    double drate        = fSLPeakInterpolator.GetDelayRate();
    double frate        = fSLPeakInterpolator.GetFringeRate();
    double famp         = fSLPeakInterpolator.GetFringeAmplitude();
    double fphase       = fSLPeakInterpolator.GetFringePhase();
    double phase_delay  = fSLPeakInterpolator.GetPhaseDelay();   // seconds
    double peak_mhz     = fSLPeakInterpolator.GetPeakSkyFrequencyMHz();

    // sbdelay is undefined for spectral-line data; set to 0.
    fParameterStore->Set("/fringe/sbdelay", 0.0);

    // mbdelay is set to the phase delay expressed in microseconds.
    // Downstream consumers expect microseconds for this key.
    // NOTE: this is the PHASE delay, not the group delay.
    fParameterStore->Set("/fringe/mbdelay", phase_delay * 1e6);

    fParameterStore->Set("/fringe/drate",          drate);
    fParameterStore->Set("/fringe/frate",          frate);
    fParameterStore->Set("/fringe/famp",           famp);
    fParameterStore->Set("/fringe/fringe_phase",   fphase);
    fParameterStore->Set("/fringe/raw_resid_phase_rad", fphase);
    fParameterStore->Set("/fringe/peak_spectral_freq", peak_mhz);
}

} // namespace hops
