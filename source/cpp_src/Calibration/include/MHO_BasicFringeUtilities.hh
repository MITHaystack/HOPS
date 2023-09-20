#ifndef MHO_BasicFringeUtilities_HH__
#define MHO_BasicFringeUtilities_HH__

/*
*File: MHO_BasicFringeUtilities.hh
*Class: MHO_BasicFringeUtilities
*Author:
*Email:
*Date: Tue Sep 19 04:11:24 PM EDT 2023
*Description: a collection of helper functions to organize fringe fitting
*/

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

class MHO_BasicFringeUtilities
{
        
    public:
        MHO_BasicFringeUtilities(){};
        virtual ~MHO_BasicFringeUtilities(){};
    
    public:
        //helper functions
        static int parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore);
        
        static void configure_data_library(MHO_ContainerStore* store);
        
        static void extract_clock_early(const mho_json& clk, double& clock_early, std::string& clock_early_units, double& clock_rate, std::string& clock_rate_units, std::string& origin, std::string& validity);
        static void extract_clock_model(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        static void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        
        static void calculate_freq_space(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        static void calculate_clock_model(MHO_ParameterStore* paramStore);
        static void precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        
        static void calculate_fringe_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, const mho_json& vexInfo);
        static void fill_plot_data(MHO_ParameterStore* paramStore, mho_json& plot_dict);
        
        
        
        

        // 
        // static void init_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category);

        static void basic_fringe_search(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        
        static mho_json construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, mho_json& vexInfo);

        static void set_default_parameters(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);

        static double calculate_residual_phase(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore){return 0.0;};

};

}//end namespace

#endif /* end of include guard: MHO_BasicFringeUtilities_HH__ */
