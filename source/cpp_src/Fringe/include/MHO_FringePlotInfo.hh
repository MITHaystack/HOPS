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

class MHO_FringePlotInfo
{

    public:
        MHO_FringePlotInfo(){};
        virtual ~MHO_FringePlotInfo(){};

    public:
        static mho_json construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore,
                                            MHO_OperatorToolbox* toolbox, mho_json& vexInfo);
        static void fill_plot_data(MHO_ParameterStore* paramStore, mho_json& plot_dict);
};

} // namespace hops

#endif /*! end of include guard: MHO_FringePlotInfo_HH__ */
