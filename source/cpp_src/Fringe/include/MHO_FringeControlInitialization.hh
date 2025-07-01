#ifndef MHO_FringeControlInitialization_HH__
#define MHO_FringeControlInitialization_HH__

//global messaging util
#include "MHO_Message.hh"

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
 *@file MHO_FringeControlInitialization.hh
 *@class MHO_FringeControlInitialization
 *@author J. Barrettj - barrettj@mit.edu
 *@date Mon Feb 5 13:59:09 2024 -0500
 *@brief collection of helper functions for fringe fitter start-up
 */

/**
 * @brief Class MHO_FringeControlInitialization
 */
class MHO_FringeControlInitialization
{

    public:
        MHO_FringeControlInitialization(){};
        virtual ~MHO_FringeControlInitialization(){};

    public:
        //control format and control_statements objects should reference the MHO_FringeData object
        /**
         * @brief Process control file and populate applicable statements in parameter store.
         * 
         * @param paramStore Pointer to MHO_ParameterStore object for storing parameters.
         * @param control_format Reference to mho_json object for control format definition.
         * @param control_statements Reference to mho_json object for applicable control statements.
         * @note This is a static function.
         */
        static void process_control_file(MHO_ParameterStore* paramStore, mho_json& control_format,
                                         mho_json& control_statements);
        /**
         * @brief Adds default operator format definitions to an mho_json object for fringe control initialization.
         * 
         * @param format Reference to an mho_json object where defaults will be added.
         * @note This is a static function.
         */
        static void add_default_operator_format_def(mho_json& format);
        /**
         * @brief Adds default operators to control statements for MHO fringe control initialization.
         * 
         * @param statements Reference to mho_json object where default operators will be added.
         * @note This is a static function.
         */
        static void add_default_operators(mho_json& statements);
        /**
         * @brief Adds a default polarization product sum operator to the given statements.
         * 
         * @param statements Reference to an mho_json object containing control statements.
         * @note This is a static function.
         */
        static void add_polprod_sum_operator(mho_json& statements);

        /**
         * @brief Checks if input string represents a linear polarization product.
         * 
         * @param pp Input string representing polarization product.
         * @return True if pp is one of predefined linear products, false otherwise.
         * @note This is a static function.
         */
        static bool is_linear_polprod(std::string pp);
        /**
         * @brief Adds a default correction operator for dpar to the given statements.
         * 
         * @param statements Reference to mho_json object containing control statements
         * @note This is a static function.
         */
        static void add_dpar_sign_correction_operator(mho_json& statements);

        /**
         * @brief Checks if given polarization product is circular (RR, LL, RL, LR).
         * 
         * @param pp Input polarization product string
         * @return True if circular, false otherwise
         * @note This is a static function.
         */
        static bool is_circular_polprod(std::string pp);
        /**
         * @brief Adds a predefined correction operator for circular field rotation to the given statements.
         * 
         * @param statements Reference to a list of JSON statements where the correction operator will be added.
         * @note This is a static function.
         */
        static void add_circ_field_rotation_operator(mho_json& statements);

        //checks if the special ionospheric fringe fitter is needed
        /**
         * @brief Checks if special ionospheric fringe fitter is needed by searching control statements.
         * 
         * @param control Input mho_json object containing control statements
         * @return Boolean indicating whether ion search is needed
         * @note This is a static function.
         */
        static bool need_ion_search(mho_json* control);
};

} // namespace hops

#endif /*! end of include guard: MHO_FringeControlInitialization_HH__ */
