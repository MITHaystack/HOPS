#ifndef MHO_BasicFringeUtilities_HH__
#define MHO_BasicFringeUtilities_HH__

//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ParameterStore.hh"

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
 *@date Wed Sep 20 15:37:46 2023 -0400
 *@brief a collection of helper functions to organize fringe fitting
 */

/**
 * @brief Class MHO_BasicFringeUtilities
 */
class MHO_BasicFringeUtilities
{

    public:
        MHO_BasicFringeUtilities(){};
        virtual ~MHO_BasicFringeUtilities(){};

    public:
        //helper functions
        /**
         * @brief Calculates and returns the average sideband value from visibility data.
         *
         * @param conStore Input container store for accessing visibility data.
         * @param paramStore Parameter store (not used in this function)
         * @return Average sideband value as a double.
         * @note This is a static function.
         */
        static double calculate_sbavg(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);

        /**
         * @brief Calculates residual phase using data extracted from container and parameter stores.
         *
         * @param conStore Input MHO_ContainerStore for data access
         * @param paramStore Input MHO_ParameterStore for configuration values
         * @return Residual phase as a double value
         * @note This is a static function.
         */
        static double calculate_residual_phase(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);

        /**
         * @brief Calculates fringe solution info using data extracted from container store, parameter store, and vexInfo.
         *
         * @param conStore MHO_ContainerStore pointer for accessing container data
         * @param paramStore MHO_ParameterStore pointer for accessing parameter data
         * @param vexInfo const mho_json& containing vex information
         * @note This is a static function.
         */
        static void calculate_fringe_solution_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore,
                                                   const mho_json& vexInfo);

        /**
         * @brief Determines and updates sample rate and period using channel bandwidth info from visibilities.
         *
         * @param conStore Input container store for accessing visibility data.
         * @param paramStore Input/Output parameter store for retrieving and updating sample rate and period.
         * @note This is a static function.
         */
        static void determine_sample_rate(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);

        /**
         * @brief Calculates bandwidth correction factor for SNR using MHO_ContainerStore and MHO_ParameterStore data.
         *
         * @param conStore Input container store containing visibility and weight data
         * @param paramStore Input parameter store containing configuration settings
         * @return Bandwidth correction factor as a double value
         * @note This is a static function.
         */
        static double calculate_snr_correction_factor(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);

        /**
         * @brief Calculates ionospheric covariance matrix using data from container and parameter stores.
         *
         * @param conStore MHO_ContainerStore containing data for calculation
         * @param paramStore MHO_ParameterStore containing parameters for calculation
         * @note This is a static function.
         */
        static void calculate_ion_covariance(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
};

} // namespace hops

#endif /*! end of include guard: MHO_BasicFringeUtilities_HH__ */
