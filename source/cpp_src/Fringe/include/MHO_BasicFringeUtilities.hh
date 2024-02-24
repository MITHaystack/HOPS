#ifndef MHO_BasicFringeUtilities_HH__
#define MHO_BasicFringeUtilities_HH__


//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"

//helper functions
#include "MHO_BasicFringeInfo.hh"

// //initialization
// #include "MHO_OperatorBuilderManager.hh"

namespace hops
{

/*!
*@file MHO_BasicFringeUtilities.hh
*@class MHO_BasicFringeUtilities
*@author J. Barrettj - barrettj@mit.edu
*@date Tue Sep 19 04:11:24 PM EDT 2023
*@brief a collection of helper functions to organize fringe fitting
*/

class MHO_BasicFringeUtilities
{

    public:
        MHO_BasicFringeUtilities(){};
        virtual ~MHO_BasicFringeUtilities(){};

    public:
        //helper functions
        static void basic_fringe_search(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        static double calculate_residual_phase(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        static void calculate_fringe_solution_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, const mho_json& vexInfo);
        static void determine_sample_rate(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);

};

}//end namespace

#endif /*! end of include guard: MHO_BasicFringeUtilities_HH__ */
