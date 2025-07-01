#ifndef MHO_FringePlotInfo_HH__
#define MHO_FringePlotInfo_HH__

//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file MHO_FringePlotInfo.hh
 *@class MHO_FringePlotInfo
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 16:26:33 2023 -0400
 *@brief a collection of helper functions to organize fringe fitting
 */

/**
 * @brief Class MHO_FringePlotInfo
 */
class MHO_FringePlotInfo
{

    public:
        MHO_FringePlotInfo(){};
        virtual ~MHO_FringePlotInfo(){};

    public:
        /**
         * @brief Constructs plot data using provided stores and toolbox, populating vexInfo.
         * 
         * @param conStore MHO_ContainerStore pointer for accessing visibility and weight data
         * @param paramStore MHO_ParameterStore pointer for retrieving configuration parameters
         * @param toolbox MHO_OperatorToolbox pointer for performing computations
         * @param vexInfo Reference to mho_json object for storing vex information
         * @return mho_json containing constructed plot data
         * @note This is a static function.
         */
        static mho_json construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore,
                                            MHO_OperatorToolbox* toolbox, mho_json& vexInfo);
        /**
         * @brief Fills a JSON object with plot data retrieved from an MHO_ParameterStore.
         * 
         * @param paramStore Pointer to MHO_ParameterStore containing plot data
         * @param plot_dict Reference to mho_json object that will store the retrieved plot data
         * @note This is a static function.
         */
        static void fill_plot_data(MHO_ParameterStore* paramStore, mho_json& plot_dict);
};

} // namespace hops

#endif /*! end of include guard: MHO_FringePlotInfo_HH__ */
