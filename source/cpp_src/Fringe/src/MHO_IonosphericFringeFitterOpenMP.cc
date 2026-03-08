#include "MHO_IonosphericFringeFitterOpenMP.hh"

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

//experimental ion phase correction
#include "MHO_IonosphericPhaseCorrection.hh"
#include "MHO_MathUtilities.hh"

// number of points in fine search
#define N_FINE_PTS 12
#define N_MED_PTS 12         // number of points in medium search
#define N_FINE_PTS_SMOOTH 24 // # of fine points with new smoothing algorithm

#define MAX_ION_PTS 100 //legacy max number of ion function points

namespace hops
{

MHO_IonosphericFringeFitterOpenMP::MHO_IonosphericFringeFitterOpenMP(MHO_FringeData* data): MHO_BasicFringeFitter(data)
{
    ion_npts = MAX_ION_PTS;
    delete this->fMBDSearch; //delete the base class version of fMBDSearch (which could be OpenMP enabled)
    this->fMBDSearch = new MHO_MBDelaySearch(); //use single threaded version for this fringe fitter
};

MHO_IonosphericFringeFitterOpenMP::~MHO_IonosphericFringeFitterOpenMP()
{
    delete this->fMBDSearch;
    this->fMBDSearch = nullptr;
};

void MHO_IonosphericFringeFitterOpenMP::Run()
{
    profiler_scope();

    bool is_finished = fParameterStore->GetAs< bool >("/status/is_finished");
    bool skipped = fParameterStore->GetAs< bool >("/status/skipped");
    if(!is_finished && !skipped) //execute if we are not finished and are not skipping
    {
        //determine if we use the smoothed algorithm or not
        bool do_smoothing;
        bool ok = fParameterStore->Get("/control/fit/ion_smooth", do_smoothing);
        if(!ok)
        {
            do_smoothing = false;
        }

        //do bare bones first-pass (no iono) to set the sbd
        //coarse_fringe_search();

        int ret_val = 0;
        if(do_smoothing)
        {
            ret_val = ion_search_smooth();
            if(ret_val != 0)
            {
                msg_error("fringe", "could not execute smoothed ion search" << eom);
            }
        }
        else
        {
            ret_val = rjc_ion_search();
            if(ret_val != 0)
            {
                msg_error("fringe", "could not execute standard ion search" << eom);
            }
        }

        fParameterStore->Set("/status/is_finished", true);
        //have sampled all grid points, find the solution and finalize
        //calculate the fringe properties
        MHO_BasicFringeUtilities::calculate_fringe_solution_info(fContainerStore, fParameterStore, fVexInfo);
    }

    
}

void MHO_IonosphericFringeFitterOpenMP::Finalize()
{
    profiler_scope();
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
        // fMBDSearch->GetSBDWindow(low,high);
        win[0] = fInitialSBWin[0];
        win[1] = fInitialSBWin[1];
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

        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "finalize");
    }
    
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void MHO_IonosphericFringeFitterOpenMP::initialize_ion_threads()
{
#ifdef _OPENMP
    fNIonThreads = omp_get_max_threads();
#else
    fNIonThreads = 1;
#endif
    //no benefit in having more threads than ion points
    if(fNIonThreads > ion_npts) { fNIonThreads = std::max(1, ion_npts); }

    msg_debug("fringe", "MHO_IonosphericFringeFitterOpenMP using " << fNIonThreads
                            << " thread(s) for parallel ion search" << eom);

    //snapshot clean vis_data before any ion correction is applied
    fVisRef.Copy(*vis_data);

    //parameters needed to initialize per-thread search objects
    double ref_freq = fParameterStore->GetAs< double >("/control/config/ref_freq");
    double frt_offset = fParameterStore->GetAs< double >("/config/frt_offset");
    bool optimize_closure_flag = false;
    fParameterStore->Get("/control/fit/optimize_closure", optimize_closure_flag);
    bool is_mixed = ContainsMixedSideband(vis_data);

    //clear previous state (allows re-initialization if called again)
    fPerThreadVis.clear();
    fPerThreadSBD.clear();
    fPerThreadSSBNormFX.clear();
    fPerThreadMSBNormFX.clear();
    fPerThreadNormFXPtr.clear();
    fPerThreadMBDSearch.clear();
    fPerThreadPeakInterp.clear();
    fPerThreadIono.clear();

    fPerThreadVis.resize(fNIonThreads);
    fPerThreadSBD.resize(fNIonThreads);
    fPerThreadSSBNormFX.resize(fNIonThreads);
    fPerThreadMSBNormFX.resize(fNIonThreads);
    fPerThreadNormFXPtr.resize(fNIonThreads, nullptr);
    fPerThreadMBDSearch.resize(fNIonThreads);
    fPerThreadPeakInterp.resize(fNIonThreads);
    fPerThreadIono.resize(fNIonThreads);

    //all Initialize() calls are serial (FFTW plan creation is not thread-safe)
    for(int t = 0; t < fNIonThreads; t++)
    {
        //per-thread vis copy (reset at the start of each parallel ionloop body)
        fPerThreadVis[t].Copy(fVisRef);

        //per-thread SBD output (copy structure from already-sized sbd_data)
        fPerThreadSBD[t].Copy(*sbd_data);

        //per-thread normFX: choose SSB or MSB to match the main pipeline
        if(is_mixed)
        {
            fPerThreadMSBNormFX[t] = std::make_unique< MHO_MixedSidebandNormFX >();
            fPerThreadMSBNormFX[t]->SetArgs(&fPerThreadVis[t], &fPerThreadSBD[t]);
            fPerThreadMSBNormFX[t]->SetWeights(wt_data);
            fPerThreadMSBNormFX[t]->Initialize();
            fPerThreadNormFXPtr[t] = fPerThreadMSBNormFX[t].get();
        }
        else
        {
            fPerThreadSSBNormFX[t] = std::make_unique< MHO_SingleSidebandNormFX >();
            fPerThreadSSBNormFX[t]->SetArgs(&fPerThreadVis[t], &fPerThreadSBD[t]);
            fPerThreadSSBNormFX[t]->SetWeights(wt_data);
            fPerThreadSSBNormFX[t]->Initialize();
            fPerThreadNormFXPtr[t] = fPerThreadSSBNormFX[t].get();
        }

        //per-thread MBD search (always single-threaded to avoid nested parallelism)
        fPerThreadMBDSearch[t] = std::make_unique< MHO_MBDelaySearch >();
        fPerThreadMBDSearch[t]->SetWeights(wt_data);
        fPerThreadMBDSearch[t]->SetReferenceFrequency(ref_freq);
        fPerThreadMBDSearch[t]->SetArgs(&fPerThreadSBD[t]);
        fPerThreadMBDSearch[t]->Initialize();

        //propagate search windows from control parameters
        if(fParameterStore->IsPresent("/control/fit/sb_win"))
        {
            auto sbwin = fParameterStore->GetAs< std::vector< double > >("/control/fit/sb_win");
            fPerThreadMBDSearch[t]->SetSBDWindow(sbwin[0], sbwin[1]);
        }
        if(fParameterStore->IsPresent("/control/fit/mb_win"))
        {
            auto mbwin = fParameterStore->GetAs< std::vector< double > >("/control/fit/mb_win");
            fPerThreadMBDSearch[t]->SetMBDWindow(mbwin[0], mbwin[1]);
        }
        if(fParameterStore->IsPresent("/control/fit/dr_win"))
        {
            auto drwin = fParameterStore->GetAs< std::vector< double > >("/control/fit/dr_win");
            fPerThreadMBDSearch[t]->SetDRWindow(drwin[0], drwin[1]);
        }

        //per-thread peak interpolator
        fPerThreadPeakInterp[t] = std::make_unique< MHO_InterpolateFringePeakOptimized >();
        if(optimize_closure_flag) { fPerThreadPeakInterp[t]->EnableOptimizeClosure(); }
        fPerThreadPeakInterp[t]->SetReferenceFrequency(ref_freq);
        fPerThreadPeakInterp[t]->SetReferenceTimeOffset(frt_offset);
        fPerThreadPeakInterp[t]->SetSBDArray(&fPerThreadSBD[t]);
        fPerThreadPeakInterp[t]->SetWeights(wt_data);

        //per-thread ionospheric corrector (in-place on fPerThreadVis[t])
        fPerThreadIono[t] = std::make_unique< MHO_IonosphericPhaseCorrection >();
        fPerThreadIono[t]->SetArgs(&fPerThreadVis[t]);
        fPerThreadIono[t]->Initialize();
    }
}

