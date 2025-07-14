#ifndef MHO_BasicFringeDataConfiguration_HH__
#define MHO_BasicFringeDataConfiguration_HH__

#include "hops_version.hh"

//global messaging util
#include "MHO_Message.hh"
#include "MHO_Profiler.hh"

//data/config passing classes
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_ScanDataStore.hh"

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

/**
 * @brief Class MHO_BasicFringeDataConfiguration
 */
class MHO_BasicFringeDataConfiguration
{

    public:
        MHO_BasicFringeDataConfiguration(){};
        virtual ~MHO_BasicFringeDataConfiguration(){};

    public:

        /**
         * @brief Parses baseline_freqgrp string into separate baseline and freqgrp strings (expects ':' as separator).
         * 
         * @param baseline_freqgrp Input string containing baseline and frequency group information.
         * @param baseline (std::string&)
         * @param freqgrp (std::string&)
         * @note This is a static function.
         */
        static void parse_baseline_freqgrp(std::string baseline_freqgrp, std::string& baseline, std::string& freqgrp);
        
        /**
         * @brief Parses a vector of strings to extract and concatenate control file syntax after 'set' command.
         * 
         * @param arglist Input vector of strings containing commands and arguments
         * @param set_arg_index Output index of 'set' command position in arglist
         * @return Concatenated string of control file syntax after 'set' command
         * @note This is a static function.
         */
        static std::string parse_set_string(const std::vector< std::string >& arglist, int& set_arg_index);
        
        /**
         * @brief Sanitizes a directory path by ensuring it ends with '/' and exists.
         * 
         * @param dir Input directory path to sanitize.
         * @return Sanitized directory path (preserves symlinks).
         * @note This is a static function.
         */
        static std::string sanitize_directory(std::string dir);
        
        /**
         * @brief Finds and returns the associated root (ovex) file in the given directory.
         * 
         * @param dir Input directory path as a string.
         * @return The full path of the associated root (ovex) file as a string.
         * @note This is a static function.
         */
        static std::string find_associated_root_file(std::string dir);

        /**
         * @brief Performs a sanity check on command line parameters after parsing.
         * 
         * @param paramStore Pointer to MHO_ParameterStore containing parsed command line arguments.
         * @return 0 if parameters are valid, 1 otherwise.
         * @note This is a static function.
         */
        static int sanity_check(MHO_ParameterStore* paramStore);


        /**
         * @brief Parses command line arguments and stores them in a parameter store for later use by fourfit
         * 
         * @param argc Number of command line arguments
         * @param argv Array of command line argument strings
         * @param paramStore Pointer to MHO_ParameterStore object to store parsed arguments
         * @return 0 if successful, non-zero otherwise
         * @note This is a static function.
         */
        static int parse_fourfit_command_line(int argc, char** argv, MHO_ParameterStore* paramStore);

        /**
         * @brief Determines scan directories and associated root files for processing.
         * 
         * @param initial_dir Initial directory to start search.
         * @param scans Output vector of scan directories found.
         * @param roots Output vector of associated root files.
         * @note This is a static function.
         */
        static void determine_scans(const std::string& initial_dir, std::vector< std::string >& scans,
                                    std::vector< std::string >& roots);

        /**
         * @brief Determines baselines present for each scan in a given directory matching a specified baseline pattern.
         * 
         * @param dir Input directory path
         * @param baseline Baseline pattern to match (2-char code or wildcard)
         * @param baseline_files Output vector of baseline-file pairs
         * @note This is a static function.
         */
        static void determine_baselines(const std::string& dir, const std::string& baseline,
                                        std::vector< std::pair< std::string, std::string > >& baseline_files);

        /**
         * @brief Determines frequency groups and polarization products to process for each baseline from a given filename.
         * 
         * @param filename Input filename containing (visibility) data
         * @param cmd_fgroup Command line argument for frequency group filter
         * @param cmd_pprod Command line argument for polarization product filter
         * @param fgroups Output vector of frequency groups to process
         * @param pprods Output vector of polarization products to process
         * @note This is a static function.
         */
        static void determine_fgroups_polproducts(const std::string& filename, const std::string& cmd_fgroup,
                                                  const std::string& cmd_pprod, std::vector< std::string >& fgroups,
                                                  std::vector< std::string >& pprods);

