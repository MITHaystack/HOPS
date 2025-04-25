#ifndef MHO_ParameterConfigurator_HH__
#define MHO_ParameterConfigurator_HH__

#include <string>
#include <utility>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file MHO_ParameterConfigurator.hh
 *@class MHO_ParameterConfigurator
 *@date Mon Jun 12 12:51:32 2023 -0400
 *@brief
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_ParameterConfigurator
{

    public:
        MHO_ParameterConfigurator(MHO_ParameterStore* pstore, mho_json control_format): fParameterStore(pstore)
        {
            fFormat = control_format;
        };

        virtual ~MHO_ParameterConfigurator(){};

        //json config for this parameter (parsed from the control file)
        virtual void SetConditions(const mho_json& cond) { fConditions = cond; } //conditional statements

        virtual void SetAttributes(const mho_json& attr) { fAttributes = attr; }; //configuration parameters

        virtual bool Configure();

    protected:
        /*! data */
        enum class ParamType
        {
            config,
            global,
            station,
            baseline,
            fit,
            plot,
            unknown
        };

        enum class ParamValueType
        {
            int_type,         //single integer parameter
            real_type,        //single float parameter
            bool_type,        //single boolean parameter
            string_type,      //single string parameter
            list_int_type,    //list of ints with arbitrary length
            list_real_type,   //list of floats with arbitrary length
            list_string_type, //list of strings with arbitrary length
            compound_type,    //multiple elements of different types
            logical_intersection_list_string_type, //list of strings, if multiple lists are encountered take the logical intersection (e.g. freqs)
            unknown
        };

        typedef ParamType param_t;
        typedef ParamValueType paramv_t;

        ParamType DetermineParamType(const std::string& par_type) const;
        ParamValueType DetermineParamValueType(const std::string& par_value_type) const;

        template< typename XValueType > void SetScalarParameter(std::string path, const XValueType& value);
        template< typename XValueType > void GetScalarParameter(std::string path, XValueType& value);

        template< typename XValueType > void SetVectorParameter(std::string path, const std::vector< XValueType >& values);
        template< typename XValueType > void GetVectorParameter(std::string path, std::vector< XValueType >& values);

        void SetCompoundParameter(std::string path, const mho_json& values);

        std::vector< std::string > LogicalIntersection(std::vector< std::string >& values1, std::vector< std::string>& values2) const;

        MHO_ParameterStore* fParameterStore;
        mho_json fFormat; //format description for each parameter

        //provided for the configuration of the parameter that is to be set
        mho_json fConditions;
        mho_json fAttributes;
};

template< typename XValueType > void MHO_ParameterConfigurator::SetScalarParameter(std::string path, const XValueType& value)
{
    bool ok = fParameterStore->Set(path, value);
    if(!ok)
    {
        msg_warn("initialization", "could not set parameter: " << path << eom);
    }
}

template< typename XValueType > void MHO_ParameterConfigurator::GetScalarParameter(std::string path, XValueType& value)
{
    bool ok = fParameterStore->Get(path, value);
    if(!ok)
    {
        msg_info("initialization", "could not get parameter, using default value for: " << path << eom);
    }
}

template< typename XValueType >
void MHO_ParameterConfigurator::SetVectorParameter(std::string path, const std::vector< XValueType >& values)
{
    bool ok = fParameterStore->Set(path, values);
    if(!ok)
    {
        msg_warn("initialization", "could not set parameter vector: " << path << eom);
    }
}

template< typename XValueType >
void MHO_ParameterConfigurator::GetVectorParameter(std::string path, std::vector< XValueType >& values)
{
    bool ok = fParameterStore->Get(path, values);
    if(!ok)
    {
        msg_info("initialization", "could not get parameter vector, using default value for: " << path << eom);
    }
}

} // namespace hops

#endif /*! end of include guard: MHO_ParameterConfigurator_HH__ */
