#include "MHO_InitialFringeInfo.hh"

//precalculate_quantities
#include "MHO_Clock.hh"
#include "MHO_DelayModel.hh"
#include "MHO_Reducer.hh"
#include "MHO_StationModel.hh"
#include "MHO_UniformGridPointsCalculator.hh"

namespace hops
{

void MHO_InitialFringeInfo::calculate_freq_space(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    visibility_type* vis = conStore->GetObject< visibility_type >(std::string("vis"));
    //calculate the frequency grid used in the MBD search
    MHO_UniformGridPointsCalculator fGridCalc;
    std::vector< double> in_freq_pts(std::get<CHANNEL_AXIS>(*vis).GetData(), 
                                     std::get<CHANNEL_AXIS>(*vis).GetData() + std::get<CHANNEL_AXIS>(*vis).GetSize() );
    std::vector< double > freq_pts;
    double freq_eps = 1e-4; //tolerance of 0.1kHz
    //dsb channel pairs share a sky_freq so we need combine them at the same location
    //this eliminates non-unique (within the tolerance) adjacent frequencies
    std::map<std::size_t, std::size_t> index_map;
    fGridCalc.GetUniquePoints(in_freq_pts, freq_eps, freq_pts, index_map);
    fGridCalc.SetPoints(freq_pts);
    fGridCalc.Calculate();

    double gridStart = fGridCalc.GetGridStart();
    double gridSpace = fGridCalc.GetGridSpacing();
    double ambig = 1.0 / gridSpace;
    double nGridPoints = fGridCalc.GetNGridPoints();
    double averageFreq = fGridCalc.GetGridAverage();
    double freqSpread = fGridCalc.GetSpread();

    //correct the frequency spread if we only have 1 channel
    //the number of channels present after cuts
    int nchan = paramStore->GetAs< int >("/config/nchannels");
    double channel_bandwidth = paramStore->GetAs< double >("/config/channel_bandwidth");

    if(nchan == 1)
    {
        freqSpread = channel_bandwidth / std::sqrt(12.0); //uniform distribution
    }

    paramStore->Set("/fringe/start_frequency", gridStart);
    paramStore->Set("/fringe/frequency_spacing", gridSpace);
    paramStore->Set("/fringe/ambiguity", ambig);
    paramStore->Set("/fringe/n_frequency_points", nGridPoints);
    paramStore->Set("/fringe/average_frequency", averageFreq);
    paramStore->Set("/fringe/frequency_spread", freqSpread);
}

void MHO_InitialFringeInfo::calculate_clock_model(MHO_ParameterStore* paramStore)
{
    std::string frt_string = paramStore->GetAs< std::string >("/vex/scan/fourfit_reftime");
    auto frt_tp = hops_clock::from_vex_format(frt_string);

    double ref_offset = paramStore->GetAs< double >("/ref_station/clock_early_offset");
    double ref_rate = paramStore->GetAs< double >("/ref_station/clock_rate");
    std::string ref_validity_epoch = paramStore->GetAs< std::string >("/ref_station/clock_validity");
    std::string ref_origin_epoch = paramStore->GetAs< std::string >("/ref_station/clock_origin");

    auto ref_origin_tp = frt_tp;
    if(ref_origin_epoch != "")
    {
        ref_origin_tp = hops_clock::from_vex_format(ref_origin_epoch);
    }

    double rem_offset = paramStore->GetAs< double >("/rem_station/clock_early_offset");
    double rem_rate = paramStore->GetAs< double >("/rem_station/clock_rate");
    std::string rem_validity_epoch = paramStore->GetAs< std::string >("/rem_station/clock_validity");
    std::string rem_origin_epoch = paramStore->GetAs< std::string >("/rem_station/clock_origin");

    auto rem_origin_tp = frt_tp;
    if(rem_origin_epoch != "")
    {
        rem_origin_tp = hops_clock::from_vex_format(rem_origin_epoch);
    }

    //TODO ALSO ENSURE THAT THE VALIDITY EPOCH IS BEFORE THE FRT
    double refdiff = 0.0;
    if(ref_rate != 0.0)
    {
        auto ref_tdiff_duration = frt_tp - ref_origin_tp;
        refdiff = std::chrono::duration< double >(ref_tdiff_duration).count();
    }
    if(refdiff > 3.0e5)
    {
        msg_info("main", "reference station clockrate epoch: " << hops_clock::to_iso8601_format(ref_origin_tp)
                                                               << ", is highly discrepant from FRT: "
                                                               << hops_clock::to_iso8601_format(frt_tp) << eom);
    }

    double remdiff = 0.0;
    if(rem_rate != 0.0)
    {
        auto rem_tdiff_duration = frt_tp - rem_origin_tp;
        remdiff = std::chrono::duration< double >(rem_tdiff_duration).count();
    }

    if(remdiff > 3.0e5)
    {
        msg_info("main", "remote station clockrate epoch: " << hops_clock::to_iso8601_format(rem_origin_tp)
                                                            << ", is highly discrepant from FRT: "
                                                            << hops_clock::to_iso8601_format(frt_tp) << eom);
    }

    double ref_clock = ref_offset + (refdiff * ref_rate) * 1e6; //in usec!
    double rem_clock = rem_offset + (remdiff * rem_rate) * 1e6; //in usec!

    paramStore->Set("/ref_station/clock_offset_at_frt", ref_clock);
    paramStore->Set("/rem_station/clock_offset_at_frt", rem_clock);
}

void MHO_InitialFringeInfo::compute_total_summed_weights(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    weight_type* wt_data = conStore->GetObject< weight_type >(std::string("weight"));
    if(wt_data == nullptr)
    {
        msg_fatal("main", "could not find weight object with name: weight." << eom);
        std::exit(1);
    }

    //compute the sum of the data weights
    weight_type temp_weights;
    temp_weights.Copy(*wt_data);
    MHO_Reducer< weight_type, MHO_CompoundSum > wt_reducer;
    wt_reducer.SetArgs(&temp_weights);
    for(std::size_t i = 0; i < weight_type::rank::value; i++)
    {
        wt_reducer.ReduceAxis(i);
    }
    wt_reducer.Initialize();
    wt_reducer.Execute();
    double total_ap_frac = temp_weights[0];

    paramStore->Set("/fringe/total_summed_weights", total_ap_frac);
    wt_data->Insert("total_summed_weights", total_ap_frac);
}

void MHO_InitialFringeInfo::determine_n_active_channels(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    //just loop over the weights and count the number of channels which contain an AP with weight > 0
    weight_type* wt_data = conStore->GetObject< weight_type >(std::string("weight"));
    if(wt_data == nullptr)
    {
        msg_fatal("main", "could not find weight object with name: weight." << eom);
        std::exit(1);
    }

    std::size_t n_active_channels = 0;
    auto chan_ax = &(std::get< CHANNEL_AXIS >(*wt_data));
    for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
    {
        auto slice = wt_data->SliceView(":", ch, ":", ":");
        double sum = 0.0;
        for(auto it = slice.begin(); it != slice.end(); it++)
        {
            sum += *it;
        }
        if(sum > 0.0)
        {
            n_active_channels++;
        }
    }

    paramStore->Set("/fringe/n_active_channels", n_active_channels);
}

void MHO_InitialFringeInfo::precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    visibility_type* vis_data = conStore->GetObject< visibility_type >(std::string("vis"));
    if(vis_data == nullptr)
    {
        msg_fatal("main", "could not find visibility object with name: vis." << eom);
        std::exit(1);
    }

