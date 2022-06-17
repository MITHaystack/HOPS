#include "MHO_VexElementLineGenerator.hh"

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
            //ret_val = GenerateListInt(element_name, element);
        break;
        case vex_real_type:
            ret_val = GenerateReal(element_name, element);
        break;
        case vex_list_real_type:
            // = GenerateListReal(element_name, element);
        break;
        case vex_keyword_type:
            ret_val = GenerateKeyword(element_name, element);
        break;
        case vex_string_type:
            ret_val = GenerateString(element_name, element);
        break;
        case vex_list_string_type:
            //ret_val = GenerateListString(element_name, element);
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
        // case vex_compound_type: //all compound types treated the same way
        //     ret_val = GenerateCompound(element_name, element, format);
        // case vex_list_compound_type:
        //     element_data = ProcessCompound(element_name, format, tokens);
        // break;
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

    // std::string val;
    // return val;
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
        for(std::size_t i=0; i<obj.size(); i++)
        {
            if(i == 0)
            {
                val << " " << obj[i].get<double>() << " " << obj["units"].get<std::string>() << " ";
            }
            if( i != (obj.size()-1) ){ val << ":"; }
        }
    }
    else if(obj.contains("values"))
    {
        for(std::size_t i=0; i<obj.size(); i++)
        {
            val << " " << obj[i].get<double>() << " ";
            if( i != (obj.size()-1) ){ val << ":"; }
        }
    }
    else
    {
        msg_error("vex", "could not form valid vex string from json list_real element: "<< element_name << eom)
    }
    return val.str();
    // 
    // std::string val;
    // return val;
}

std::string 
MHO_VexElementLineGenerator::GenerateKeyword(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << " " << obj;//.get<std::string>() << " ";
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateString(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << " " << obj;//.get<std::string>() << " ";
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateListString(std::string element_name, mho_json& obj)
{
    // std::stringstream val;
    // for(std::size_t i=0; i<obj.size(); i++)
    // {
    //     val << " " << obj[i].get<std::string>() << " ";
    //     if( i != (obj.size()-1) ){ val << ":"; }
    // }
    // return val.str();

    std::string val;
    return val;
}

std::string 
MHO_VexElementLineGenerator::GenerateEpoch(std::string element_name, mho_json& obj)
{
    std::stringstream val;
    val << " " << obj;//.get<std::string>() << " ";
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
    val << " " << obj;//.get<std::string>() << " ";
    return val.str();
}

std::string 
MHO_VexElementLineGenerator::GenerateCompound(std::string element_name, mho_json& obj)
{
    std::string val;
    return val;
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


}