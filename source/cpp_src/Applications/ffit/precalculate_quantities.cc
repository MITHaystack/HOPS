#include "ffit.hh"

#include "MHO_Reducer.hh"
#include "MHO_DelayModel.hh"
#include "MHO_Clock.hh"


void calculate_clock_model(MHO_ParameterStore* paramStore)
{
    std::string frt_string = paramStore->GetAs<std::string>("/vex/scan/fourfit_reftime");
    auto frt_tp = hops_clock::from_vex_format(frt_string);


    double ref_offset = paramStore->GetAs<double>("/ref_station/clock_early_offset");
    double ref_rate = paramStore->GetAs<double>("/ref_station/clock_rate");
    std::string ref_validity_epoch = paramStore->GetAs<std::string>("/ref_station/clock_validity");
    std::string ref_origin_epoch = paramStore->GetAs<std::string>("/ref_station/clock_origin");
    
    auto ref_origin_tp = frt_tp;
    if(ref_origin_epoch != ""){ref_origin_tp = hops_clock::from_vex_format(ref_origin_epoch);}

    double rem_offset = paramStore->GetAs<double>("/rem_station/clock_early_offset");
    double rem_rate = paramStore->GetAs<double>("/rem_station/clock_rate");
    std::string rem_validity_epoch = paramStore->GetAs<std::string>("/rem_station/clock_validity");
    std::string rem_origin_epoch = paramStore->GetAs<std::string>("/rem_station/clock_origin");

    auto rem_origin_tp = frt_tp;
    if(rem_origin_epoch != ""){rem_origin_tp = hops_clock::from_vex_format(rem_origin_epoch);}


    // "ref_station": {
    // "antenna_ref": "ALMA",
    // "clock_early_offset": 2107.147,
    // "clock_early_offset_units": "usec",
    // "clock_origin": "2021y098d23h52m00s",
    // "clock_rate": 7e-15,
    // "clock_ref": "Aa",
    // "clock_validity": "2021y104d12h28m00s",

    //TODO ALSO ENSURE THAT THE VALIDITY EPOCH IS BEFORE THE FRT


    double refdiff = 0.0;
    if(ref_rate != 0.0)
    {
        auto ref_tdiff_duration = frt_tp - ref_origin_tp;
        refdiff = std::chrono::duration<double>(ref_tdiff_duration).count(); 
    }
    if(refdiff > 3.0e5)
    {
        msg_info("main", "reference station clockrate epoch: " <<
        hops_clock::to_iso8601_format(ref_origin_tp) << 
        ", is highly discrepant from FRT: " << hops_clock::to_iso8601_format(frt_tp) << eom );
    }
    
    double remdiff = 0.0;
    if(rem_rate != 0.0)
    {
        auto rem_tdiff_duration = frt_tp - rem_origin_tp;
        remdiff = std::chrono::duration<double>(rem_tdiff_duration).count(); 
    }
    
    if(remdiff > 3.0e5)
    {
        msg_info("main", "remote station clockrate epoch: " <<
        hops_clock::to_iso8601_format(rem_origin_tp) << 
        ", is highly discrepant from FRT: " << hops_clock::to_iso8601_format(frt_tp) << eom );
    }

    double ref_clock = ref_offset + (refdiff*ref_rate)*1e6; //in usec!
    double rem_clock = rem_offset + (remdiff*rem_rate)*1e6; //in usec!

    paramStore->Set("/ref_station/clock_offset_at_frt", ref_clock);
    paramStore->Set("/rem_station/clock_offset_at_frt", rem_clock);
}


//calculate useful quantities used later throughout the program
void precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)
{
    visibility_type* vis_data = conStore->GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore->GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr || wt_data == nullptr )
    {
        msg_fatal("main", "could not find visibility or weight objects with names (vis, weight)." << eom);
        std::exit(1);
    }

    auto ap_ax = &(std::get<TIME_AXIS>(*vis_data));
    if(ap_ax->GetSize() <= 1)
    {
        msg_fatal("main", "could not determine AP period for data." << eom);
        std::exit(1);
    }

    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    paramStore->Set("ap_period", ap_delta);

    //offset to the start of the data 
    double start_offset = ap_ax->at(0);
    std::cout<<"offset to the start of the first ap = "<<start_offset<<std::endl;
    paramStore->Set("start_offset", start_offset);
    
    //offset to the start of the data 
    double stop_offset = ap_ax->at( ap_ax->GetSize()-1 ) + ap_delta;
    std::cout<<"offset to end of the last ap = "<<stop_offset<<std::endl;
    paramStore->Set("stop_offset", stop_offset);

    //the number of channels present after cuts 
    int nchan = std::get<CHANNEL_AXIS>(*vis_data).GetSize();
    paramStore->Set("nchannels", nchan);

    //compute the sum of the data weights
    weight_type temp_weights;
    temp_weights.Copy(*wt_data);
    MHO_Reducer<weight_type, MHO_CompoundSum> wt_reducer;
    wt_reducer.SetArgs(&temp_weights);
    for(std::size_t i=0; i<weight_type::rank::value; i++)
    {
        wt_reducer.ReduceAxis(i);
    }
    wt_reducer.Initialize();
    wt_reducer.Execute();
    double total_ap_frac = temp_weights[0];
    std::cout<<"reduced weights = "<<temp_weights[0]<<std::endl;
    paramStore->Set("/fringe/total_summed_weights", total_ap_frac);
    wt_data->Insert("total_summed_weights", total_ap_frac);
    
    //compute the a priori delay model
    station_coord_type* ref_data = conStore->GetObject<station_coord_type>(std::string("ref_sta"));
    station_coord_type* rem_data = conStore->GetObject<station_coord_type>(std::string("rem_sta"));
    MHO_DelayModel delay_model;
    std::string frt_vex_string = paramStore->GetAs<std::string>("/vex/scan/fourfit_reftime");
    delay_model.SetFourfitReferenceTimeVexString(frt_vex_string);
    delay_model.SetReferenceStationData(ref_data);
    delay_model.SetRemoteStationData(rem_data);
    delay_model.ComputeModel();
    
    //calculate the offset to the refence time (within the scan)
    auto frt = hops_clock::from_vex_format(frt_vex_string);
    std::string start_vex_string = paramStore->GetAs<std::string>("/vex/scan/start");
    auto start_time = hops_clock::from_vex_format(start_vex_string);
    auto offset_to_frt_duration = frt - start_time;
    double frt_offset = std::chrono::duration<double>(offset_to_frt_duration).count(); 
    paramStore->Set("frt_offset", frt_offset);
    
    double adelay = delay_model.GetDelay();
    double arate = delay_model.GetRate();
    double aaccel = delay_model.GetAcceleration();

    //store and  convert to microsec 
    //TODO FIXME - document units of all the various parameters/quantities
    paramStore->Set("/model/adelay", 1e6*adelay);
    paramStore->Set("/model/arate", 1e6*arate);
    paramStore->Set("/model/aaccel", 1e6*aaccel);
    
    //figureout the clock information at the FRT
    calculate_clock_model(paramStore);
}
