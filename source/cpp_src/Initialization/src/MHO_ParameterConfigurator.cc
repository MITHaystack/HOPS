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
        std::string parameter_type = "config"; //default type is config
        if(fFormat[name].contains("parameter_type")){ parameter_type = fFormat[name]["parameter_type"].get<std::string>(); }
        param_t param_type = DetermineParamType(parameter_type);

        //look up the parameter type in the format
        std::string value_type = fFormat[name]["type"].get<std::string>();
        //TODO -- move the control parameters into a sorted structure
        std::string default_path = "/control/" + parameter_type + "/" + name;
        std::vector< std::string > explicit_paths;

        //certain parameter types must modify their path based on their conditional
        //statement, specifically 'station' parameters need to include the station mk4 ID
        //so we can distinguish them later (reference vs remote station)
        // e.g. we want to store them as '/control/station/E/ionosphere' or '/control/station/G/pc_mode'
        // rather than just /control/station/<parameter_name>
        if(param_type == ParamType::station)
        {
            //for now only 'station' parameters get this treatment, because of
            //'or' and 'and' statements we may have multiple paths that are valid

            //note that explict paths override the values at the base level
            //for example "/control/station/pc_mode/multitone" will be overriden
            //by "/control/station/G/pc_mode/manual" if the latter is present

            for(auto tokit = fConditions.begin(); tokit != fConditions.end(); tokit++)
            {
                if(*tokit == "station") //next token must be station MK4 ID
                {
                    std::string mk4id = *(++tokit);
                    std::string station_path = "/control/" + parameter_type + "/" + mk4id + "/" + name;
                    explicit_paths.push_back(station_path);
                }
            }
        }

        //just a single path, so add the default path here
        if(explicit_paths.size() == 0)
        {
            explicit_paths.push_back(default_path);
        }

        // std::string path = name;
        switch( DetermineParamValueType(value_type) )
        {
            case ParamValueType::int_type:
            {
                //braces needed to avoid 'crossing initialization' error
                int value = fAttributes["value"].get<int>();
                for(std::size_t nst = 0; nst<explicit_paths.size(); nst++)
                {
                    SetScalarParameter(explicit_paths[nst], value);
                }
            }
            break;
            case ParamValueType::real_type:
            {
                double value = fAttributes["value"].get<double>();
                for(std::size_t nst = 0; nst<explicit_paths.size(); nst++)
                {
                    SetScalarParameter(explicit_paths[nst], value);
                }
            }
            break;
            case ParamValueType::bool_type:
            {
                bool value = fAttributes["value"].get<bool>();
                for(std::size_t nst = 0; nst<explicit_paths.size(); nst++)
                {
                    SetScalarParameter(explicit_paths[nst], value);
                }
            }
            break;
            case ParamValueType::string_type:
            {
                std::string value = fAttributes["value"].get<std::string>();
                for(std::size_t nst = 0; nst<explicit_paths.size(); nst++)
                {
                    SetScalarParameter(explicit_paths[nst], value);
                }
            }
            break;
            case ParamValueType::list_int_type:
            {
                std::vector< int > values = fAttributes["value"].get< std::vector< int > >();
                for(std::size_t nst = 0; nst<explicit_paths.size(); nst++)
                {
                    SetVectorParameter(explicit_paths[nst], values);
                }
            }
            break;
            case ParamValueType::list_real_type:
            {
                std::vector< double > values = fAttributes["value"].get< std::vector< double > >();
                for(std::size_t nst = 0; nst<explicit_paths.size(); nst++)
                {
                    SetVectorParameter(explicit_paths[nst], values);
                }
            }
            break;
            case ParamValueType::list_string_type:
            {
                std::vector< std::string > values = fAttributes["value"].get< std::vector< std::string > >();
                for(std::size_t nst = 0; nst<explicit_paths.size(); nst++)
                {
                    SetVectorParameter(explicit_paths[nst], values);
                }
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
    if(par_value_type == "list_string" || par_value_type == "fixed_length_list_string" ){return ParamValueType::list_string_type;}
    return ParamValueType::unknown;
}

MHO_ParameterConfigurator::ParamType
MHO_ParameterConfigurator::DetermineParamType(const std::string& par_type) const
{
    if(par_type == "config"){return ParamType::config;} //default type
    if(par_type == "global"){return ParamType::global;}
    if(par_type == "station"){return ParamType::station;}
    if(par_type == "baseline"){return ParamType::baseline;}
    if(par_type == "fit"){return ParamType::fit;}
    if(par_type == "plot"){return ParamType::plot;}
    return ParamType::unknown;
}


}
