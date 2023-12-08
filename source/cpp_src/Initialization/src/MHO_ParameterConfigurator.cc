#include "MHO_ParameterConfigurator.hh"

namespace hops
{

bool
MHO_ParameterConfigurator::Configure()
{
    //make sure everything gets put under /control section in parameters
    return Configure_V2();

    //find the format for this attribute
    std::string name = fAttributes["name"].get<std::string>();
    std::string statement_type = fAttributes["statement_type"].get<std::string>();

    if(statement_type == "parameter")
    {
        std::string parameter_type = "generic";
        if(fFormat[name].contains("parameter_type")){ parameter_type = fFormat[name]["parameter_type"].get<std::string>(); }
        param_t param_type = DetermineParamType(parameter_type);

        //look up the parameter type in the format
        std::string value_type = fFormat[name]["type"].get<std::string>();
        //TODO -- move the control parameters into a sorted structure
        // std::string path = "/control/" + parameter_type + "/" + name;

        std::string path = name;
        switch( DetermineParamValueType(value_type) )
        {
            case ParamValueType::int_type:
            {
                //braces needed to avoid 'crossing initialization' error
                int value = fAttributes["value"].get<int>();
                SetScalarParameter(path, value);
            }
            break;
            case ParamValueType::real_type:
            {
                double value = fAttributes["value"].get<double>();
                SetScalarParameter(path, value);
            }
            break;
            case ParamValueType::bool_type:
            {
                bool value = fAttributes["value"].get<bool>();
                SetScalarParameter(path, value);
            }
            break;
            case ParamValueType::string_type:
            {
                std::string value = fAttributes["value"].get<std::string>();
                SetScalarParameter(path, value);
            }
            break;
            case ParamValueType::list_int_type:
            {
                std::vector< int > values = fAttributes["value"].get< std::vector< int > >();
                SetVectorParameter(path, values);
            }
            break;
            case ParamValueType::list_real_type:
            {
                std::vector< double > values = fAttributes["value"].get< std::vector< double > >();
                SetVectorParameter(path, values);
            }
            break;
            case ParamValueType::list_string_type:
            {
                std::vector< std::string > values = fAttributes["value"].get< std::vector< std::string > >();
                SetVectorParameter(path, values);
            }
            break;
            default:
                msg_debug("initialization", "could not determine the parameter: " <<name <<"'s value type: "<< value_type << eom);
                return false;
        };
    }

    return true; //ok, we were not passed a paramter-attribute
}

bool MHO_ParameterConfigurator::Configure_V2()
{
    //find the format for this attribute
    std::string name = fAttributes["name"].get<std::string>();
    std::string statement_type = fAttributes["statement_type"].get<std::string>();

    if(statement_type == "parameter")
    {
        std::string parameter_type = "generic";
        if(fFormat[name].contains("parameter_type")){ parameter_type = fFormat[name]["parameter_type"].get<std::string>(); }
        param_t param_type = DetermineParamType(parameter_type);

        //look up the parameter type in the format
        std::string value_type = fFormat[name]["type"].get<std::string>();
        //TODO -- move the control parameters into a sorted structure
        std::string path = "/control/" + parameter_type + "/" + name;

        // std::string path = name;
        switch( DetermineParamValueType(value_type) )
        {
            case ParamValueType::int_type:
            {
                //braces needed to avoid 'crossing initialization' error
                int value = fAttributes["value"].get<int>();
                SetScalarParameter(path, value);
            }
            break;
            case ParamValueType::real_type:
            {
                double value = fAttributes["value"].get<double>();
                SetScalarParameter(path, value);
            }
            break;
            case ParamValueType::bool_type:
            {
                bool value = fAttributes["value"].get<bool>();
                SetScalarParameter(path, value);
            }
            break;
            case ParamValueType::string_type:
            {
                std::string value = fAttributes["value"].get<std::string>();
                SetScalarParameter(path, value);
            }
            break;
            case ParamValueType::list_int_type:
            {
                std::vector< int > values = fAttributes["value"].get< std::vector< int > >();
                SetVectorParameter(path, values);
            }
            break;
            case ParamValueType::list_real_type:
            {
                std::vector< double > values = fAttributes["value"].get< std::vector< double > >();
                SetVectorParameter(path, values);
            }
            break;
            case ParamValueType::list_string_type:
            {
                std::vector< std::string > values = fAttributes["value"].get< std::vector< std::string > >();
                SetVectorParameter(path, values);
            }
            break;
            default:
                msg_debug("initialization", "could not determine the parameter: " <<name <<"'s value type: "<< value_type << eom);
                return false;
        };
    }
    return true; //ok, we were not passed a paramter-attribute
}



MHO_ParameterConfigurator::ParamValueType
MHO_ParameterConfigurator::DetermineParamValueType(const std::string& par_value_type) const
{
    if(par_value_type == "int"){return ParamValueType::int_type;}
    if(par_value_type == "real"){return ParamValueType::real_type;}
    if(par_value_type == "bool"){return ParamValueType::bool_type;}
    if(par_value_type == "string"){return ParamValueType::string_type;}
    if(par_value_type == "list_int"){return ParamValueType::list_int_type;}
    if(par_value_type == "list_real"){return ParamValueType::list_real_type;}
    if(par_value_type == "list_string"){return ParamValueType::list_string_type;}
    return ParamValueType::unknown;
}

MHO_ParameterConfigurator::ParamType
MHO_ParameterConfigurator::DetermineParamType(const std::string& par_type) const
{
    if(par_type == "config"){return ParamType::config;}
    if(par_type == "global"){return ParamType::global;}
    if(par_type == "station"){return ParamType::station;}
    if(par_type == "baseline"){return ParamType::baseline;}
    if(par_type == "fit"){return ParamType::fit;}
    if(par_type == "plot"){return ParamType::plot;}
    if(par_type == "generic"){return ParamType::generic;}
    return ParamType::unknown;
}


}
