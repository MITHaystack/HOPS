#include "ffit.hh"

#include "MHO_Reducer.hh"
#include "MHO_DelayModel.hh"

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

    // //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    // double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    // std::cout<<"time midpoint = "<<midpoint_time<<std::endl;


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

    double ap_delay = delay_model.GetDelay();
    double ap_rate = delay_model.GetRate();
    double ap_accel = delay_model.GetAcceleration();

    paramStore->Set("/model/adelay", ap_delay);
    paramStore->Set("/model/arate", ap_rate);
    paramStore->Set("/model/aaccel", ap_accel);
    
    
}
