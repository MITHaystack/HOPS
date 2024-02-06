#include "MHO_IonosphericFringeFitter.hh"

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

//experimental ion phase correction
#include "MHO_IonosphericPhaseCorrection.hh"
#include "MHO_MathUtilities.hh"

// number of points in fine search
#define N_FINE_PTS 12
#define N_MED_PTS 12                // number of points in medium search
#define N_FINE_PTS_SMOOTH 24        // # of fine points with new smoothing algorithm
#define MAX_ION_PTS 100



namespace hops
{


MHO_IonosphericFringeFitter::MHO_IonosphericFringeFitter(MHO_FringeData* data):
    MHO_BasicFringeFitter(data)
{};

MHO_IonosphericFringeFitter::~MHO_IonosphericFringeFitter(){};



void MHO_IonosphericFringeFitter::Run()
{
    std::cout<<"dumping parameter store = "<<std::endl;
    fParameterStore->Dump();

    bool is_finished = fParameterStore->GetAs<bool>("/status/is_finished");
    bool skipped = fParameterStore->GetAs<bool>("/status/skipped");
    if( !is_finished  && !skipped) //execute if we are not finished and are not skipping
    {
        rjc_ion_search();
        fParameterStore->Set("/status/is_finished", true);
        //have sampled all grid points, find the solution and finalize
        //calculate the fringe properties
        MHO_BasicFringeUtilities::calculate_fringe_solution_info(fContainerStore, fParameterStore, fVexInfo);
    }


}



// void MHO_IonosphericFringeFitter::Finalize()
// {
//     ////////////////////////////////////////////////////////////////////////////
//     //PLOTTING/DEBUG
//     ////////////////////////////////////////////////////////////////////////////
//     //TODO may want to reorg the way this is done
// 
//     bool is_finished = fParameterStore->GetAs<bool>("/status/is_finished");
//     bool skipped = fParameterStore->GetAs<bool>("/status/skipped");
//     if( is_finished  && !skipped ) //have to be finished and not-skipped
//     {
//         fPlotData = MHO_FringePlotInfo::construct_plot_data(&fContainerStore, &fParameterStore, fVexInfo);
//         MHO_FringePlotInfo::fill_plot_data(&fParameterStore, fPlotData);
//     }
// }





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


int 
MHO_IonosphericFringeFitter::rjc_ion_search() //(struct type_pass *pass)
{

    int i,
    k,
    kmax,
    ilmax,
    level,
    ionloop,
    rc,
    koff,
    nip,
    win_dr_save[2];

    double win_sb_save[2];


    double coarse_spacing,
    medium_spacing,
    fine_spacing,
    step,
    bottom,
    center,
    valmax,
    y[3],
    q[3],
    xmax,
    ampmax,
    xlo;

    //from param
    double values[MAX_ION_PTS];
    double win_ion[2];
    int ion_pts;
    double ion_diff;
    double last_ion_diff = 0.0;
    double win_dr[2];
    double win_sb[2];


    visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
    if( vis_data == nullptr )
    {
        msg_fatal("fringe", "could not find visibility object with names (vis)." << eom);
        std::exit(1);
    }

    //iono phase op
    MHO_IonosphericPhaseCorrection iono;
    iono.SetArgs(vis_data);
    iono.Initialize();

    bool first_pass = true;

    //from status
    double dtec[MAX_ION_PTS][2];
    int loopion;
    int nion;

    ion_pts = fParameterStore->GetAs<int>("/control/fit/ion_npts");
    std::vector< double > iwin;
    fParameterStore->Get("/control/fit/ion_win", iwin);
    win_ion[0] = iwin[0];
    win_ion[1] = iwin[1];


    // prepare for ionospheric search
    center = (win_ion[0] + win_ion[1]) / 2.0;
    // condition total # of points
    if (ion_pts > MAX_ION_PTS - N_MED_PTS - N_FINE_PTS - 1)   
    {
        ion_pts = MAX_ION_PTS - N_MED_PTS - N_FINE_PTS - 1;   
        //msg ("limited ion search to %d points", 2, ion_pts);
    }
    coarse_spacing = win_ion[1] - win_ion[0];
    if (ion_pts > 1)
    {
        coarse_spacing /= ion_pts - 1;
        nip = 0;
    }

    medium_spacing = 2.0;
    fine_spacing = 0.4;
    // do search over ionosphere differential
    // TEC (if desired)
    for (level=0; level<4; level++)     // search level (coarse, medium, fine, final)
    {
        switch (level)
        {
            case 0:                     // set up for coarse ion search
                std::cout<<"CASE 0 "<<std::endl;
                ilmax = ion_pts;
                step = coarse_spacing;
                bottom = center - (ilmax - 1) / 2.0 * step;
                if (ion_pts == 1)// if no ionospheric search, proceed
                {
                    level = 3;          // immediately to final delay & rate search
                }
            break;
            case 1:                     // set up for medium ion search 
                std::cout<<"CASE 1 "<<std::endl;
                // find maximum from coarse search
                // should do parabolic interpolation here
                valmax = -1.0;
                for (k=0; k<ilmax; k++)
                {
                    if (values[k] > valmax)
                    {
                        valmax = values[k];
                        kmax = k;
                    }
                    // store this coarse ionosphere point
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                }
                if (kmax == 0)          // coarse maximum up against lower edge?
                {
                    center = bottom + (N_MED_PTS - 1) / 2.0 * medium_spacing;
                }
                else if (kmax == ion_pts) // upper edge?
                {
                    center = bottom + (kmax - 1) * step - (N_MED_PTS - 1) / 2.0 * medium_spacing;
                }
                else                    // max was one of the interior points
                {
                    center = bottom + kmax * step;
                }
                ilmax = N_MED_PTS;
                step = medium_spacing;
                // make medium search symmetric about level 0 max
                bottom = center - (ilmax - 1) / 2.0 * step;
            break;
            case 2:                     // set up for fine ion search 
                std::cout<<"CASE 2 "<<std::endl;
                // find maximum from medium search
                // should do parabolic interpolation here
                valmax = -1.0;
                for (k=0; k<ilmax; k++)
                {
                    if (values[k] > valmax)
                    {
                        valmax = values[k];
                        kmax = k;
                    }
                    // store this medium ionosphere point
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                }
                if (kmax == 0)          // medium maximum up against lower edge?
                {
                    center = bottom + (N_FINE_PTS - 1) / 2.0 * fine_spacing;
                }
                else if (kmax == ion_pts) // upper edge?
                {
                    center = bottom + (kmax - 1) * step - (N_FINE_PTS - 1) / 2.0 * fine_spacing;
                }
                else                    // max was one of the interior points
                {
                    center = bottom + kmax * step;
                }
                ilmax = N_FINE_PTS;
                step = fine_spacing;
                // make fine search symmetric about level 0 max
                bottom = center - (ilmax - 1) / 2.0 * step;
            break;
            case 3:                     // final evaluation
                std::cout<<"CASE 3 "<<std::endl;
                // find maximum from fine search
                valmax = -1.0;
                for (k=0; k<ilmax; k++)
                {
                    if (values[k] > valmax)
                    {
                        valmax = values[k];
                        kmax = k;
                    }
                    // store this fine ionosphere point
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                }
                // should do parabolic interpolation here
                if (kmax == 0)
                {
                    koff = +1;
                }
                else if (kmax == ilmax - 1)
                {
                    koff = -1;
                }
                else
                {
                    koff = 0;
                }
                
                for (k=0; k<3; k++)
                {
                    y[k] = values[kmax + k - 1 + koff];
                    xlo = bottom + (kmax - 1 + koff) * step;
                }
        
                std::cout<<"calling parabola"<<std::endl;
                rc = MHO_MathUtilities::parabola (y, -1.0, 1.0, &xmax, &ampmax, q);

                if (rc == 1)
                {
                    //msg ("TEC fine interpolation error; peak out of search range",1);
                }
                else if (rc == 2)
                {
                    //msg ("TEC fine interpolation error; positive curvature",1);
                }
                
                center = xlo + (xmax + 1.0) * step;
                bottom = center;
                ilmax = 1;
                step = 0.0;
            break;
        }
        
        for (ionloop=0; ionloop<ilmax; ionloop++)
        {
            loopion = ionloop;
            // offset ionosphere by search offset
            ion_diff = bottom + ionloop * step;

            // do 3-D grid search using FFT's
            rc = 0; //search(pass);
            //execute the basic fringe search algorithm
            //apply the dTEC correction here:

            //remove the effects of the last application
            std::cout<<"Applying inverse dTEC of: "<<last_ion_diff<<std::endl;
            iono.SetDifferentialTEC(last_ion_diff);
            iono.Execute();

            //apply the current ionospheric phase
            std::cout<<"Applying dTEC of: "<<ion_diff<<std::endl;
            iono.SetDifferentialTEC(-1.0*ion_diff);
            iono.Execute();
            last_ion_diff = ion_diff;

            // MHO_BasicFringeUtilities::basic_fringe_search(fContainerStore, fParameterStore);
            coarse_fringe_search();

            if(ionloop==0)
            {
                //cache the full SBD search window for later
                fMBDSearch.GetSBDWindow(win_sb_save[0], win_sb_save[1]);
                //then just limit the SBD window to bin where the max was located
                // double sbdelay = fParameterStore->GetAs<double>("/fringe/sbdelay");
                // fMBDSearch.SetSBDWindow(sbdelay, sbdelay);
                first_pass = false;
            }

            if (rc < 0)
            {
                //msg ("Error fringe searching", 2);
                return -1;
            }
            else if (rc > 0)
            {
                return rc;
            }
            
            // restore original window values for interpolation
            for(i=0; i<2; i++)
            {
                win_sb[i] = win_sb_save[i];
                win_dr[i] = win_dr_save[i];
            }
            
            // // interpolate via direct counter-rotation for
            // // more precise results
            // interp (pass);
            interpolate_peak();

            // save values for iterative search
            double delres_max = fParameterStore->GetAs<double>("/fringe/famp");
            values[ionloop] = delres_max;
            printf("ion search differential TEC %f amp %f \n", ion_diff, delres_max);
        }
    }
    
    // save the final ion. point, if there is one
    if (ion_pts > 1)
    {
        dtec[nip][0] = center;
        dtec[nip++][1] = values[0];
        nion = nip;
        sort_tecs(nion, dtec);
    }
    else{nion = 0;}

    return (0);
};


// sort tec array
void 
MHO_IonosphericFringeFitter::sort_tecs(int nion, double dtec[][2])
{
    std::cout<<"calling sort tecs"<<std::endl;
    int i,n,changed = 1;
    double temp[2];
    while (changed)
    {
        changed = 0;
        for (n=0; n<nion-1; n++)
        {
            if (dtec[n][0] > dtec[n+1][0])
            {
                for (i=0; i<2; i++)
                {
                    temp[i] = dtec[n][i];
                    dtec[n][i] = dtec[n+1][i];
                    dtec[n+1][i] = temp[i];
                }
                changed = 1;
            }
        }
    }

    //ok now stuff these into the parameter store for now:
    std::vector< double > dtec_values;
    std::vector< double > dtec_amp_values;

    for(i=0; i<nion; i++)
    {
        std::cout<<"ion: "<<i<<" : "<<dtec[i][0]<<", "<<dtec[i][1]<<std::endl;
        dtec_values.push_back(dtec[i][0]);
        dtec_amp_values.push_back(dtec[i][1]);
    }
    
    fParameterStore->Set("/fringe/dtec_array", dtec_values);
    fParameterStore->Set("/fringe/dtec_amp_array/", dtec_amp_values);

};















// experimental ion search, which performs a smoothing step of
// the coarse points, then goes immediately to a fine search
// around the maximum

int MHO_IonosphericFringeFitter::ion_search_smooth ()
    {
    int i,
        k,
        kmax,
        ilmax,
        level,
        ionloop,
        rc,
        koff,
        nip,
        win_dr_save[2];

    double win_sb_save[2];

    double coarse_spacing,
           fine_spacing,
           step,
           bottom,
           center,
           valmax,
           y[3],
           q[3],
           xmax,
           ampmax,
           xlo;



    //from param
    //double values[MAX_ION_PTS];
    double win_ion[2];
    int ion_pts;
    double ion_diff;
    double last_ion_diff = 0.0;
    double win_dr[2];
    double win_sb[2];


    visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
    if( vis_data == nullptr )
    {
       msg_fatal("fringe", "could not find visibility object with names (vis)." << eom);
       std::exit(1);
    }

    //iono phase op
    MHO_IonosphericPhaseCorrection iono;
    iono.SetArgs(vis_data);
    iono.Initialize();

    bool first_pass = true;

    //from status
    double dtec[MAX_ION_PTS][2];
    int loopion;
    int nion;

    ion_pts = fParameterStore->GetAs<int>("/control/fit/ion_npts");
    std::vector< double > iwin;
    fParameterStore->Get("/control/fit/ion_win", iwin);
    win_ion[0] = iwin[0];
    win_ion[1] = iwin[1];


    // extern int do_accounting;
    // extern struct type_status status;
    // extern struct type_param param;

    double values[MAX_ION_PTS];
    double smoothed_values[4*MAX_ION_PTS];
    // int parabola (double *, double, double, double *, double *, double *);
    // void sort_tecs (void);
    // void smoother (double *, double *, double *, int *);
    // extern void interp (struct type_pass*);
    // extern int search (struct type_pass*);

                                        // prepare for ionospheric search
    center = (win_ion[0] + win_ion[1]) / 2.0;
                                        // condition total # of points
    if (ion_pts > MAX_ION_PTS - N_FINE_PTS_SMOOTH - 1)   
        {
        ion_pts = MAX_ION_PTS - N_FINE_PTS_SMOOTH - 1;   
        //msg ("limited ion search to %d points", 2, ion_pts);
        }
    coarse_spacing = win_ion[1] - win_ion[0];
    if (ion_pts > 1)
        {
        coarse_spacing /= ion_pts - 1;
        nip = 0;
        }

    fine_spacing = 0.4;
                                        // do search over ionosphere differential
                                        // TEC (if desired)
    for (level=0; level<3; level++)     // search level (coarse, fine, final)
        {
        switch (level)
            {
            case 0:                     // set up for coarse ion search
                ilmax = ion_pts;
                step = coarse_spacing;
                bottom = center - (ilmax - 1) / 2.0 * step;
                if (ion_pts == 1)// if no ionospheric search, proceed
                    level = 3;          // immediately to final delay & rate search
                break;

            case 1:                     // set up for fine ion search 
                                        // first, store the coarse ionosphere points
                for (k=0; k<ilmax; k++)
                    {
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                    //msg("smoother input %d %f", -2, k, values[k]);
                    }
                                        // then smooth and interpolate coarse points
                smoother (values, smoothed_values, &step, &ilmax);
                // for (k=0; k<ilmax; k++)
                //     {
                //     //msg("smoother output %d %f", -2, k, smoothed_values[k]);
                //     }
                                        // find maximum from smoothed coarse search
                valmax = -1.0;
                for (k=0; k<ilmax; k++)
                    {
                    if (smoothed_values[k] > valmax)
                        {
                        valmax = smoothed_values[k];
                        kmax = k;
                        }
                    }
                if (kmax == 0)          // coarse maximum up against lower edge?
                    center = bottom + (N_FINE_PTS_SMOOTH - 1) / 2.0 * fine_spacing;
                else if (kmax == ion_pts) // upper edge?
                    center = bottom + (kmax - 1) * step 
                                    - (N_FINE_PTS_SMOOTH - 1) / 2.0 * fine_spacing;
                else                    // max was one of the interior points
                    center = bottom + kmax * step;

                ilmax = N_FINE_PTS_SMOOTH;
                step = fine_spacing;
                                        // make fine search symmetric about level 0 max
                bottom = center - (ilmax - 1) / 2.0 * step;
                break;

            case 2:                     // final evaluation
                                        // find maximum from fine search
                valmax = -1.0;
                for (k=0; k<ilmax; k++)
                    {
                    if (values[k] > valmax)
                        {
                        valmax = values[k];
                        kmax = k;
                        }
                                        // store this fine ionosphere point
                    dtec[nip][0] = bottom + k * step;
                    dtec[nip++][1] = values[k];
                    }
                                        // should do parabolic interpolation here
                if (kmax == 0)
                    koff = +1;
                else if (kmax == ilmax - 1)
                    koff = -1;
                else
                    koff = 0;

                for (k=0; k<3; k++)
                    {
                    y[k] = values[kmax + k - 1 + koff];
                    xlo = bottom + (kmax - 1 + koff) * step;
                    }

                rc = //msg (y, -1.0, 1.0, &xmax, &ampmax, q);

                // if (rc == 1)
                // {
                //     msg ("TEC fine interpolation error; peak out of search range",1);
                // }
                // else if (rc == 2)
                // {
                //     msg ("TEC fine interpolation error; positive curvature",1);
                // }

                center = xlo + (xmax + 1.0) * step;

                bottom = center;
                ilmax = 1;
                step = 0.0;
                break;
            }
        for (ionloop=0; ionloop<ilmax; ionloop++)
            {
            loopion = ionloop;
                                        // offset ionosphere by search offset
            ion_diff = bottom + ionloop * step;

                                        // do 3-D grid search using FFT's


            //remove the effects of the last application
            std::cout<<"Applying inverse dTEC of: "<<last_ion_diff<<std::endl;
            iono.SetDifferentialTEC(last_ion_diff);
            iono.Execute();

            //apply the current ionospheric phase
            std::cout<<"Applying dTEC of: "<<ion_diff<<std::endl;
            iono.SetDifferentialTEC(-1.0*ion_diff);
            iono.Execute();
            last_ion_diff = ion_diff;

            // MHO_BasicFringeUtilities::basic_fringe_search(fContainerStore, fParameterStore);
            coarse_fringe_search();

            if(ionloop==0)
            {
                //cache the full SBD search window for later
                fMBDSearch.GetSBDWindow(win_sb_save[0], win_sb_save[1]);
                //then just limit the SBD window to bin where the max was located
                // double sbdelay = fParameterStore->GetAs<double>("/fringe/sbdelay");
                // fMBDSearch.SetSBDWindow(sbdelay, sbdelay);
                first_pass = false;
            }


            // rc = search(pass);
            // if (rc < 0)
            //     {
            //     //msg ("Error fringe searching", 2);
            //     return (-1);
            //     }
            // else if (rc > 0)
            //     return (rc);

                                        // restore original window values for interpolation
            for (i=0; i<2; i++)
                {
                win_sb[i] = win_sb_save[i];
                win_dr[i] = win_dr_save[i];
                }
                                        // interpolate via direct counter-rotation for
                                        // more precise results
            //interp (pass);
            interpolate_peak();
            // if (do_accounting) 
            //     account ("Interpolate fringes");
            
                                        // save values for iterative search
            double delres_max = fParameterStore->GetAs<double>("/fringe/famp");
            values[ionloop] = delres_max;
            printf("ion search differential TEC %f amp %f \n", ion_diff, delres_max);

            //values[ionloop] = status.delres_max;
            //msg ("ion search differential TEC %f amp %f", 1, ion_diff, status.delres_max);
            }
        }
                                        // save the final ion. point, if there is one
    if (ion_pts > 1)
        {
        dtec[nip][0] = center;
        dtec[nip++][1] = values[0];
        nion = nip;
        sort_tecs(nion, dtec);
        }
    else
        nion = 0;

    return (0);
    }


// smooth an array of numbers and interpolate fourfold
// the algorithm takes the original data array f, inserts
// three 0-pts between each value, then does a convolution
// with a half-cycle of a cos curve, properly normalizing
// the result g

void MHO_IonosphericFringeFitter::smoother (double *f,           // input data array with arbitrary positive length
                                            double *g,           // output data array with fourfold interpolation
                                            double *tec_step,    // grid spacing of f in TEC units
                                            int *npts)           // pointer to length of input array - modified!
    {
    int i,
        j,
        k, kbeg, kend,
        n,
        nf,                         // # of input pts
        ng,                         // # of output pts
        ns;                         // # of smoothing curve pts
    double gwork[4*MAX_ION_PTS],
           shape[4*MAX_ION_PTS],
           ssum;
    
                                    // generate a smoothing curve. The shape of the idealized
                                    // curve for correlation as a function of TEC is dependent
                                    // on frequency distribution, but for a wide range of
                                    // reasonable 3-10 GHz distributions it has a half-width
                                    // of about 3 TEC units. Thus we use half a cosine curve,
                                    // having approximately that half power width.
    ns = 36 / *tec_step;
    ns |= 1;                        // make it odd, and ensure it isn't too large
    if (ns >= 4 * MAX_ION_PTS)
        ns = 4 * MAX_ION_PTS - 1;
    for (n=0; n<ns; n++)
        {
        shape[n] = cos (M_PI * (n - ns / 2) / ns);
        //msg ("shape %d %f", -2, n, shape[n]);
        }
    *tec_step /= 4;                 // reduced step size for interpolation

    nf = *npts;
    ng = 4 * nf - 3;
    *npts = ng;                     // update caller's copy of length
                                    // form sparse g work array from f values
    for (i=0,j=0; j<ng; j++)
        {
        if (j % 4 == 0)
            gwork[j] = f[i++];
        else
            gwork[j] = 0.0;
        g[j] = 0;                   // also clear g for later use
        }

    for (j=0; j<ng; j++)            // convolution loop
        {
        kbeg = (ns - 1) / 2 - j;    // calculate part of shape function to convolve with
        if (kbeg < 0)
            kbeg = 0;
        kend = ng + (ns - 1) / 2 - j;
        if (kend > ns)
            kend = ns;

        ssum = 0;
        for (k=kbeg; k<kend; k++)
            {
            g[j] += gwork[j+k-(ns-1)/2] * shape[k];
                                    // sum used shape for normalization
            if (gwork[j+k-(ns-1)/2] != 0)
                ssum = ssum + shape[k];
            }
        if (ssum != 0)
            g[j] /= ssum;
        }
    }
























}//end namespace