int MHO_IonosphericFringeFitterOpenMP::rjc_ion_search() //(struct type_pass *pass)
{
    profiler_scope();

    bool ok;

    int i, k, kmax, ilmax, level, ionloop, rc, koff, nip, win_dr_save[2];

    double coarse_spacing, medium_spacing, fine_spacing, step, bottom, center, valmax, y[3], q[3], xmax, ampmax, xlo;

    //from param
    std::vector< double > values;
    double win_ion[2];
    double ion_diff;
    double last_ion_diff = 0.0;
    double win_dr[2];
    double win_sb[2];
    double max_so_far = 0.0;

    visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
    if(vis_data == nullptr)
    {
        msg_error("fringe", "could not find visibility object with names (vis)." << eom);
        return 1;
    }

    bool first_pass = true;

    int nion;

    ok = fParameterStore->Get("/control/fit/ion_npts", ion_npts);
    if(!ok)
    {
        ion_npts = 1;
    }
    std::vector< double > iwin;
    ok = fParameterStore->Get("/control/fit/ion_win", iwin);
    if(ok)
    {
        win_ion[0] = std::min(iwin[0], iwin[1]);
        win_ion[1] = std::max(iwin[0], iwin[1]);
        msg_debug("fringe", "using an ion window of: (" << win_ion[0] << ", " << win_ion[1] << ")" << eom);
    }
    else
    {
        win_ion[0] = 0.0;
        win_ion[1] = 0.0;
        msg_debug("fringe",
                  "no ion window set, defaulting to ion window of: (" << win_ion[0] << ", " << win_ion[1] << ")" << eom);
    }

    //fixed ion fit...so we need to check if each station has an assigned a priori ion value
    if(ion_npts == 1)
    {
        std::string ref_id = fParameterStore->GetAs< std::string >("/ref_station/site_id");
        std::string ref_ion_path = "/control/station/" + ref_id + "/ionosphere";
        double ref_ion = 0;
        ok = fParameterStore->Get(ref_ion_path, ref_ion);
        if(!ok)
        {
            ref_ion = 0.0;
        }

        std::string rem_id = fParameterStore->GetAs< std::string >("/rem_station/site_id");
        std::string rem_ion_path = "/control/station/" + rem_id + "/ionosphere";
        double rem_ion = 0;
        ok = fParameterStore->Get(rem_ion_path, rem_ion);
        if(!ok)
        {
            rem_ion = 0.0;
        }

        double ion_delta = rem_ion - ref_ion;
        win_ion[0] = ion_delta;
        win_ion[1] = ion_delta;
    }

    //put the ion_win info into the 'fringe' section of the parameters
    fParameterStore->Set("/fringe/ion_win", win_ion);

    // prepare for ionospheric search
    center = (win_ion[0] + win_ion[1]) / 2.0;

    //pad the size of the values/dtec arrays in case smoothing is applied
    values.resize(ion_npts + N_FINE_PTS_SMOOTH + 1, 0.0);

    //from status
    std::vector< std::vector< double > > dtec;
    dtec.resize(ion_npts + N_FINE_PTS_SMOOTH + 1);
    for(std::size_t ip = 0; ip < dtec.size(); ip++)
    {
        dtec[ip].resize(2, 0.0);
    }

    coarse_spacing = win_ion[1] - win_ion[0];
    if(ion_npts > 1)
    {
        coarse_spacing /= ion_npts - 1;
        nip = 0;
    }

    //initialize per-thread search infrastructure (must be serial, after ion_npts is known)
    initialize_ion_threads();

    //main iono corrector for the serial passes (level 0 first-pass and level 3 final)
    MHO_IonosphericPhaseCorrection iono;
    iono.SetArgs(vis_data);
    iono.Initialize();

    //track narrowed SBD window to propagate to per-thread objects after first pass
    bool sbd_narrowed = false;
    double narrowed_sbd_low = 0.0, narrowed_sbd_high = 0.0;

    //result slots: one per ionloop index (max ilmax across all levels)
    int max_ilmax = std::max({ion_npts, N_MED_PTS, N_FINE_PTS, 1});
    fIonLoopResults.resize(max_ilmax);

    medium_spacing = 2.0;
    fine_spacing = 0.4;
    // do search over ionosphere differential
    // TEC (if desired)
    for(level = 0; level < 4; level++) // search level (coarse, medium, fine, final)
    {
        switch(level)
        {
            case 0: // set up for coarse ion search
                ilmax = ion_npts;
                step = coarse_spacing;
                bottom = center - (ilmax - 1) / 2.0 * step;
                if(ion_npts == 1) // if no ionospheric search, proceed
                {
                    level = 3; // immediately to final delay & rate search
                }
                break;
            case 1: // set up for medium ion search
                // find maximum from coarse search
                valmax = -1.0;
                for(k = 0; k < ilmax; k++)
                {
                    if(values[k] > valmax)
                    {
                        valmax = values[k];
                        kmax = k;
                    }
                    // store this coarse ionosphere point
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                }
                if(kmax == 0) // coarse maximum up against lower edge?
                {
                    center = bottom + (N_MED_PTS - 1) / 2.0 * medium_spacing;
                }
                else if(kmax == ion_npts) // upper edge?
                {
                    center = bottom + (kmax - 1) * step - (N_MED_PTS - 1) / 2.0 * medium_spacing;
                }
                else // max was one of the interior points
                {
                    center = bottom + kmax * step;
                }
                ilmax = N_MED_PTS;
                step = medium_spacing;
                // make medium search symmetric about level 0 max
                bottom = center - (ilmax - 1) / 2.0 * step;
                break;
            case 2: // set up for fine ion search
                // find maximum from medium search
                valmax = -1.0;
                for(k = 0; k < ilmax; k++)
                {
                    if(values[k] > valmax)
                    {
                        valmax = values[k];
                        kmax = k;
                    }
                    // store this medium ionosphere point
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                }
                if(kmax == 0) // medium maximum up against lower edge?
                {
                    center = bottom + (N_FINE_PTS - 1) / 2.0 * fine_spacing;
                }
                else if(kmax == ion_npts) // upper edge?
                {
                    center = bottom + (kmax - 1) * step - (N_FINE_PTS - 1) / 2.0 * fine_spacing;
                }
                else // max was one of the interior points
                {
                    center = bottom + kmax * step;
                }
                ilmax = N_FINE_PTS;
                step = fine_spacing;
                // make fine search symmetric about level 0 max
                bottom = center - (ilmax - 1) / 2.0 * step;
                break;
            case 3: // final evaluation
                // find maximum from fine search
                valmax = -1.0;
                for(k = 0; k < ilmax; k++)
                {
                    if(values[k] > valmax)
                    {
                        valmax = values[k];
                        kmax = k;
                    }
                    // store this fine ionosphere point
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                }
                // parabolic interpolation to find sub-bin optimum
                if(kmax == 0)
                {
                    koff = +1;
                }
                else if(kmax == ilmax - 1)
                {
                    koff = -1;
                }
                else
                {
                    koff = 0;
                }

                for(k = 0; k < 3; k++)
                {
                    y[k] = values[kmax + k - 1 + koff];
                    xlo = bottom + (kmax - 1 + koff) * step;
                }

                rc = MHO_MathUtilities::parabola(y, -1.0, 1.0, &xmax, &ampmax, q);

                if(rc == 1)
                {
                    msg_error("calibration", "TEC fine interpolation error; peak out of search range" << eom);
                }
                else if(rc == 2)
                {
                    msg_error("calibration", "TEC fine interpolation error; positive curvature" << eom);
                }

                center = xlo + (xmax + 1.0) * step;
                bottom = center;
                ilmax = 1;
                step = 0.0;
                break;
        }

        if(level == 3)
        {
            //////////////////////////////////////////////////////////////////////
            // Level 3: final serial pass using main objects.
            // Restores vis_data to the clean reference, applies the parabolic-
            // interpolated best ion correction, and runs the full pipeline so
            // that fParameterStore and vis_data are in the correct final state.
            //////////////////////////////////////////////////////////////////////
            std::memcpy(vis_data->GetData(), fVisRef.GetData(),
                        fVisRef.GetSize() * sizeof(*vis_data->GetData()));

            ion_diff = bottom; // ilmax=1, step=0
            iono.SetDifferentialTEC(-1.0 * ion_diff);
            iono.Execute();

            coarse_fringe_search(false);
            interpolate_peak();

            double delres_max = fParameterStore->GetAs< double >("/fringe/famp");
            values[0] = delres_max;
            if(delres_max > max_so_far)
            {
                max_so_far = delres_max;
                fParameterStore->Set("/fringe/ion_diff", ion_diff);
            }
        }
        else
        {
            //////////////////////////////////////////////////////////////////////
            // Levels 0, 1, 2: parallel ionloops using per-thread objects.
            // Level 0 ionloop=0 runs serially first to handle first_pass logic
            // (SBD window caching / narrowing).
            //////////////////////////////////////////////////////////////////////
            int loop_start = 0;

            if(level == 0 && first_pass)
            {
                //--- serial first pass (ionloop=0 of level 0) ---
                ion_diff = bottom; // ionloop=0, step*0=0

                iono.SetDifferentialTEC(-1.0 * ion_diff);
                iono.Execute();

                coarse_fringe_search(true); // sets MBD/DR/SBD windows on fMBDSearch
                fMBDSearch->GetSBDWindow(fInitialSBWin[0], fInitialSBWin[1]);

                interpolate_peak();

                {
                    double sbdelay = fParameterStore->GetAs< double >("/fringe/sbdelay");
                    double sbdsep = fMBDSearch->GetSBDBinSeparation();
                    double approx_snr = calculate_approx_snr();
                    if(approx_snr > 15.0)
                    {
                        msg_debug("fringe", "ionospheric fringe search cached SBD window to: ("
                                                << sbdelay << ", " << sbdelay << ")" << eom);
                        narrowed_sbd_low  = sbdelay - sbdsep;
                        narrowed_sbd_high = sbdelay + sbdsep;
                        fMBDSearch->SetSBDWindow(narrowed_sbd_low, narrowed_sbd_high);
                        sbd_narrowed = true;
                        first_pass = false;
                    }
                }

                double delres_max_0 = fParameterStore->GetAs< double >("/fringe/famp");
                fIonLoopResults[0].famp     = delres_max_0;
                fIonLoopResults[0].ion_diff = ion_diff;
                values[0] = delres_max_0; // populate values[0] - harvest loop starts at loop_start=1
                if(delres_max_0 > max_so_far)
                {
                    max_so_far = delres_max_0;
                    fParameterStore->Set("/fringe/ion_diff", ion_diff);
                }

                //restore vis_data to clean reference for the upcoming parallel section
                std::memcpy(vis_data->GetData(), fVisRef.GetData(),
                            fVisRef.GetSize() * sizeof(*vis_data->GetData()));

                //propagate the (possibly narrowed) SBD window to per-thread objects
                if(sbd_narrowed)
                {
                    for(int t = 0; t < fNIonThreads; t++)
                        fPerThreadMBDSearch[t]->SetSBDWindow(narrowed_sbd_low, narrowed_sbd_high);
                }

                loop_start = 1;
            }

            //--- parallel ionloops (loop_start..ilmax-1) ---
            #pragma omp parallel for num_threads(fNIonThreads) schedule(dynamic)
            for(ionloop = loop_start; ionloop < ilmax; ionloop++)
            {
                #ifdef _OPENMP
                    int tid = omp_get_thread_num();
                #else
                    int tid = 0;
                #endif

                double local_ion_diff = bottom + ionloop * step;

                //reset thread-local vis from the clean reference
                std::memcpy(fPerThreadVis[tid].GetData(), fVisRef.GetData(),
                            fVisRef.GetSize() * sizeof(*fVisRef.GetData()));

                //apply this iteration's ion correction (from clean state, no undo needed)
                fPerThreadIono[tid]->SetDifferentialTEC(-1.0 * local_ion_diff);
                fPerThreadIono[tid]->Execute();

                //coarse SBD/MBD/DR search
                fPerThreadSBD[tid].ZeroArray();
                fPerThreadNormFXPtr[tid]->Execute();
                fPerThreadMBDSearch[tid]->Execute();

                int c_mbdmax = fPerThreadMBDSearch[tid]->GetMBDMaxBin();
                int c_sbdmax = fPerThreadMBDSearch[tid]->GetSBDMaxBin();
                int c_drmax  = fPerThreadMBDSearch[tid]->GetDRMaxBin();

                //fine peak interpolation
                fPerThreadPeakInterp[tid]->SetMaxBins(c_sbdmax, c_mbdmax, c_drmax);
                fPerThreadPeakInterp[tid]->SetMBDAxis(fPerThreadMBDSearch[tid]->GetMBDAxis());
                fPerThreadPeakInterp[tid]->SetDRAxis(fPerThreadMBDSearch[tid]->GetDRAxis());
                fPerThreadPeakInterp[tid]->Initialize();
                fPerThreadPeakInterp[tid]->Execute();

                fIonLoopResults[ionloop].famp     = fPerThreadPeakInterp[tid]->GetFringeAmplitude();
                fIonLoopResults[ionloop].ion_diff = local_ion_diff;
            }

            //harvest results and update max_so_far (serial, after parallel section)
            for(ionloop = loop_start; ionloop < ilmax; ionloop++)
            {
                values[ionloop] = fIonLoopResults[ionloop].famp;
                if(values[ionloop] > max_so_far)
                {
                    max_so_far = values[ionloop];
                    fParameterStore->Set("/fringe/ion_diff", fIonLoopResults[ionloop].ion_diff);
                }
            }
        }
    }

    // save the final ion. point, if there is one
    if(ion_npts > 1)
    {
        dtec[nip][0] = center;
        dtec[nip++][1] = values[0];
        nion = nip;
        sort_tecs(nion, dtec);
    }
    else
    {
        nion = 0;
    }

    return 0;
};

