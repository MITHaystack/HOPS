#ifndef MHO_InitialFringeInfo_HH__
#define MHO_InitialFringeInfo_HH__

/*
*File: MHO_InitialFringeInfo.hh
*Class: MHO_InitialFringeInfo
*Author:
*Email:
*Date: Tue Sep 19 04:11:24 PM EDT 2023
*Description: collection of helper functions to populate the parameter store 
* with a priori information know before fringe fitting
*/

//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops 
{

class MHO_InitialFringeInfo
{
        
    public:
        MHO_InitialFringeInfo(){};
        virtual ~MHO_InitialFringeInfo(){};
    
    public:
        static void set_default_parameters_minimal(MHO_ParameterStore* paramStore);
        static void configure_reference_frequency(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        static void calculate_freq_space(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        static void calculate_clock_model(MHO_ParameterStore* paramStore);
        static void precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        static void compute_total_summed_weights(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);


};

}//end namespace

#endif /* end of include guard: MHO_InitialFringeInfo_HH__ */