    auto ap_ax = &(std::get< TIME_AXIS >(*vis_data));
    if(ap_ax->GetSize() <= 1)
    {
        msg_fatal("main", "could not determine AP period for data since AP axis is of size: " << ap_ax->GetSize() << eom);
        std::exit(1);
    }

    bool ok = false;
    //retrieve the station codes
    std::string ref_code;
    ok = vis_data->Retrieve("reference_station", ref_code);
    if(!ok)
    {
        ref_code = "";
    }
    paramStore->Set("/config/reference_station", ref_code);

    std::string rem_code;
    ok = vis_data->Retrieve("remote_station", rem_code);
    if(!ok)
    {
        rem_code = "";
    }
    paramStore->Set("/config/remote_station", rem_code);

    //retrieve the root code from the visib (only needed if we export to mk4)
    std::string root_code;
    ok = vis_data->Retrieve("root_code", root_code);
    if(!ok)
    {
        root_code = "XXXXXX";
    }
    paramStore->Set("/config/root_code", root_code); //should this get stuffed in config?

    //retrieve the correlation processing date from the visibilities
    std::string corr_date;
    ok = vis_data->Retrieve("correlation_date", corr_date);
    if(!ok)
    {
        corr_date = "";
    }
    paramStore->Set("/config/correlation_date", corr_date); //should this get stuffed in config?

    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    paramStore->Set("/config/ap_period", ap_delta);

