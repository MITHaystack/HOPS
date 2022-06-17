#include "MHO_VexElementLineGenerator.hh"
#include <regex>

namespace hops 
{

MHO_VexElementLineGenerator::MHO_VexElementLineGenerator(){};
MHO_VexElementLineGenerator::~MHO_VexElementLineGenerator(){};

std::string 
MHO_VexElementLineGenerator::ConstructElementLine(std::string element_name, mho_json& element, mho_json& format)
{

    std::string element_typename = format["type"].get<std::string>();
    vex_element_type etype = DetermineType(element_typename);
    std::string line = element_name + " = ";

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
            ret_val = GenerateRaDec(element_name,element);
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
    line += ret_val + ";\n";
    return line;
}



std::string 
MHO_VexElementLineGenerator::GenerateInt(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << " " << obj.get<int>() <<" ";
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateListInt(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    for(std::size_t i=0; i<obj.size(); i++)
    {
        val << " " << obj[i].get<double>() << " ";
        if( i != (obj.size()-1) ){ val << ":"; }
    }
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateReal(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    if(obj.contains("units") && obj.contains("value"))
    {
        val << " " << obj["value"].get<double>() << " " << obj["units"].get<std::string>() << " ";// << ";\n";
    }
    else if(obj.contains("value"))
    {
        val << " " << obj["value"].get<double>() << " ";// << ";\n";
    }
    else
    {
        msg_error("vex", "could not form valid vex string from json real element: "<< element_name << eom)
    }
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateListReal(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    if(obj.contains("units") && obj.contains("values"))
    {
        for(std::size_t i=0; i<obj["values"].size(); i++)
        {
            if(i == 0)
            {
                val << " " << obj["values"][i].get<double>() << " " << obj["units"].get<std::string>() << " ";
            }
            else 
            {
                val << " " << obj["values"][i].get<double>() << " ";
            }
            if( i != (obj["values"].size()-1) ){ val << ":"; }
        }
    }
    else if(obj.contains("values"))
    {
        for(std::size_t i=0; i<obj.size(); i++)
        {
            val << " " << obj["values"][i].get<double>() << " ";
            if( i != (obj["values"].size()-1) ){ val << ":"; }
        }
    }
    else
    {
        msg_error("vex", "could not form valid vex string from json list_real element: "<< element_name << eom)
    }
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateKeyword(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << " " << obj.get<std::string>() << " ";
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateString(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << " " << obj.get<std::string>() << " ";
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateListString(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    for(std::size_t i=0; i<obj.size(); i++)
    {
        val << " " << obj[i].get<std::string>() << " ";
        if( i != (obj.size()-1) ){ val << ":"; }
    }
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateEpoch(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << " " << obj.get<std::string>() << " ";
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateRaDec(std::string element_name, mho_json& obj)
{
    std::string val;
    return val;
}

std::string 
MHO_VexElementLineGenerator::GenerateLink(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << " " << obj.get<std::string>() << " ";
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateCompound(std::string element_name, mho_json& element, mho_json& format)
{
    //std::cout<<"working on compound element name:" <<element_name<<std::endl;
    //std::cout<<"elem = "<<element<<std::endl;
    //std::cout<<"fields = "<<format["fields"]<<std::endl;
    std::vector< std::string > components;
    //loop over items in format, and extract from element
    std::string hash = "#";
    std::string nothing = "";

    for(std::size_t i=0; i<format["fields"].size(); i++)
    {
        std::string raw_field_name = format["fields"][i].get<std::string>();
        //remove # prefix indicating optional elements 
        std::string field_name = std::regex_replace(raw_field_name,std::regex(hash),nothing);

        if(element.contains(field_name))
        {
            std::string par_type = format["parameters"][field_name]["type"].get<std::string>();
            //std::cout<<"with field/par = "<<field_name<<" "<<par_type<<std::endl;

            vex_element_type etype = DetermineType(par_type);
            std::string ret_val;
            switch(etype)
            {
                case vex_int_type:
                    ret_val = GenerateInt(field_name, element[field_name]);
                break;
                case vex_list_int_type:
                    ret_val = GenerateListInt(field_name,  element[field_name]);
                break;
                case vex_real_type:
                    ret_val = GenerateReal(field_name, element[field_name]);
                break;
                case vex_list_real_type:
                    ret_val = GenerateListReal(field_name,  element[field_name]);
                break;
                case vex_keyword_type:
                    ret_val = GenerateKeyword(field_name,  element[field_name]);
                break;
                case vex_string_type:
                    ret_val = GenerateString(field_name,  element[field_name]);
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
            //std::cout<<"appending "<<ret_val<<std::endl;
            components.push_back(ret_val);
        }
        else if( IsOptionalField(raw_field_name) && !IsTrailingOptionalField(raw_field_name, format["fields"]) )
        {
            //add and empty place holder for optional fields which are not trailing elements
            std::string ret_val = " ";
            components.push_back(ret_val);
        }
    }

    std::string line;
    for(std::size_t j=0; j<components.size(); j++)
    {
        line += components[j];
        if(j < components.size() - 1){ line += ":";}
    }
    return line;
}

MHO_VexElementLineGenerator::vex_element_type 
MHO_VexElementLineGenerator::DetermineType(std::string etype)
{
    if(etype == "int"){return vex_int_type;}
    if(etype == "list_int"){return vex_list_int_type;}
    if(etype == "real"){return vex_real_type;}
    if(etype == "list_real"){return vex_list_real_type;}
    if(etype == "epoch"){return vex_epoch_type;}
    if(etype == "ra"){return vex_radec_type;}
    if(etype == "dec"){return vex_radec_type;}
    if(etype == "string"){return vex_string_type;}
    if(etype == "list_string"){return vex_list_string_type;}
    if(etype == "compound"){return vex_compound_type;}
    if(etype == "list_compound"){return vex_list_compound_type;}
    if(etype == "keyword"){return vex_keyword_type;}
    if(etype == "reference"){return vex_reference_type;}
    if(etype == "link"){return vex_link_type;}
    return vex_unknown_type;
}


bool
MHO_VexElementLineGenerator::IsOptionalField(std::string& field_name)
{
    if( field_name.find_first_of("#") != std::string::npos){return true;}
    return false;
}

bool 
MHO_VexElementLineGenerator::IsTrailingOptionalField(std::string field_name, mho_json& fields)
{
    std::size_t start_idx = 0;
    for(std::size_t i=0; i<fields.size(); i++)
    {
        if( fields[i].get<std::string>() == field_name){start_idx = i; break;}
    }

    bool ret_val = true;
    for(std::size_t i=start_idx; i<fields.size(); i++)
    {
        std::string tmp_name = fields[i].get<std::string>();
        if( !IsOptionalField(tmp_name) ){ret_val = false;}
    }
    return ret_val;
}

}