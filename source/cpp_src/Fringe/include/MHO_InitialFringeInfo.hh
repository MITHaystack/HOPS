#ifndef MHO_InitialFringeInfo_HH__
#define MHO_InitialFringeInfo_HH__


//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
*@file MHO_InitialFringeInfo.hh
*@class MHO_InitialFringeInfo
*@author J. Barrettj - barrettj@mit.edu
*@date Wed Sep 20 16:12:23 2023 -0400
*@brief collection of helper functions to populate the parameter store
* with a priori information know before fringe fitting
*/

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
        static void determine_n_active_channels(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);


};

}//end namespace

#endif /*! end of include guard: MHO_InitialFringeInfo_HH__ */
