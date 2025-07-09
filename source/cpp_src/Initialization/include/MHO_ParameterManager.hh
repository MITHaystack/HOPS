#ifndef MHO_ParameterManager_HH__
#define MHO_ParameterManager_HH__

#include "MHO_ParameterConfigurator.hh"

namespace hops
{

/*!
 *@file MHO_ParameterManager.hh
 *@class MHO_ParameterManager
 *@date Mon Jun 12 14:55:36 2023 -0400
 *@brief Manager class for the MHO_ParameterConfigurator
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_ParameterManager
{
    public:
        MHO_ParameterManager(MHO_ParameterStore* pstore, const mho_json& control_format)
            : fDefaultParameterConfig(pstore, control_format)
        {
            fFormat = control_format;
        };

        virtual ~MHO_ParameterManager(){};

        /**
         * @brief Setter for control statements
         * 
         * @param statements Control file statements of type mho_json
         */
        void SetControlStatements(mho_json* statements) { fControl = statements; };

        /**
         * @brief Configures all control file set parameters by iterating through control statements and processing parameter statements.
         */
        void ConfigureAll();

    private:
        mho_json* fControl;

        mho_json fFormat;
        MHO_ParameterConfigurator fDefaultParameterConfig;
};

} // namespace hops

#endif /*! end of include guard: MHO_ParameterManager_HH__ */
