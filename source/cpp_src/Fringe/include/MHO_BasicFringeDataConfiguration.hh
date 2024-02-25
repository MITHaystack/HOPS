#ifndef MHO_BasicFringeDataConfiguration_HH__
#define MHO_BasicFringeDataConfiguration_HH__


//global messaging util
#include "MHO_Message.hh"
#include "MHO_Profiler.hh"

//data/config passing classes
#include "MHO_ScanDataStore.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"

namespace hops
{

/*!
*@file MHO_BasicFringeDataConfiguration.hh
*@class MHO_BasicFringeDataConfiguration
*@author J. Barrettj - barrettj@mit.edu
*@date Wed Sep 20 16:40:35 2023 -0400 Tue Sep 19 04:11:24 PM EDT 2023
*@brief collection of helper functions for fringe fitter start-up
*/

class MHO_BasicFringeDataConfiguration
{

    public:
        MHO_BasicFringeDataConfiguration(){};
        virtual ~MHO_BasicFringeDataConfiguration(){};

    public:

        //functions for consuming command line arguments
        static void parse_baseline_freqgrp(std::string baseline_freqgrp, std::string& baseline, std::string& freqgrp);
        static std::string parse_set_string(const std::vector< std::string >& arglist, int& set_arg_index);
        static std::string sanitize_directory(std::string dir);
        static std::string find_associated_root_file(std::string dir);

        //sanity check of parameters after command line parsing
        static int sanity_check(MHO_ParameterStore* paramStore);

        //some post-command line parse initialization (populates the scan store)
        static void populate_initial_parameters(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore);
        static std::vector< std::string > determine_required_pol_products(std::string polprod);

        //functions that are called within the fringe fitter class
        static void configure_visibility_data(MHO_ContainerStore* store);
        static void configure_station_data(MHO_ScanDataStore* scanStore,
                                           MHO_ContainerStore* containerStore,
                                           std::string ref_station_mk4id,
                                           std::string rem_station_mk4id);
        static void init_and_exec_operators(MHO_OperatorBuilderManager* build_manager, MHO_OperatorToolbox* opToolbox, const char* category);

        static mho_json ConvertProfileEvents(std::vector< MHO_ProfileEvent >& events);

};

}//end namespace

#endif /*! end of include guard: MHO_BasicFringeDataConfiguration_HH__ */
