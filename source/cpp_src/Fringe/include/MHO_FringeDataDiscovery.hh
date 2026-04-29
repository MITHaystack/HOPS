#ifndef MHO_FringeDataDiscovery_HH__
#define MHO_FringeDataDiscovery_HH__

#include "hops_version.hh"

//utilities
#include "MHO_DirectoryInterface.hh"
#include "MHO_Message.hh"
#include "MHO_Profiler.hh"
#include "MHO_Tokenizer.hh"

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
 *@file MHO_FringeDataDiscovery.hh
 *@class MHO_FringeDataDiscovery
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 16:40:35 2023 -0400 Tue Sep 19 04:11:24 PM EDT 2023
 *@brief collection of helper functions for fringe fitter start-up
 */

/**
 * @brief Class MHO_FringeDataDiscovery
 */
class MHO_FringeDataDiscovery
{

    public:
        MHO_FringeDataDiscovery(){};
        virtual ~MHO_FringeDataDiscovery(){};

    public:
        /**
         * @brief Finds and returns the associated root (ovex) file in the given directory.
         *
         * @param dir Input directory path as a string.
         * @return The full path of the associated root (ovex) file as a string.
         * @note This is a static function.
         */
        static std::string find_associated_root_file(std::string dir);

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
         * @brief Parses a polarization product string to determine required pol-products.
         *
         * @param polprod Input polarization product string.
         * @return Vector of unique required polarization products.
         * @note This is a static function.
         */
        static std::vector< std::string > determine_required_pol_products(std::string polprod);
};

} // namespace hops

#endif /*! end of include guard: MHO_FringeDataDiscovery_HH__ */