// sort tec array
void MHO_IonosphericFringeFitterOpenMP::sort_tecs(int nion, std::vector< std::vector< double > >& dtec)
{
    int i, n, changed = 1;
    double temp[2];
    while(changed)
    {
        changed = 0;
        for(n = 0; n < nion - 1; n++)
        {
            if(dtec[n][0] > dtec[n + 1][0])
            {
                for(i = 0; i < 2; i++)
                {
                    temp[i] = dtec[n][i];
                    dtec[n][i] = dtec[n + 1][i];
                    dtec[n + 1][i] = temp[i];
                }
                changed = 1;
            }
        }
    }

    //ok now stuff these into the parameter store for now:
    std::vector< double > dtec_values;
    std::vector< double > dtec_amp_values;

    for(i = 0; i < nion; i++)
    {
        dtec_values.push_back(dtec[i][0]);
        dtec_amp_values.push_back(dtec[i][1]);
    }

    fParameterStore->Set("/fringe/dtec_array", dtec_values);
    fParameterStore->Set("/fringe/dtec_amp_array/", dtec_amp_values);

    
};

// experimental ion search, which performs a smoothing step of
// the coarse points, then goes immediately to a fine search
// around the maximum

int MHO_IonosphericFringeFitterOpenMP::ion_search_smooth()
{
    profiler_scope();

    bool ok;
    int k, kmax, ilmax, level, ionloop, rc, koff, nip;
    double coarse_spacing, fine_spacing, step, bottom, center, valmax, y[3], q[3], xmax, ampmax, xlo;

    //from param
    double win_ion[2];
    double ion_diff;
    double max_so_far = 0.0;

    visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
    if(vis_data == nullptr)
    {
        msg_error("fringe", "could not find visibility object with names (vis)." << eom);
        return 1;
    }

    bool first_pass = true;
    int nion;

    ok = fParameterStore->Get("/control/fit/ion_npts", ion_npts);
    if(!ok)
    {
        ion_npts = 1;
    }
    std::vector< double > iwin;
    ok = fParameterStore->Get("/control/fit/ion_win", iwin);
    if(ok)
    {
        win_ion[0] = std::min(iwin[0], iwin[1]);
        win_ion[1] = std::max(iwin[0], iwin[1]);
        msg_debug("fringe", "using an ion window of: (" << win_ion[0] << ", " << win_ion[1] << ")" << eom);
    }
    else
    {
        win_ion[0] = 0.0;
        win_ion[1] = 0.0;
        msg_debug("fringe",
                  "no ion window set, defaulting to ion window of: (" << win_ion[0] << ", " << win_ion[1] << ")" << eom);
    }

    //fixed ion fit...so we need to check if each station has an assigned a priori ion value
    if(ion_npts == 1)
    {
        std::string ref_id = fParameterStore->GetAs< std::string >("/ref_station/site_id");
        std::string ref_ion_path = "/control/station/" + ref_id + "/ionosphere";
        double ref_ion = 0.0;
        ok = fParameterStore->Get(ref_ion_path, ref_ion);
        if(!ok)
        {
            ref_ion = 0.0;
        }

        std::string rem_id = fParameterStore->GetAs< std::string >("/rem_station/site_id");
        std::string rem_ion_path = "/control/station/" + rem_id + "/ionosphere";
        double rem_ion = 0.0;
        ok = fParameterStore->Get(rem_ion_path, rem_ion);
        if(!ok)
        {
            rem_ion = 0.0;
        }

        double ion_delta = rem_ion - ref_ion;
        win_ion[0] = ion_delta;
        win_ion[1] = ion_delta;
    }

    //put the ion_win info into the 'fringe' section of the parameters
    fParameterStore->Set("/fringe/ion_win", win_ion);

    //pad the values and dtec arrays in case of smoothing
    std::vector< double > values;
    values.resize(ion_npts + N_FINE_PTS_SMOOTH + 1, 0.0);
    std::vector< double > smoothed_values;
    smoothed_values.resize(4 * ion_npts, 0.0);

    std::vector< std::vector< double > > dtec;
    dtec.resize(ion_npts + N_FINE_PTS_SMOOTH + 1);
    for(std::size_t ip = 0; ip < dtec.size(); ip++)
    {
        dtec[ip].resize(2, 0.0);
    }

    // prepare for ionospheric search
    center = (win_ion[0] + win_ion[1]) / 2.0;
    coarse_spacing = win_ion[1] - win_ion[0];
    if(ion_npts > 1)
    {
        coarse_spacing /= ion_npts - 1;
        nip = 0;
    }

    fine_spacing = 0.4;

    //initialize per-thread search infrastructure (must be serial, after ion_npts is known)
    initialize_ion_threads();

    //main iono corrector for the serial passes (level 0 first-pass and level 2 final)
    MHO_IonosphericPhaseCorrection iono;
    iono.SetArgs(vis_data);
    iono.Initialize();

    //track narrowed SBD window to propagate to per-thread objects after first pass
    bool sbd_narrowed = false;
    double narrowed_sbd_low = 0.0, narrowed_sbd_high = 0.0;

    //result slots: one per ionloop index (max ilmax across all levels)
    int max_ilmax = std::max({ion_npts, N_FINE_PTS_SMOOTH, 1});
    fIonLoopResults.resize(max_ilmax);

    // do search over ionosphere differential TEC (if desired)
    for(level = 0; level < 3; level++) // search level (coarse, fine, final)
    {
        switch(level)
        {
            case 0: // set up for coarse ion search
                ilmax = ion_npts;
                step = coarse_spacing;
                bottom = center - (ilmax - 1) / 2.0 * step;
                if(ion_npts == 1) // if no ionospheric search, proceed
                    level = 2;    // immediately to final delay & rate search
                break;

            case 1: // set up for fine ion search (with smoothing)
                // first, store the coarse ionosphere points
                for(k = 0; k < ilmax; k++)
                {
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                }
                // then smooth and interpolate coarse points
                smoother(&(values[0]), &(smoothed_values[0]), &step, &ilmax);
                // find maximum from smoothed coarse search
                valmax = -1.0;
                for(k = 0; k < ilmax; k++)
                {
                    if(smoothed_values[k] > valmax)
                    {
                        valmax = smoothed_values[k];
                        kmax = k;
                    }
                }
                if(kmax == 0) // coarse maximum up against lower edge?
                    center = bottom + (N_FINE_PTS_SMOOTH - 1) / 2.0 * fine_spacing;
                else if(kmax == ion_npts) // upper edge?
                    center = bottom + (kmax - 1) * step - (N_FINE_PTS_SMOOTH - 1) / 2.0 * fine_spacing;
                else // max was one of the interior points
                    center = bottom + kmax * step;
                ilmax = N_FINE_PTS_SMOOTH;
                step = fine_spacing;
                // make fine search symmetric about smoothed max
                bottom = center - (ilmax - 1) / 2.0 * step;
                break;

            case 2: // final evaluation
                // find maximum from fine search
                valmax = -1.0;
                for(k = 0; k < ilmax; k++)
                {
                    if(values[k] > valmax)
                    {
                        valmax = values[k];
                        kmax = k;
                    }
                    // store this fine ionosphere point
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                }
                if(kmax == 0)
                    koff = +1;
                else if(kmax == ilmax - 1)
                    koff = -1;
                else
                    koff = 0;

                for(k = 0; k < 3; k++)
                {
                    y[k] = values[kmax + k - 1 + koff];
                    xlo = bottom + (kmax - 1 + koff) * step;
                }

                rc = MHO_MathUtilities::parabola(y, -1.0, 1.0, &xmax, &ampmax, q);

                if(rc == 1)
                {
                    msg_error("fringe", "TEC fine interpolation error; peak out of search range" << eom);
                }
                else if(rc == 2)
                {
                    msg_error("fringe", "TEC fine interpolation error; positive curvature" << eom);
                }

                center = xlo + (xmax + 1.0) * step;
                bottom = center;
                ilmax = 1;
                step = 0.0;
                break;
        }

        if(level == 2)
        {
            //////////////////////////////////////////////////////////////////////
            // Level 2: final serial pass using main objects.
            // Restores vis_data to the clean reference, applies the parabolic-
            // interpolated best ion correction, and runs the full pipeline so
            // that fParameterStore and vis_data are in the correct final state.
            //////////////////////////////////////////////////////////////////////
            std::memcpy(vis_data->GetData(), fVisRef.GetData(),
                        fVisRef.GetSize() * sizeof(*vis_data->GetData()));

            ion_diff = bottom; // ilmax=1, step=0
            iono.SetDifferentialTEC(-1.0 * ion_diff);
            iono.Execute();

            coarse_fringe_search(false);
            interpolate_peak();

            double delres_max = fParameterStore->GetAs< double >("/fringe/famp");
            values[0] = delres_max;
            if(delres_max > max_so_far)
            {
                max_so_far = delres_max;
                fParameterStore->Set("/fringe/ion_diff", ion_diff);
            }
        }
        else
        {
            //////////////////////////////////////////////////////////////////////
            // Levels 0 and 1: parallel ionloops using per-thread objects.
            // Level 0 ionloop=0 runs serially first to handle first_pass logic
            // (SBD window caching / narrowing).
            //////////////////////////////////////////////////////////////////////
            int loop_start = 0;

            if(level == 0 && first_pass)
            {
                //--- serial first pass (ionloop=0 of level 0) ---
                ion_diff = bottom; // ionloop=0

                iono.SetDifferentialTEC(-1.0 * ion_diff);
                iono.Execute();

                coarse_fringe_search(true); // sets MBD/DR/SBD windows on fMBDSearch
                fMBDSearch->GetSBDWindow(fInitialSBWin[0], fInitialSBWin[1]);

                interpolate_peak();

                {
                    double sbdelay = fParameterStore->GetAs< double >("/fringe/sbdelay");
                    double sbdsep = fMBDSearch->GetSBDBinSeparation();
                    double approx_snr = calculate_approx_snr();
                    if(approx_snr > 15.0)
                    {
                        msg_debug("fringe", "ionospheric fringe search cached SBD window to: ("
                                                << sbdelay << ", " << sbdelay << ")" << eom);
                        narrowed_sbd_low  = sbdelay - sbdsep;
                        narrowed_sbd_high = sbdelay + sbdsep;
                        fMBDSearch->SetSBDWindow(narrowed_sbd_low, narrowed_sbd_high);
                        sbd_narrowed = true;
                        first_pass = false;
                    }
                }

                double delres_max_0 = fParameterStore->GetAs< double >("/fringe/famp");
                fIonLoopResults[0].famp     = delres_max_0;
                fIonLoopResults[0].ion_diff = ion_diff;
                values[0] = delres_max_0; // populate values[0] - harvest loop starts at loop_start=1
                if(delres_max_0 > max_so_far)
                {
                    max_so_far = delres_max_0;
                    fParameterStore->Set("/fringe/ion_diff", ion_diff);
                }

                //restore vis_data to clean reference for the upcoming parallel section
                std::memcpy(vis_data->GetData(), fVisRef.GetData(),
                            fVisRef.GetSize() * sizeof(*vis_data->GetData()));

                //propagate the (possibly narrowed) SBD window to per-thread objects
                if(sbd_narrowed)
                {
                    for(int t = 0; t < fNIonThreads; t++)
                        fPerThreadMBDSearch[t]->SetSBDWindow(narrowed_sbd_low, narrowed_sbd_high);
                }

                loop_start = 1;
            }

            //--- parallel ionloops (loop_start..ilmax-1) ---
            #pragma omp parallel for num_threads(fNIonThreads) schedule(dynamic)
            for(ionloop = loop_start; ionloop < ilmax; ionloop++)
            {
                #ifdef _OPENMP
                    int tid = omp_get_thread_num();
                #else
                    int tid = 0;
                #endif

                double local_ion_diff = bottom + ionloop * step;

                //reset thread-local vis from the clean reference
                std::memcpy(fPerThreadVis[tid].GetData(), fVisRef.GetData(),
                            fVisRef.GetSize() * sizeof(*fVisRef.GetData()));

                //apply this iteration's ion correction (from clean state, no undo needed)
                fPerThreadIono[tid]->SetDifferentialTEC(-1.0 * local_ion_diff);
                fPerThreadIono[tid]->Execute();

                //coarse SBD/MBD/DR search
                fPerThreadSBD[tid].ZeroArray();
                fPerThreadNormFXPtr[tid]->Execute();
                fPerThreadMBDSearch[tid]->Execute();

                int c_mbdmax = fPerThreadMBDSearch[tid]->GetMBDMaxBin();
                int c_sbdmax = fPerThreadMBDSearch[tid]->GetSBDMaxBin();
                int c_drmax  = fPerThreadMBDSearch[tid]->GetDRMaxBin();

                //fine peak interpolation
                fPerThreadPeakInterp[tid]->SetMaxBins(c_sbdmax, c_mbdmax, c_drmax);
                fPerThreadPeakInterp[tid]->SetMBDAxis(fPerThreadMBDSearch[tid]->GetMBDAxis());
                fPerThreadPeakInterp[tid]->SetDRAxis(fPerThreadMBDSearch[tid]->GetDRAxis());
                fPerThreadPeakInterp[tid]->Initialize();
                fPerThreadPeakInterp[tid]->Execute();

                fIonLoopResults[ionloop].famp     = fPerThreadPeakInterp[tid]->GetFringeAmplitude();
                fIonLoopResults[ionloop].ion_diff = local_ion_diff;
            }

            //harvest results and update max_so_far (serial, after parallel section)
            for(ionloop = loop_start; ionloop < ilmax; ionloop++)
            {
                values[ionloop] = fIonLoopResults[ionloop].famp;
                if(values[ionloop] > max_so_far)
                {
                    max_so_far = values[ionloop];
                    fParameterStore->Set("/fringe/ion_diff", fIonLoopResults[ionloop].ion_diff);
                }
            }
        }
    }

    // save the final ion. point, if there is one
    if(ion_npts > 1)
    {
        dtec[nip][0] = center;
        dtec[nip++][1] = values[0];
        nion = nip;
        sort_tecs(nion, dtec);
    }
    else
        nion = 0;

    return 0;
}

