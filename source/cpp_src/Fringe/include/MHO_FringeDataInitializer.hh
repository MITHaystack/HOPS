#ifndef MHO_FringeDataInitializer_HH__
#define MHO_FringeDataInitializer_HH__

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
 *@file MHO_FringeDataInitializer.hh
 *@class MHO_FringeDataInitializer
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 16:40:35 2023 -0400 Tue Sep 19 04:11:24 PM EDT 2023
 *@brief collection of helper functions for fringe fitter start-up
 */

/**
 * @brief Class MHO_FringeDataInitializer
 */
class MHO_FringeDataInitializer
{

    public:
        MHO_FringeDataInitializer(){};
        virtual ~MHO_FringeDataInitializer(){};

    public:
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
};

} // namespace hops

#endif /*! end of include guard: MHO_FringeDataInitializer_HH__ */
