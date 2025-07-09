#ifndef MHO_InitialFringeInfo_HH__
#define MHO_InitialFringeInfo_HH__

//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file MHO_InitialFringeInfo.hh
 *@class MHO_InitialFringeInfo
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 16:12:23 2023 -0400
 *@brief collection of helper functions to populate the parameter store
 * with a priori information known before fringe fitting
 */

/**
 * @brief Class MHO_InitialFringeInfo
 */
class MHO_InitialFringeInfo
{

    public:
        MHO_InitialFringeInfo(){};
        virtual ~MHO_InitialFringeInfo(){};

    public:
        /**
         * @brief Sets default MHO_ParameterStore values for minimal configuration.
         * 
         * @param paramStore Pointer to an MHO_ParameterStore object where defaults are set.
         * @note This is a static function.
         */
        static void set_default_parameters_minimal(MHO_ParameterStore* paramStore);
        
        /**
         * @brief Sets default reference frequency (determined from visibility data) if not already set in parameter store.
         * 
         * @param conStore Pointer to MHO_ContainerStore for accessing visibility data.
         * @param paramStore Pointer to MHO_ParameterStore for managing parameters.
         * @note This is a static function.
         */
        static void configure_reference_frequency(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        
        /**
         * @brief Calculates frequency space and stores fringe parameters in paramStore.
         * 
         * @param conStore Input container store for visibility data
         * @param paramStore Parameter store to hold calculated fringe parameters
         * @note This is a static function.
         */
        static void calculate_freq_space(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        
        /**
         * @brief Calculates clock model for reference and remote stations using parameters from MHO_ParameterStore.
         * 
         * @param paramStore Pointer to MHO_ParameterStore containing relevant clock parameters
         * @note This is a static function.
         */
        static void calculate_clock_model(MHO_ParameterStore* paramStore);
        
        /**
         * @brief Precalculates and stores relevant quantities from visibility data into parameter store.
         * 
         * @param conStore Input container store for retrieving visibility data
         * @param paramStore Output parameter store for storing precalculated quantities
         * @note This is a static function.
         */
        static void precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        
        /**
         * @brief Calculates and stores total summed weights from container store.
         * 
         * @param conStore Input MHO_ContainerStore containing weight data
         * @param paramStore Output MHO_ParameterStore to store result
         * @note This is a static function.
         */
        static void compute_total_summed_weights(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
        
        /**
         * @brief Counts and stores the number of active channels in MHO_ContainerStore based on weight.
         * 
         * @param conStore Input container store for weights data
         * @param paramStore Output parameter store to set n_active_channels
         * @note This is a static function.
         */
        static void determine_n_active_channels(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
};

} // namespace hops

#endif /*! end of include guard: MHO_InitialFringeInfo_HH__ */