// smooth an array of numbers and interpolate fourfold
// the algorithm takes the original data array f, inserts
// three 0-pts between each value, then does a convolution
// with a half-cycle of a cos curve, properly normalizing
// the result g

void MHO_IonosphericFringeFitterOpenMP::smoother(double* f,        // input data array with arbitrary positive length
                                           double* g,        // output data array with fourfold interpolation
                                           double* tec_step, // grid spacing of f in TEC units
                                           int* npts)        // pointer to length of input array - modified!
{
    int i, j, k, kbeg, kend, n,
        nf, // # of input pts
        ng, // # of output pts
        ns; // # of smoothing curve pts

    std::vector< double > gwork;
    gwork.resize(4 * ion_npts, 0.0);
    std::vector< double > shape;
    shape.resize(4 * ion_npts, 0.0);
    double ssum;

    // generate a smoothing curve. The shape of the idealized
    // curve for correlation as a function of TEC is dependent
    // on frequency distribution, but for a wide range of
    // reasonable 3-10 GHz distributions it has a half-width
    // of about 3 TEC units. Thus we use half a cosine curve,
    // having approximately that half power width.
    ns = 36 / *tec_step;
    ns |= 1; // make it odd, and ensure it isn't too large

    if(ns >= 4 * ion_npts)
        ns = 4 * ion_npts - 1;
    for(n = 0; n < ns; n++)
    {
        shape[n] = cos(M_PI * (n - ns / 2) / ns);
        //msg ("shape %d %f", -2, n, shape[n]);
    }
    *tec_step /= 4; // reduced step size for interpolation

    nf = *npts;
    ng = 4 * nf - 3;
    *npts = ng; // update caller's copy of length
                // form sparse g work array from f values
    for(i = 0, j = 0; j < ng; j++)
    {
        if(j % 4 == 0)
            gwork[j] = f[i++];
        else
            gwork[j] = 0.0;
        g[j] = 0; // also clear g for later use
    }

    for(j = 0; j < ng; j++) // convolution loop
    {
        kbeg = (ns - 1) / 2 - j; // calculate part of shape function to convolve with
        if(kbeg < 0)
            kbeg = 0;
        kend = ng + (ns - 1) / 2 - j;
        if(kend > ns)
            kend = ns;

        ssum = 0;
        for(k = kbeg; k < kend; k++)
        {
            g[j] += gwork[j + k - (ns - 1) / 2] * shape[k];
            // sum used shape for normalization
            if(gwork[j + k - (ns - 1) / 2] != 0)
                ssum = ssum + shape[k];
        }
        if(ssum != 0)
            g[j] /= ssum;
    }
}

double MHO_IonosphericFringeFitterOpenMP::calculate_approx_snr()
{
    //snr_approx = 1e-4 * status.delres_max * param.inv_sigma * sqrt((double)status.total_ap_frac * 2.0);
    double eff_npols = 1.0;
    std::vector< std::string > pp_vec = fParameterStore->GetAs< std::vector< std::string > >("/config/polprod_set");
    if(pp_vec.size() >= 2)
    {
        eff_npols = 2.0;
    }

    double bw_corr_factor = 1.0;
    double ap_delta = fParameterStore->GetAs< double >("/config/ap_period");
    double samp_period = fParameterStore->GetAs< double >("/vex/scan/sample_period/value");
    double total_summed_weights = fParameterStore->GetAs< double >("/fringe/total_summed_weights");
    double famp = fParameterStore->GetAs< double >("/fringe/famp");
    double snr =
        MHO_BasicFringeInfo::calculate_snr(eff_npols, ap_delta, samp_period, total_summed_weights, famp, bw_corr_factor);
    return snr;
}

} // namespace hops
