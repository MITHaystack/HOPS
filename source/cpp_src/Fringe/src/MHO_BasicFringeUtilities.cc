#include "MHO_BasicFringeUtilities.hh"

//helper functions
#include "MHO_BasicFringeInfo.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//basic_fringe_search
#include "MHO_NormFX.hh"
#include "MHO_DelayRate.hh"
#include "MHO_MBDelaySearch.hh"
#include "MHO_InterpolateFringePeak.hh"
#include "MHO_UniformGridPointsCalculator.hh"

//construct_plot_data
#include "MHO_ComputePlotData.hh"

namespace hops 
{

void 
MHO_BasicFringeUtilities::calculate_fringe_solution_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, const mho_json& vexInfo)
{
    //vex section info
    mho_json freq_section = vexInfo["$FREQ"];
    auto freq_info = freq_section.begin().value();
    double sample_rate = freq_info["sample_rate"]["value"];
    //TODO FIXME (what if channels have multiple-bandwidths?, units?)
    double samp_period = 1.0/(sample_rate*1e6);

    //configuration parameters
    double ref_freq = paramStore->GetAs<double>("ref_freq");
    double ap_delta = paramStore->GetAs<double>("ap_period");

    //fringe quantities
    double total_summed_weights = paramStore->GetAs<double>("/fringe/total_summed_weights");
    double sbdelay = paramStore->GetAs<double>("/fringe/sbdelay");
    double mbdelay = paramStore->GetAs<double>("/fringe/mbdelay");
    double drate = paramStore->GetAs<double>("/fringe/drate");
    double frate = paramStore->GetAs<double>("/fringe/frate");
    double famp = paramStore->GetAs<double>("/fringe/famp");

    //a priori (correlator) model quantities
    double adelay = paramStore->GetAs<double>("/model/adelay");
    double arate = paramStore->GetAs<double>("/model/arate");
    double aaccel = paramStore->GetAs<double>("/model/aaccel");

    std::string frt_vex_string = paramStore->GetAs<std::string>("/vex/scan/fourfit_reftime");
    auto frt = hops_clock::from_vex_format(frt_vex_string);
    legacy_hops_date frt_ldate = hops_clock::to_legacy_hops_date(frt);

    std::string start_vex_string = paramStore->GetAs<std::string>("/vex/scan/start");
    auto start_time = hops_clock::from_vex_format(start_vex_string);
    double start_offset = paramStore->GetAs<double>("start_offset");
    int64_t start_offset_as_nanosec = (int64_t)start_offset*SEC_TO_NANOSEC; //KLUDGE;

    start_time = start_time + hops_clock::duration(start_offset_as_nanosec);
    legacy_hops_date start_ldate = hops_clock::to_legacy_hops_date(start_time);

    double stop_offset = paramStore->GetAs<double>("stop_offset");
    int64_t stop_offset_as_nanosec = (int64_t)stop_offset*SEC_TO_NANOSEC; //KLUDGE;

    auto stop_time = hops_clock::from_vex_format(start_vex_string);
    stop_time = stop_time + hops_clock::duration(stop_offset_as_nanosec);
    legacy_hops_date stop_ldate = hops_clock::to_legacy_hops_date(stop_time);
    std::string year_doy = 
        MHO_BasicFringeInfo::leftpadzeros_integer(4, frt_ldate.year) + ":" + 
        MHO_BasicFringeInfo::leftpadzeros_integer(3, frt_ldate.day);

    //figure out the legacy date/time stamps for start, stop, and FRT
    paramStore->Set("/fringe/year_doy", year_doy);
    paramStore->Set("/fringe/legacy_start_timestamp", MHO_BasicFringeInfo::make_legacy_datetime_format(start_ldate) );
    paramStore->Set("/fringe/legacy_stop_timestamp", MHO_BasicFringeInfo::make_legacy_datetime_format(stop_ldate) );
    paramStore->Set("/fringe/legacy_frt_timestamp", MHO_BasicFringeInfo::make_legacy_datetime_format(frt_ldate) );

    //calculate SNR
    #pragma message("TODO FIXME -- properly calcualte the effective number of pol-products.")
    double eff_npol = 1.0;
    double snr = MHO_BasicFringeInfo::calculate_snr(eff_npol, ap_delta, samp_period, total_summed_weights, famp);
    paramStore->Set("/fringe/snr", snr);

    //calculate integration time
    int nchan = paramStore->GetAs<int>("nchannels");
    double integration_time =  (total_summed_weights*ap_delta)/(double)nchan;
    paramStore->Set("/fringe/integration_time", integration_time);

    //calculate quality code
    std::string quality_code = MHO_BasicFringeInfo::calculate_qf();
    paramStore->Set("/fringe/quality_code", quality_code);

    //total number of points searched
    std::size_t nmbd = paramStore->GetAs<std::size_t>("/fringe/n_mbd_points");
    std::size_t nsbd = paramStore->GetAs<std::size_t>("/fringe/n_sbd_points");
    std::size_t ndr = paramStore->GetAs<std::size_t>("/fringe/n_dr_points");
    double total_npts_searched = (double)nmbd * (double)nsbd *(double)ndr;

    //residual phase in radians and degrees
    double resid_phase_rad = calculate_residual_phase(conStore, paramStore);
    double resid_phase_deg = std::fmod(resid_phase_rad*(180.0/M_PI), 360.0);

    //calculate the a priori phase and total phase
    double aphase = std::fmod( ref_freq*adelay*360.0, 360.0); //from fill_208.c, no conversion from radians??
    double tot_phase_deg = std::fmod( aphase + resid_phase_rad*(180.0/M_PI), 360.0 );
    paramStore->Set("/fringe/aphase", aphase);

    //calculate the probability of false detection, THIS IS BROKEN
    #pragma message("TODO FIXME - PFD calculation needs the MBD/SBD/DR windows defined")
    double pfd = MHO_BasicFringeInfo::calculate_pfd(snr, total_npts_searched);
    pfd = 0.0;
    paramStore->Set("/fringe/prob_false_detect", pfd);

    //TODO FIXME -- -acount for units (these are printed on fringe plot in usec)
    double tot_mbd = adelay + mbdelay;
    double tot_sbd = adelay + sbdelay;

    double ambig = paramStore->GetAs<double>("/fringe/ambiguity");
    double freq_spacing = paramStore->GetAs<double>("/fringe/frequency_spacing");
    std::string mbd_anchor = paramStore->GetAs<std::string>("mbd_anchor");

    // anchor total mbd to sbd if desired by control
    double delta_mbd = 0.0;
    double freq0 = paramStore->GetAs<double>("/fringe/start_frequency");
    if(mbd_anchor == "sbd")
    {
        delta_mbd = ambig * std::floor( (tot_sbd - tot_mbd) / ambig + 0.5);
        tot_mbd += delta_mbd;
        //tweaks tot_phase_deg and resid_phase_deg
        MHO_BasicFringeInfo::correct_phases_mbd_anchor_sbd(ref_freq, freq0, freq_spacing, delta_mbd, tot_phase_deg, resid_phase_deg);
    }
    double resid_ph_delay = resid_phase_rad / (2.0 * M_PI * ref_freq);
    double ph_delay = adelay + resid_ph_delay;

    paramStore->Set("/fringe/resid_phase", resid_phase_deg);
    paramStore->Set("/fringe/resid_ph_delay", resid_ph_delay);
    paramStore->Set("/fringe/phase_delay", ph_delay);
    paramStore->Set("/fringe/tot_phase", tot_phase_deg);

    double tot_drate = arate + drate;
    paramStore->Set("/fringe/total_sbdelay", tot_sbd);
    paramStore->Set("/fringe/total_mbdelay", tot_mbd);
    paramStore->Set("/fringe/total_drate", tot_drate);

    double sbd_sep = paramStore->GetAs<double>("/fringe/sbd_separation");
    double freq_spread = paramStore->GetAs<double>("/fringe/frequency_spread");
    double mbd_error = MHO_BasicFringeInfo::calculate_mbd_no_ion_error(freq_spread, snr);

    #pragma message("TODO FIXME, calculate SBAVG properly")
    double sbavg = 1.0;

    double sbd_error = MHO_BasicFringeInfo::calculate_sbd_error(sbd_sep, snr, sbavg);

    int total_naps = paramStore->GetAs<int>("total_naps");
    double drate_error = MHO_BasicFringeInfo::calculate_drate_error_v1(snr, ref_freq, total_naps, ap_delta);
    //may want to consider using this version in the future
    //double drate_error = calculate_drate_error_v2(snr, ref_freq, integration_time);

    paramStore->Set("/fringe/mbd_error", mbd_error);
    paramStore->Set("/fringe/sbd_error", sbd_error);
    paramStore->Set("/fringe/drate_error", drate_error);

    double ph_err = MHO_BasicFringeInfo::calculate_phase_error(sbavg, snr);
    double phdelay_err = MHO_BasicFringeInfo::calculate_phase_delay_error(sbavg, snr, ref_freq);

    paramStore->Set("/fringe/phase_error", ph_err);
    paramStore->Set("/fringe/phase_delay_error", phdelay_err);

    double ref_clock_off = paramStore->GetAs<double>("/ref_station/clock_offset_at_frt");
    double rem_clock_off = paramStore->GetAs<double>("/rem_station/clock_offset_at_frt");
    double ref_rate = paramStore->GetAs<double>("/ref_station/clock_rate");
    double rem_rate = paramStore->GetAs<double>("/rem_station/clock_rate");

    double clock_offset = rem_clock_off - ref_clock_off;
    double clock_rate =  rem_rate - ref_rate;

    paramStore->Set("/fringe/relative_clock_offset", clock_offset); //usec
    paramStore->Set("/fringe/relative_clock_rate", clock_rate*1e6); //usec/s
}


