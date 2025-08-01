#include "MHO_VexElementLineGenerator.hh"

namespace hops
{

MHO_VexElementLineGenerator::MHO_VexElementLineGenerator()
{
    fSpace = " ";
};

MHO_VexElementLineGenerator::~MHO_VexElementLineGenerator(){};

std::string MHO_VexElementLineGenerator::ConstructElementLine(std::string element_name, mho_json& element, mho_json& format)
{

    std::string element_typename = format["type"].get< std::string >();
    vex_element_type etype = MHO_VexDefinitions::DetermineType(element_typename);
    std::string line = element_name + fSpace + MHO_VexDefinitions::AssignmentOp() + fSpace;

    std::string ret_val;

    switch(etype)
    {
        case vex_int_type:
            ret_val = GenerateInt(element_name, element);
            break;
        case vex_list_int_type:
            ret_val = GenerateListInt(element_name, element);
            break;
        case vex_real_type:
            ret_val = GenerateReal(element_name, element);
            break;
        case vex_list_real_type:
            ret_val = GenerateListReal(element_name, element);
            break;
        case vex_keyword_type:
            ret_val = GenerateKeyword(element_name, element);
            break;
        case vex_string_type:
            ret_val = GenerateString(element_name, element);
            break;
        case vex_list_string_type:
            ret_val = GenerateListString(element_name, element);
            break;
        case vex_epoch_type:
            ret_val = GenerateEpoch(element_name, element);
            break;
        case vex_radec_type:
            ret_val = GenerateRaDec(element_name, element);
            break;
        case vex_link_type:
            ret_val = GenerateLink(element_name, element);
            break;
        case vex_compound_type: //all compound types treated the same way
            ret_val = GenerateCompound(element_name, element, format);
            break;
        case vex_list_compound_type:
            ret_val = GenerateCompound(element_name, element, format);
            break;
        default:
            break;
    }
    line += ret_val + MHO_VexDefinitions::StatementLineEnd();
    return line;
}

std::string MHO_VexElementLineGenerator::GenerateInt(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << fSpace << obj.get< int >() << fSpace;
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateListInt(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    for(std::size_t i = 0; i < obj.size(); i++)
    {
        val << fSpace << obj[i].get< int >() << fSpace;
        if(i != (obj.size() - 1))
        {
            val << MHO_VexDefinitions::ElementDelim();
        }
    }
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateReal(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << std::setprecision(std::numeric_limits< double >::digits10 + 1);
    if(obj.contains("units") && obj.contains("value"))
    {
        val << fSpace << obj["value"].get< double >() << fSpace << obj["units"].get< std::string >() << fSpace;
    }
    else if(obj.contains("value"))
    {
        val << fSpace << obj["value"].get< double >() << fSpace;
    }
    else
    {
        msg_error("vex", "could not form valid vex string from json real element: " << element_name << eom);
    }
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateListReal(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << std::setprecision(std::numeric_limits< double >::digits10 + 1);
    if(obj.contains("units") && obj.contains("values"))
    {
        for(std::size_t i = 0; i < obj["values"].size(); i++)
        {
            if(i == 0)
            {
                val << fSpace << obj["values"][i].get< double >() << fSpace << obj["units"].get< std::string >() << fSpace;
            }
            else
            {
                val << fSpace << obj["values"][i].get< double >() << fSpace;
            }
            if(i != (obj["values"].size() - 1))
            {
                val << MHO_VexDefinitions::ElementDelim();
            }
        }
    }
    else if(obj.contains("values"))
    {
        for(std::size_t i = 0; i < obj["values"].size(); i++)
        {
            val << fSpace << obj["values"][i].get< double >() << fSpace;
            if(i != (obj["values"].size() - 1))
            {
                val << MHO_VexDefinitions::ElementDelim();
            }
        }
    }
    else
    {
        msg_error("vex", "could not form valid vex string from json list_real element: " << element_name << eom);
    }
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateKeyword(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << fSpace << obj.get< std::string >() << fSpace;
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateString(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << fSpace << obj.get< std::string >() << fSpace;
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateListString(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    for(std::size_t i = 0; i < obj.size(); i++)
    {
        val << fSpace << obj[i].get< std::string >() << fSpace;
        if(i != (obj.size() - 1))
        {
            val << MHO_VexDefinitions::ElementDelim();
        }
    }
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateEpoch(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << fSpace << obj.get< std::string >() << fSpace;
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateRaDec(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << fSpace << obj.get< std::string >() << fSpace;
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateLink(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << fSpace << obj.get< std::string >() << fSpace;
    return val.str();
}

std::string MHO_VexElementLineGenerator::GenerateCompound(std::string element_name, mho_json& element, mho_json& format)
{
    std::vector< std::string > components;
    //loop over items in format, and extract from element
    std::string bang = MHO_VexDefinitions::OptionalFlag();
    std::string nothing = "";

    for(std::size_t i = 0; i < format["fields"].size(); i++)
    {
        std::string raw_field_name = format["fields"][i].get< std::string >();
        std::string field_name = string_pattern_replace(raw_field_name, bang, nothing);
        //std::regex_replace(raw_field_name, std::regex(bang), nothing);

        if(element.contains(field_name))
        {
            std::string par_type = format["parameters"][field_name]["type"].get< std::string >();
            vex_element_type etype = MHO_VexDefinitions::DetermineType(par_type);
            std::string ret_val;

            switch(etype)
            {
                case vex_int_type:
                    ret_val = GenerateInt(field_name, element[field_name]);
                    break;
                case vex_list_int_type:
                    ret_val = GenerateListInt(field_name, element[field_name]);
                    break;
                case vex_real_type:
                    ret_val = GenerateReal(field_name, element[field_name]);
                    break;
                case vex_list_real_type:
                    ret_val = GenerateListReal(field_name, element[field_name]);
                    break;
                case vex_keyword_type:
                    ret_val = GenerateKeyword(field_name, element[field_name]);
                    break;
                case vex_string_type:
                    ret_val = GenerateString(field_name, element[field_name]);
                    break;
                case vex_list_string_type:
                    ret_val = GenerateListString(field_name, element[field_name]);
                    break;
                case vex_epoch_type:
                    ret_val = GenerateEpoch(field_name, element[field_name]);
                    break;
                case vex_radec_type:
                    ret_val = GenerateRaDec(field_name, element[field_name]);
                    break;
                case vex_link_type:
                    ret_val = GenerateLink(field_name, element[field_name]);
                    break;
                default:
                    break;
            }
            components.push_back(ret_val);
        }
        else if(MHO_VexDefinitions::IsOptionalField(raw_field_name) &&
                !IsTrailingOptionalField(raw_field_name, format["fields"]))
        {
            //std::cout<<"raw_field_name = "<<raw_field_name<<std::endl;
            //add and empty space for optional fields which are not trailing elements
            std::string ret_val = fSpace;
            components.push_back(ret_val);
        }
        else if(!MHO_VexDefinitions::IsOptionalField(raw_field_name))
        {
            //std::cout<<"encountered empty not-optional field: raw_field_name = "<<raw_field_name<<std::endl;
            //add and empty space for non-optional files which are missing
            std::string ret_val = fSpace;
            components.push_back(ret_val);
        }
    }

    std::string line;
    for(std::size_t j = 0; j < components.size(); j++)
    {
        line += components[j];
        if(j < components.size() - 1)
        {
            line += MHO_VexDefinitions::ElementDelim();
        }
    }
    return line;
}

bool MHO_VexElementLineGenerator::IsTrailingOptionalField(std::string field_name, mho_json& fields)
{
    if(MHO_VexDefinitions::IsOptionalField(field_name))
    {
        std::size_t start_idx = 0;
        for(std::size_t i = 0; i < fields.size(); i++)
        {
            if(fields[i].get< std::string >() == field_name)
            {
                start_idx = i;
                break;
            }
        }

        bool ret_val = true;
        for(std::size_t i = start_idx; i < fields.size(); i++)
        {
            std::string tmp_name = fields[i].get< std::string >();
            if(!MHO_VexDefinitions::IsOptionalField(tmp_name))
            {
                ret_val = false;
            }
        }
        return ret_val;
    }
    return false;
}

} // namespace hops
