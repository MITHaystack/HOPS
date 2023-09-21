#ifndef MHO_FringePlotInfo_HH__
#define MHO_FringePlotInfo_HH__

/*
*File: MHO_FringePlotInfo.hh
*Class: MHO_FringePlotInfo
*Author:
*Email:
*Date: Tue Sep 19 04:11:24 PM EDT 2023
*Description: a collection of helper functions to organize fringe fitting
*/

//global messaging util
#include "MHO_Message.hh"

//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops 
{

class MHO_FringePlotInfo
{
        
    public:
        MHO_FringePlotInfo(){};
        virtual ~MHO_FringePlotInfo(){};
    
    public:

        static mho_json construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, mho_json& vexInfo);
        static void fill_plot_data(MHO_ParameterStore* paramStore, mho_json& plot_dict);
};

}//end namespace

#endif /* end of include guard: MHO_FringePlotInfo_HH__ */
