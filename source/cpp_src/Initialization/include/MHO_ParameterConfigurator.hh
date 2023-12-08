#ifndef MHO_ParameterConfigurator_HH__
#define MHO_ParameterConfigurator_HH__

#include <string>
#include <utility>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_JSONHeaderWrapper.hh"


namespace hops {
    
class MHO_ParameterConfigurator
{

    public:
        MHO_ParameterConfigurator(MHO_ParameterStore* pstore, mho_json control_format):
            fParameterStore(pstore)
        {
            fFormat = control_format;
        };

        virtual ~MHO_ParameterConfigurator(){};

        //json config for this parameter (parsed from the control file)
        virtual void SetConditions(const mho_json& cond){fConditions = cond;} //conditional statements
        virtual void SetAttributes(const mho_json& attr){fAttributes = attr;}; //configuration parameters
        virtual bool Configure();

    protected:
        
        void Configure_V2();
        
        /* data */
        enum class ParamType
        {
            config,
            global,
            station,
            baseline,
            fit,
            plot,
            generic,
            unknown
        };
        
        enum class ParamValueType
        {
            int_type,
            real_type,
            bool_type,
            string_type,
            list_int_type,
            list_real_type,
            list_string_type,
            unknown
        };
        
        
        typedef ParamType param_t;
        typedef ParamValueType paramv_t;

        ParamType DetermineParamType(const std::string& par_type) const;
        ParamValueType DetermineParamValueType(const std::string& par_value_type) const;

        template< typename XValueType >
        void SetScalarParameter(std::string path, const XValueType& value);

        template< typename XValueType >
        void SetVectorParameter(std::string path, const std::vector<XValueType>& values);

        MHO_ParameterStore* fParameterStore;
        mho_json fFormat; //format description for each parameter

        //provided for the configuration of the parameter that is to be setting
        mho_json fConditions;
        mho_json fAttributes;

};


template< typename XValueType >
void 
MHO_ParameterConfigurator::SetScalarParameter(std::string path, const XValueType& value)
{
    bool ok = fParameterStore->Set(path, value);
    if(!ok){msg_warn("initialization", "could not set parameter: " << path << eom);}
}

template< typename XValueType >
void MHO_ParameterConfigurator::SetVectorParameter(std::string path, const std::vector<XValueType>& values)
{
    bool ok = fParameterStore->Set(path, values);
    if(!ok){msg_warn("initialization", "could not set parameter vector: " << path << eom);}
}



} /* hops */



#endif /* end of include guard: MHO_ParameterConfigurator_HH__ */
