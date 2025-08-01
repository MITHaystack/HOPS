#include "MHO_IonosphericFringeFitter.hh"

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

MHO_IonosphericFringeFitter::MHO_IonosphericFringeFitter(MHO_FringeData* data): MHO_BasicFringeFitter(data)
{
    ion_npts = MAX_ION_PTS;
};

MHO_IonosphericFringeFitter::~MHO_IonosphericFringeFitter(){};

void MHO_IonosphericFringeFitter::Run()
{
    profiler_start();

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

    profiler_stop();
}

void MHO_IonosphericFringeFitter::Finalize()
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
    profiler_stop();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int MHO_IonosphericFringeFitter::rjc_ion_search() //(struct type_pass *pass)
{
    profiler_start();

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

    //iono phase op
    MHO_IonosphericPhaseCorrection iono;
    iono.SetArgs(vis_data);
    iono.Initialize();

    bool first_pass = true;

    int loopion;
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
        std::string ref_mk4id = fParameterStore->GetAs< std::string >("/ref_station/mk4id");
        std::string ref_ion_path = "/control/station/" + ref_mk4id + "/ionosphere";
        double ref_ion = 0;
        ok = fParameterStore->Get(ref_ion_path, ref_ion);
        if(!ok)
        {
            ref_ion = 0.0;
        }

        std::string rem_mk4id = fParameterStore->GetAs< std::string >("/rem_station/mk4id");
        std::string rem_ion_path = "/control/station/" + rem_mk4id + "/ionosphere";
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
                // should do parabolic interpolation here
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
                // should do parabolic interpolation here
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
                // should do parabolic interpolation here
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

        for(ionloop = 0; ionloop < ilmax; ionloop++)
        {
            loopion = ionloop;
            // offset ionosphere by search offset
            ion_diff = bottom + ionloop * step;

            // do 3-D grid search using FFT's

            //remove the effects of the last application
            iono.SetDifferentialTEC(last_ion_diff);
            iono.Execute();

            //apply the current ionospheric phase
            iono.SetDifferentialTEC(-1.0 * ion_diff);
            iono.Execute();
            last_ion_diff = ion_diff;

            coarse_fringe_search(first_pass);

            if(first_pass)
            {
                //cache the full SBD search window for later
                fMBDSearch->GetSBDWindow(fInitialSBWin[0], fInitialSBWin[1]);
            }

            // restore original window values for interpolation
            for(i = 0; i < 2; i++)
            {
                win_sb[i] = fInitialSBWin[i];
                win_dr[i] = win_dr_save[i];
            }

            // // interpolate via direct counter-rotation for
            // // more precise results
            interpolate_peak();

            if(first_pass)
            {
                //limit the SBD window to bin where the max was located
                double sbdelay = fParameterStore->GetAs< double >("/fringe/sbdelay");
                double sbdsep = fMBDSearch->GetSBDBinSeparation();
                double approx_snr = calculate_approx_snr();
                if(approx_snr > 15.0) //but only if the SNR was high enough
                {
                    msg_debug("fringe",
                              "ionospheric fringe search cached SBD window to: (" << sbdelay << ", " << sbdelay << ")" << eom);
                    fMBDSearch->SetSBDWindow(sbdelay - sbdsep, sbdelay + sbdsep);
                    first_pass = false;
                }
            }

            // save values for iterative search
            double delres_max = fParameterStore->GetAs< double >("/fringe/famp");
            values[ionloop] = delres_max;

            if(delres_max > max_so_far)
            {
                max_so_far = delres_max;
                fParameterStore->Set("/fringe/ion_diff", ion_diff);
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
void MHO_IonosphericFringeFitter::sort_tecs(int nion, std::vector< std::vector< double > >& dtec)
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

    profiler_stop();
};

// experimental ion search, which performs a smoothing step of
// the coarse points, then goes immediately to a fine search
// around the maximum

int MHO_IonosphericFringeFitter::ion_search_smooth()
{

    profiler_start();

    bool ok;
    int i, k, kmax, ilmax, level, ionloop, rc, koff, nip, win_dr_save[2];
    double coarse_spacing, fine_spacing, step, bottom, center, valmax, y[3], q[3], xmax, ampmax, xlo;

    //from param
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

    //iono phase op
    MHO_IonosphericPhaseCorrection iono;
    iono.SetArgs(vis_data);
    iono.Initialize();

    bool first_pass = true;
    int loopion;
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
        std::string ref_mk4id = fParameterStore->GetAs< std::string >("/ref_station/mk4id");
        std::string ref_ion_path = "/control/station/" + ref_mk4id + "/ionosphere";
        double ref_ion = 0.0;
        ok = fParameterStore->Get(ref_ion_path, ref_ion);
        if(!ok)
        {
            ref_ion = 0.0;
        }

        std::string rem_mk4id = fParameterStore->GetAs< std::string >("/rem_station/mk4id");
        std::string rem_ion_path = "/control/station/" + rem_mk4id + "/ionosphere";
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
    // do search over ionosphere differential
    // TEC (if desired)
    for(level = 0; level < 3; level++) // search level (coarse, fine, final)
    {
        switch(level)
        {
            case 0: // set up for coarse ion search
                ilmax = ion_npts;
                step = coarse_spacing;
                bottom = center - (ilmax - 1) / 2.0 * step;
                if(ion_npts == 1) // if no ionospheric search, proceed
                    level = 3;    // immediately to final delay & rate search
                break;

            case 1: // set up for fine ion search
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
                // make fine search symmetric about level 0 max
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
                // should do parabolic interpolation here
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
        for(ionloop = 0; ionloop < ilmax; ionloop++)
        {
            loopion = ionloop;
            // offset ionosphere by search offset
            ion_diff = bottom + ionloop * step;

            // do 3-D grid search using FFT's

            //remove the effects of the last application
            iono.SetDifferentialTEC(last_ion_diff);
            iono.Execute();

            //apply the current ionospheric phase
            iono.SetDifferentialTEC(-1.0 * ion_diff);
            iono.Execute();
            last_ion_diff = ion_diff;

            coarse_fringe_search(first_pass);

            // if(first_pass)
            // {
            //     //cache the full SBD search window for later
            //     fMBDSearch->GetSBDWindow(fInitialSBWin[0], fInitialSBWin[1]);
            //     //then just limit the SBD window to bin where the max was located
            //     double sbdelay = fParameterStore->GetAs<double>("/fringe/sbdelay");
            //     msg_debug("fringe", "ionospheric fringe search cached SBD window to: ("<<sbdelay<<", "<<sbdelay<< ")"<<eom);
            //     fMBDSearch->SetSBDWindow(sbdelay, sbdelay);
            //     first_pass = false;
            // }

            if(first_pass)
            {
                //cache the full SBD search window for later
                fMBDSearch->GetSBDWindow(fInitialSBWin[0], fInitialSBWin[1]);
                // //then just limit the SBD window to bin where the max was located
                // double sbdelay = fParameterStore->GetAs< double >("/fringe/sbdelay");
                // double sbdsep = fMBDSearch->GetSBDBinSeparation();
                // double approx_snr = calculate_approx_snr();
                // if(approx_snr > 15.0)
                // {
                //     msg_debug("fringe",
                //               "ionospheric fringe search cached SBD window to: (" << sbdelay << ", " << sbdelay << ")" << eom);
                //     fMBDSearch->SetSBDWindow(sbdelay - sbdsep, sbdelay + sbdsep);
                //     first_pass = false;
                // }

                // msg_debug("fringe",
                //           "ionospheric fringe search cached SBD window to: (" << sbdelay << ", " << sbdelay << ")" << eom);
                // fMBDSearch->SetSBDWindow(sbdelay - sbdsep, sbdelay + sbdsep);
                // first_pass = false;
            }

            // restore original window values for interpolation
            for(i = 0; i < 2; i++)
            {
                win_sb[i] = fInitialSBWin[i];
                win_dr[i] = win_dr_save[i];
            }
            // interpolate via direct counter-rotation for
            // more precise results
            interpolate_peak();

            if(first_pass)
            {
                //limit the SBD window to bin where the max was located
                double sbdelay = fParameterStore->GetAs< double >("/fringe/sbdelay");
                double sbdsep = fMBDSearch->GetSBDBinSeparation();
                double approx_snr = calculate_approx_snr();
                if(approx_snr > 15.0) //but only if the SNR was high enough
                {
                    msg_debug("fringe",
                              "ionospheric fringe search cached SBD window to: (" << sbdelay << ", " << sbdelay << ")" << eom);
                    fMBDSearch->SetSBDWindow(sbdelay - sbdsep, sbdelay + sbdsep);
                    first_pass = false;
                }
            }

            // save values for iterative search
            double delres_max = fParameterStore->GetAs< double >("/fringe/famp");
            values[ionloop] = delres_max;

            if(delres_max > max_so_far)
            {
                max_so_far = delres_max;
                fParameterStore->Set("/fringe/ion_diff", ion_diff);
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

    profiler_stop();

    return 0;
}

// smooth an array of numbers and interpolate fourfold
// the algorithm takes the original data array f, inserts
// three 0-pts between each value, then does a convolution
// with a half-cycle of a cos curve, properly normalizing
// the result g

void MHO_IonosphericFringeFitter::smoother(double* f,        // input data array with arbitrary positive length
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

double MHO_IonosphericFringeFitter::calculate_approx_snr()
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
