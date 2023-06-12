#include "MHO_ParameterConfigurator.hh"

namespace hops 
{

bool 
MHO_ParameterConfigurator::Configure()
{
    //find the format for this attribute
    std::string name = fAttributes["name"].get<std::string>();
    std::string statement_type = fAttributes["statement_type"].get<std::string>();
    if(statement_type == "parameter")
    {
        //look up the parameter type in the format
        std::string frmt_type = fFormat[name]["type"].get<std::string>();

        switch( DetermineParamType(frmt_type) ) 
        {
            case ParamType::int_type:
            {       //braces needed to avoid 'crossing initialization' error
                    int value = fAttributes["value"].get<int>();
                    SetScalarParameter(name, value);
            }
            break;
            case ParamType::real_type:
            {
                double value = fAttributes["value"].get<double>();
                SetScalarParameter(name, value);
            }
            break;
            case ParamType::string_type:
            {
                std::string value = fAttributes["value"].get<std::string>();
                SetScalarParameter(name, value);
            }
            break;
            case ParamType::list_int_type:
            {
                std::vector< int > values = fAttributes["value"].get< std::vector< int > >();
                SetVectorParameter(name, values);
            }
            break;
            case ParamType::list_real_type:
            {
                std::vector< double > values = fAttributes["value"].get< std::vector< double > >();
                SetVectorParameter(name, values);
            }
            break;
            case ParamType::list_string_type:
            {
                std::vector< std::string > values = fAttributes["value"].get< std::vector< std::string > >();
                SetVectorParameter(name, values);
            }
            break;
            default:
                msg_debug("initialization", "could not determine the parameter type: "<< frmt_type << eom);
                return false;
        };
    }

    return true; //ok, we were not passed a paramter-attribute
}


MHO_ParameterConfigurator::ParamType 
MHO_ParameterConfigurator::DetermineParamType(const std::string& par_type) const
{
    if(par_type == "int")
    {
        return ParamType::int_type;
    }

    if(par_type == "real")
    {
        return ParamType::real_type;
    }

    if(par_type == "string")
    {
        return ParamType::string_type;
    }

    if(par_type == "list_int")
    {
        return ParamType::list_int_type;
    }

    if(par_type == "list_real")
    {
        return ParamType::list_real_type;
    }

    if(par_type == "list_string")
    {
        return ParamType::list_string_type;
    }

    return ParamType::unknown;
}



}