    //append info about the total number of APs
    int naps = ap_ax->GetSize();
    paramStore->Set("/config/total_naps", naps);

    //append info about the number of lags
    auto freq_ax = &(std::get< FREQ_AXIS >(*vis_data));
    int nlags = 2 * freq_ax->GetSize(); //factor of 2 is a properly of padding used in the normFX routine
    paramStore->Set("/config/nlags", nlags);

    //grab the channel bandwidth (assume to be the same for all channels)
    double bandwidth = 0;
    auto chan_ax = &(std::get< CHANNEL_AXIS >(*vis_data));
    std::string bandwidthlabelkey = "bandwidth";
    bool bwkey_present = chan_ax->RetrieveIndexLabelKeyValue(0, bandwidthlabelkey, bandwidth);
    if(bandwidth == 0 || !bwkey_present)
    {
        msg_error("fringe", "channel-0 missing bandwidth label value." << eom);
    }
    paramStore->Set("/config/channel_bandwidth", bandwidth);

    //offset to the start of the data
    double start_offset = ap_ax->at(0);
    paramStore->Set("start_offset", start_offset);

    //offset to the start of the data
    double stop_offset = ap_ax->at(ap_ax->GetSize() - 1) + ap_delta;
    paramStore->Set("stop_offset", stop_offset);

    //the number of channels present after cuts
    int nchan = chan_ax->GetSize();
    paramStore->Set("/config/nchannels", nchan);

    //compute the a priori delay model
    station_coord_type* ref_data = conStore->GetObject< station_coord_type >(std::string("ref_sta"));
    station_coord_type* rem_data = conStore->GetObject< station_coord_type >(std::string("rem_sta"));
    MHO_DelayModel delay_model;
    std::string frt_vex_string = paramStore->GetAs< std::string >("/vex/scan/fourfit_reftime");
    double ref_clockoff = paramStore->GetAs< double >("/ref_station/clock_early_offset"); //usec
    double ref_clockrate = paramStore->GetAs< double >("/ref_station/clock_rate");

    delay_model.SetFourfitReferenceTimeVexString(frt_vex_string);
    delay_model.SetReferenceStationData(ref_data);
    delay_model.SetRemoteStationData(rem_data);
    delay_model.SetReferenceStationClockOffset(ref_clockoff * 1e-6);
    delay_model.SetReferenceStationClockRate(ref_clockrate);

    delay_model.ComputeModel();

    //calculate the offset to the reference time (within the scan)
    auto frt = hops_clock::from_vex_format(frt_vex_string);
    std::string start_vex_string = paramStore->GetAs< std::string >("/vex/scan/start");
    auto start_time = hops_clock::from_vex_format(start_vex_string);
    auto offset_to_frt_duration = frt - start_time;
    double frt_offset = std::chrono::duration< double >(offset_to_frt_duration).count();
    paramStore->Set("/config/frt_offset", frt_offset);

    //calculate the data start and stop dates (including the start/stop offsets)
    int64_t start_offset_ns = 1e9 * start_offset;
    int64_t stop_offset_ns = 1e9 * stop_offset;
    auto data_start_tp = start_time + std::chrono::nanoseconds(start_offset_ns);
    auto data_stop_tp = start_time + std::chrono::nanoseconds(stop_offset_ns);
    std::string data_start_vex = hops_clock::to_vex_format(data_start_tp);
    std::string data_stop_vex = hops_clock::to_vex_format(data_stop_tp);
    paramStore->Set("/fringe/start_date", data_start_vex); //store as vex formatted string
    paramStore->Set("/fringe/stop_date", data_stop_vex);