double 
MHO_BasicFringeUtilities::calculate_residual_phase(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    double total_summed_weights = paramStore->GetAs<double>("/fringe/total_summed_weights");
    double ref_freq = paramStore->GetAs<double>("ref_freq");
    double mbd = paramStore->GetAs<double>("/fringe/mbdelay");
    double drate = paramStore->GetAs<double>("/fringe/drate");
    double sbd = paramStore->GetAs<double>("/fringe/sbdelay");
    double sbd_max_bin = paramStore->GetAs<double>("/fringe/max_sbd_bin");
    double frt_offset = paramStore->GetAs<double>("frt_offset");
    double ap_delta =  paramStore->GetAs<double>("ap_period");

    auto weights = conStore->GetObject<weight_type>(std::string("weight"));
    auto sbd_arr = conStore->GetObject<visibility_type>(std::string("sbd"));

    std::size_t POLPROD = 0;
    std::size_t nchan = sbd_arr->GetDimension(CHANNEL_AXIS);
    std::size_t nap = sbd_arr->GetDimension(TIME_AXIS);

    //now we are going to loop over all of the channels/AP
    //and perform the weighted sum of the data at the max-SBD bin
    //with the fitted delay-rate rotation (but mbd=0) applied
    auto chan_ax = &( std::get<CHANNEL_AXIS>(*sbd_arr) );
    auto ap_ax = &(std::get<TIME_AXIS>(*sbd_arr));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*sbd_arr) );
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);

    paramStore->Set("/fringe/sbd_separation", sbd_delta);

    MHO_FringeRotation frot;
    frot.SetSBDSeparation(sbd_delta);
    frot.SetSBDMaxBin(sbd_max_bin);
    frot.SetNSBDBins(sbd_ax->GetSize()/4);  //this is nlags, FACTOR OF 4 is because sbd space is padded by a factor of 4
    frot.SetSBDMax( sbd );

    std::complex<double> sum_all = 0.0;
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
        MHO_IntervalLabel ilabel(ch,ch);
        std::string net_sideband = "?";
        std::string sidebandlabelkey = "net_sideband";
        auto other_labels = chan_ax->GetIntervalsWhichIntersect(&ilabel);
        for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
        {
            if( (*olit)->HasKey(sidebandlabelkey) )
            {
                (*olit)->Retrieve(sidebandlabelkey, net_sideband);
                break;
            }
        }

        frot.SetSideband(0); //DSB
        if(net_sideband == "U")
        {
            frot.SetSideband(1);
        }

        if(net_sideband == "L")
        {
            frot.SetSideband(-1);
        }

        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
            std::complex<double> vis = (*sbd_arr)(POLPROD, ch, ap, sbd_max_bin); //pick out data at SBD max bin
            std::complex<double> vr = frot.vrot(tdelta, freq, ref_freq, drate, mbd);
            std::complex<double> z = vis*vr;
            //apply weight and sum
            double w = (*weights)(POLPROD, ch, ap, 0);
            std::complex<double> wght_phsr = z*w;
            if(net_sideband == "U")
            {
                sum_all += -1.0*wght_phsr;
            }
            else
            {
                sum_all += wght_phsr;
            }
        }
    }

    double coh_avg_phase = std::arg(sum_all);
    return coh_avg_phase; //radians
}


