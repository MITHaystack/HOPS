#ifndef MHO_FringeControlInitialization_HH__
#define MHO_FringeControlInitialization_HH__

/*!
*@file MHO_FringeControlInitialization.hh
*@class MHO_FringeControlInitialization
*@author J. Barrettj - barrettj@mit.edu
*@date Tue Sep 19 04:11:24 PM EDT 2023
*@brief collection of helper functions for fringe fitter start-up
*/

//global messaging util
#include "MHO_Message.hh"

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

class MHO_FringeControlInitialization
{

    public:
        MHO_FringeControlInitialization(){};
        virtual ~MHO_FringeControlInitialization(){};

    public:

        //control format and control_statements objects should reference the MHO_FringeData object
        static void process_control_file(MHO_ParameterStore* paramStore, mho_json& control_format, mho_json& control_statements);
        static void add_default_operator_format_def(mho_json& format);
        static void add_default_operators(mho_json& statements);
        static void add_polprod_sum_operator(mho_json& statements);

        static bool is_linear_polprod(std::string pp);
        static void add_dpar_sign_correction_operator(mho_json& statements);

        //checks if the special ionospheric fringe fitter is needed
        static bool need_ion_search(mho_json* control);


};

}//end namespace

#endif /*! end of include guard: MHO_FringeControlInitialization_HH__ */
