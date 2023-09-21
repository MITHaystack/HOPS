#ifndef MHO_BasicFringeDataConfiguration_HH__
#define MHO_BasicFringeDataConfiguration_HH__

/*
*File: MHO_BasicFringeDataConfiguration.hh
*Class: MHO_BasicFringeDataConfiguration
*Author:
*Email:
*Date: Tue Sep 19 04:11:24 PM EDT 2023
*Description: collection of helper functions for fringe fitter start-up
*/

//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"

namespace hops 
{

class MHO_BasicFringeDataConfiguration
{
        
    public:
        MHO_BasicFringeDataConfiguration(){};
        virtual ~MHO_BasicFringeDataConfiguration(){};
    
    public:
        //helper functions
        static int parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore);
        static void configure_data_library(MHO_ContainerStore* store);
        static void init_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category);
};

}//end namespace

#endif /* end of include guard: MHO_BasicFringeDataConfiguration_HH__ */
