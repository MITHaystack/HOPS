#ifndef MHO_BasicFringeDataConfiguration_HH__
#define MHO_BasicFringeDataConfiguration_HH__

#include "hops_version.h"

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

        //parses the command line options and puts them in a parameter store object
        static int parse_fourfit_command_line(int argc, char** argv, MHO_ParameterStore* paramStore);

        //determines which directories are scan data that should be processed
        static void determine_scans(const std::string& initial_dir, std::vector< std::string >& scans, std::vector< std::string >& roots);
        
        //determines which baselines are present for each scan
        static void determine_baselines(const std::string& dir,
                                        const std::string& baseline,
                                        std::vector< std::pair< std::string, std::string > >& baseline_files);
                                        
        //determines what frequency groups and pol-products should be processed for each baseline
        static void determine_fgroups_polproducts(const std::string& filename,
                                            const std::string& cmd_fgroup,
                                            const std::string& cmd_pprod,
                                            std::vector< std::string >& fgroups,
                                            std::vector< std::string >& pprods );

        //loops over all data and constructs (concatenated strings) containing all of the pass information
        static void determine_passes(MHO_ParameterStore* cmdline_params, 
                                     std::string& cscans, 
                                     std::string& croots,
                                     std::string& cbaselines,
                                     std::string& cfgroups,
                                     std::string& cpolprods);
        
        //takes the (concatenated) strings from determine_passes, and breaks them into a vector of json objects 
        //describing the data item to be processed on each pass
        static void split_passes(std::vector<mho_json>& pass_vector,
                                 const std::string& cscans, 
                                 const std::string& croots,
                                 const std::string& cbaselines,
                                 const std::string& cfgroups,
                                 const std::string& cpolprods);

        //some post-command line parse initialization (populates the scan store)
        static bool initialize_scan_data(MHO_ParameterStore*, MHO_ScanDataStore* scanStore);
        static void populate_initial_parameters(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore);
        static std::vector< std::string > determine_required_pol_products(std::string polprod);

        //functions that are called within the fringe fitter class
        static void configure_visibility_data(MHO_ContainerStore* store);
        static void configure_station_data(MHO_ScanDataStore* scanStore,
                                           MHO_ContainerStore* containerStore,
                                           std::string ref_station_mk4id,
                                           std::string rem_station_mk4id);
                                           
        //initializes and executes (in order) all the operators associated with a category
        static void init_and_exec_operators(MHO_OperatorBuilderManager* build_manager, MHO_OperatorToolbox* opToolbox, const char* category);

        //dumps the profiler events into a json object
        static mho_json ConvertProfileEvents(std::vector< MHO_ProfileEvent >& events);

};

}//end namespace

#endif /*! end of include guard: MHO_BasicFringeDataConfiguration_HH__ */