        //loops over all data and constructs (concatenated strings) containing all of the pass information
        /**
         * @brief Determines and concatenates data passes for processing, including scans, baselines, frequency groups, and polarization products.
         * 
         * @param cmdline_params Pointer to MHO_ParameterStore containing command line parameters
         * @param cscans Reference to string storing concatenated scan directories
         * @param croots Reference to string storing concatenated associated root files
         * @param cbaselines Reference to string storing concatenated baseline files
         * @param cfgroups Reference to string storing concatenated frequency groups
         * @param cpolprods Reference to string storing concatenated polarization products
         * @note This is a static function.
         */
        static void determine_passes(MHO_ParameterStore* cmdline_params, std::string& cscans, std::string& croots,
                                     std::string& cbaselines, std::string& cfgroups, std::string& cpolprods);

        //takes the (concatenated) strings from determine_passes, and breaks them into a vector of json objects
        //describing the data item(s) to be processed on each pass
        /**
         * @brief Breaks concatenated strings into vector of json objects describing data items for each processing pass.
         * 
         * @param pass_vector Output vector of mho_json objects representing data passes
         * @param cscans Concatenated string of scan directories
         * @param croots Concatenated string of root (ovex) files
         * @param cbaselines Concatenated string of baselines
         * @param cfgroups Concatenated string of frequency groups 
         * @param cpolprods Concatenated string of polarization product
         * @note This is a static function.
         */
        static void split_passes(std::vector< mho_json >& pass_vector, const std::string& cscans, const std::string& croots,
                                 const std::string& cbaselines, const std::string& cfgroups, const std::string& cpolprods);

        /**
         * @brief Initializes scan data store using parameters and sets root file name.
         * 
         * @param param1 Pointer to MHO_ParameterStore containing initialization parameters
         * @param scanStore Pointer to MHO_ScanDataStore to be initialized
         * @return True if scan store is valid, false otherwise
         * @note This is a static function.
         */
        static bool initialize_scan_data(MHO_ParameterStore*, MHO_ScanDataStore* scanStore);
        
        /**
         * @brief Initializes parameter store and scan store for fringe processing, sets initial values
         * 
         * @param paramStore Pointer to MHO_ParameterStore object for storing parameters
         * @param scanStore Pointer to MHO_ScanDataStore object for handling scan data
         * @note This is a static function.
         */
        static void populate_initial_parameters(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore);
        
        /**
         * @brief Parses a polarization product string to determine required pol-products.
         * 
         * @param polprod Input polarization product string.
         * @return Vector of unique required polarization products.
         * @note This is a static function.
         */
        static std::vector< std::string > determine_required_pol_products(std::string polprod);

        //functions that are called within the fringe fitter class
        /**
         * @brief Configures visibility data by checking and processing visibility_type and weight_type objects in the store.
         * 
         * @param store Pointer to MHO_ContainerStore containing visibility and weight data.
         * @note This is a static function.
         */
        static void configure_visibility_data(MHO_ContainerStore* store);
        
        /**
         * @brief Configures station data by loading and renaming objects in MHO_ScanDataStore and MHO_ContainerStore.
         * 
         * @param scanStore Pointer to MHO_ScanDataStore for loading station data
         * @param containerStore Pointer to MHO_ContainerStore for renaming objects
         * @param ref_station_mk4id Reference station's mk4id for loading and renaming
         * @param rem_station_mk4id Remote station's mk4id for loading and renaming
         * @note This is a static function.
         */
        static void configure_station_data(MHO_ScanDataStore* scanStore, MHO_ContainerStore* containerStore,
                                           std::string ref_station_mk4id, std::string rem_station_mk4id);

        /**
         * @brief Initializes and executes (in priority value order) all operators associated with a given category.
         * 
         * @param build_manager Pointer to MHO_OperatorBuilderManager for building operators.
         * @param opToolbox Pointer to MHO_OperatorToolbox for getting operators by category.
         * @param category Category name as string.
         * @note This is a static function.
         */
        static void init_and_exec_operators(MHO_OperatorBuilderManager* build_manager, MHO_OperatorToolbox* opToolbox,
                                            const char* category);

        /**
         * @brief Converts a vector of profile events into a JSON object.
         * 
         * @param events Input vector of MHO_ProfileEvent objects
         * @return JSON object containing event details
         * @note This is a static function.
         */
        static mho_json ConvertProfileEvents(std::vector< MHO_ProfileEvent >& events);
};

} // namespace hops

#endif /*! end of include guard: MHO_BasicFringeDataConfiguration_HH__ */