void 
MHO_BasicFringeUtilities::basic_fringe_search(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    bool ok;
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("main", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }

    //temporarily organize the main fringe search in this function
    //TODO consolidate the coarse search in a single class, so it can be mixed-and-matched
    //with different interpolation schemes

    //output for the delay
    std::size_t bl_dim[visibility_type::rank::value];
    vis_data->GetDimensions(bl_dim);
    visibility_type* sbd_data = vis_data->CloneEmpty();
    bl_dim[FREQ_AXIS] *= 4; //normfx implementation demands this
    sbd_data->Resize(bl_dim);

    ////////////////////////////////////////////////////////////////////////////
    //COARSE SBD, DR, MBD SEARCH ALGO
    ////////////////////////////////////////////////////////////////////////////

    //run norm-fx via the wrapper class (x-form to SBD space)
    MHO_NormFX nfxOp;
    nfxOp.SetArgs(vis_data, wt_data, sbd_data);
    ok = nfxOp.Initialize();
    check_step_fatal(ok, "main", "normfx initialization." << eom );

    ok = nfxOp.Execute();
    check_step_fatal(ok, "main", "normfx execution." << eom );

    //take snapshot of sbd data after normfx
    take_snapshot_here("test", "sbd", __FILE__, __LINE__, sbd_data);

    //run the transformation to delay rate space (this also involves a zero padded FFT)
    MHO_DelayRate drOp;
    visibility_type* sbd_dr_data = sbd_data->CloneEmpty();
    double ref_freq = paramStore->GetAs<double>("ref_freq");
    drOp.SetReferenceFrequency(ref_freq);
    drOp.SetArgs(sbd_data, wt_data, sbd_dr_data);
    ok = drOp.Initialize();
    check_step_fatal(ok, "main", "dr initialization." << eom );
    ok = drOp.Execute();
    check_step_fatal(ok, "main", "dr execution." << eom );

    take_snapshot_here("test", "sbd_dr", __FILE__, __LINE__, sbd_dr_data);

    //coarse SBD/MBD/DR search (locates max bin)
    MHO_MBDelaySearch mbdSearch;
    mbdSearch.SetArgs(sbd_dr_data);
    ok = mbdSearch.Initialize();
    check_step_fatal(ok, "main", "mbd initialization." << eom );
    ok = mbdSearch.Execute();
    check_step_fatal(ok, "main", "mbd execution." << eom );

    int c_mbdmax = mbdSearch.GetMBDMaxBin();
    int c_sbdmax = mbdSearch.GetSBDMaxBin();
    int c_drmax = mbdSearch.GetDRMaxBin();
    double freq_spacing = mbdSearch.GetFrequencySpacing();
    double ave_freq = mbdSearch.GetAverageFrequency();


    paramStore->Set("/fringe/max_mbd_bin", c_mbdmax);
    paramStore->Set("/fringe/max_sbd_bin", c_sbdmax);
    paramStore->Set("/fringe/max_dr_bin", c_drmax);
    // paramStore->Set("/fringe/ambiguity", 1.0/freq_spacing);
    // paramStore->Set("/fringe/average_frequency", ave_freq);

    std::size_t n_mbd_pts = sbd_dr_data->GetDimension(CHANNEL_AXIS);
    std::size_t n_dr_pts = sbd_dr_data->GetDimension(TIME_AXIS);
    std::size_t n_sbd_pts = sbd_dr_data->GetDimension(FREQ_AXIS);
    paramStore->Set("/fringe/n_mbd_points", n_mbd_pts);
    paramStore->Set("/fringe/n_sbd_points", n_sbd_pts);
    paramStore->Set("/fringe/n_dr_points", n_dr_pts);

    ////////////////////////////////////////////////////////////////////////////
    //FINE INTERPOLATION STEP (search over 5x5x5 grid around peak)
    ////////////////////////////////////////////////////////////////////////////
    MHO_InterpolateFringePeak fringeInterp;

    bool optimize_closure_flag = false;
    bool is_oc_set = paramStore->Get(std::string("optimize_closure"), optimize_closure_flag );
    //NOTE, this has no effect on fringe-phase when using 'simul' algo
    //This is also true in the legacy code simul implementation, and which is
    //currently is the only one implemented
    if(optimize_closure_flag){fringeInterp.EnableOptimizeClosure();}

    fringeInterp.SetReferenceFrequency(ref_freq);
    fringeInterp.SetMaxBins(c_sbdmax, c_mbdmax, c_drmax);

    fringeInterp.SetSBDArray(sbd_data);
    fringeInterp.SetWeights(wt_data);

    #pragma message("TODO FIXME -- we shouldn't be referencing internal members of the MHO_MBDelaySearch class workspace")
    //Figure out how best to present this axis data to the fine-interp function.
    fringeInterp.SetMBDAxis( mbdSearch.GetMBDAxis() );
    fringeInterp.SetDRAxis( mbdSearch.GetDRAxis() );

    fringeInterp.Initialize();
    fringeInterp.Execute();

    double sbdelay = fringeInterp.GetSBDelay();
    double mbdelay = fringeInterp.GetMBDelay();
    double drate = fringeInterp.GetDelayRate();
    double frate = fringeInterp.GetFringeRate();
    double famp = fringeInterp.GetFringeAmplitude();

    paramStore->Set("/fringe/sbdelay", sbdelay);
    paramStore->Set("/fringe/mbdelay", mbdelay);
    paramStore->Set("/fringe/drate", drate);
    paramStore->Set("/fringe/frate", frate);
    paramStore->Set("/fringe/famp", famp);

    //add the sbd_data, and sbd_dr_data to the container store
    conStore->AddObject(sbd_data);
    conStore->AddObject(sbd_dr_data);
    conStore->SetShortName(sbd_data->GetObjectUUID(), "sbd");
    conStore->SetShortName(sbd_dr_data->GetObjectUUID(), "sbd_dr");
}


}//end namespace
