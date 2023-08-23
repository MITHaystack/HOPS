#include "ffit.hh"

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
}
