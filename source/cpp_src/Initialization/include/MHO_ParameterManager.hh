#ifndef MHO_ParameterManager_HH__
#define MHO_ParameterManager_HH__



#include "MHO_ParameterConfigurator.hh"

namespace hops
{

/*!
*@file MHO_ParameterManager.hh
*@class MHO_ParameterManager
*@date
*@brief
*@author J. Barrett - barrettj@mit.edu
*/

class MHO_ParameterManager
{
    public:

        MHO_ParameterManager(MHO_ParameterStore* pstore, const mho_json& control_format):
            fDefaultParameterConfig(pstore, control_format)
        {
            fFormat = control_format;
        };

        virtual ~MHO_ParameterManager(){};

        void SetControlStatements(mho_json* statements){fControl = statements;};

        void ConfigureAll();

    private:


        mho_json* fControl;

        mho_json fFormat;
        MHO_ParameterConfigurator fDefaultParameterConfig;
};

}


#endif /*! end of include guard: MHO_ParameterManager_HH__ */