    double adelay = delay_model.GetDelay();
    double arate = delay_model.GetRate();
    double aaccel = delay_model.GetAcceleration();

    double ref_adelay = delay_model.GetRefDelay();
    double ref_arate = delay_model.GetRefRate();
    double ref_stn_delay = delay_model.GetRefStationDelay();

    //store and  convert to microsec
    //TODO FIXME - document units of all the various parameters/quantities
    paramStore->Set("/model/adelay", 1e6 * adelay);
    paramStore->Set("/model/arate", 1e6 * arate);
    paramStore->Set("/model/aaccel", 1e6 * aaccel);
    paramStore->Set("/model/ref_adelay", 1e6 * ref_adelay);
    paramStore->Set("/model/ref_arate", 1e6 * ref_arate);
    paramStore->Set("/model/ref_station_delay", ref_stn_delay); //kept in units of sec

    //figure out the clock information at the FRT
    calculate_clock_model(paramStore);

    //determines properties of the frequency grid used in the MBD search
    calculate_freq_space(conStore, paramStore);

    //figure out the reference station's coordinates (az, el, par_angle, u, v, w)
    //these are all evaluated at the fourfit reference time...with the exception of parallactic_angle
    //since parallactic angle is not given as a spline (this is just the value at the beginning of the scan)
    MHO_StationModel ref_model;
    ref_model.SetEvaluationTimeVexString(frt_vex_string);
    ref_model.SetStationData(ref_data);
    ref_model.ComputeModel();
    paramStore->Set("/ref_station/azimuth", ref_model.GetAzimuth());
    paramStore->Set("/ref_station/elevation", ref_model.GetElevation());
    paramStore->Set("/ref_station/parallactic_angle", ref_model.GetParallacticAngle());
    paramStore->Set("/ref_station/u", ref_model.GetUCoordinate());
    paramStore->Set("/ref_station/v", ref_model.GetVCoordinate());
    paramStore->Set("/ref_station/w", ref_model.GetWCoordinate());

    //figure out the remote station's coordinates (az, el, par_angle, u, v, w)
    MHO_StationModel rem_model;
    rem_model.SetEvaluationTimeVexString(frt_vex_string);
    rem_model.SetStationData(rem_data);
    rem_model.ComputeModel();
    paramStore->Set("/rem_station/azimuth", rem_model.GetAzimuth());
    paramStore->Set("/rem_station/elevation", rem_model.GetElevation());
    paramStore->Set("/rem_station/parallactic_angle", rem_model.GetParallacticAngle());
    paramStore->Set("/rem_station/u", rem_model.GetUCoordinate());
    paramStore->Set("/rem_station/v", rem_model.GetVCoordinate());
    paramStore->Set("/rem_station/w", rem_model.GetWCoordinate());
}

void MHO_InitialFringeInfo::set_default_parameters_minimal(MHO_ParameterStore* paramStore)
{
    //default mbd_anchor is model (instead of sbd)
    std::string mbd_anchor = "model";
    paramStore->Set("/control/config/mbd_anchor", mbd_anchor);
};

void MHO_InitialFringeInfo::configure_reference_frequency(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    //check if the parameter store already has a reference frequency set, and if not set the default
    if(!(paramStore->IsPresent("/control/config/ref_freq")))
    {
        //grab the visibility data, so we can determine the default reference frequency
        visibility_type* vis_data = conStore->GetObject< visibility_type >(std::string("vis"));

        //the first frequency in the array serves as the reference frequency if this value remains unset in the control file
        double first_freq = std::get< CHANNEL_AXIS >(*vis_data)(0);
        paramStore->Set("/control/config/ref_freq", first_freq);
    }

    // if( (paramStore->IsPresent("ref_freq") ) )
    // {
    //     double ref_freq = paramStore->GetAs<double>("ref_freq");
    //     paramStore->Set("/control/config/ref_freq", ref_freq);
    // }
};

} // namespace hops
