#include "MHO_VexElementLineGenerator.hh"

namespace hops 
{

MHO_VexElementLineGenerator::MHO_VexElementLineGenerator(){};
MHO_VexElementLineGenerator::~MHO_VexElementLineGenerator(){};

std::string 
MHO_VexElementLineGenerator::ConstructElementLine(mho_json& element, mho_json& format, std::vector< std::string >& lines)
{
    //loop over items in format, and extract from element
    std::string hash = "#";
    std::string nothing = "";
    for(auto field: fBlockFormat["fields"].items())
    {
        std::string raw_field_name = field.value().get<std::string>();
        //remove # prefix indicating optional elements 
        std::string field_name = std::regex_replace(raw_field_name,std::regex(hash),nothing);
        if(element.contains(field_name))
        {
            std::string par_type = fBlockFormat["parameters"][field_name]["type"].get<std::string>();
            if(par_type.find("list_compound") != std::string::npos)
            {
                //this is a list of compound elements
            }
            else if (par_type.find("list") != std::string::npos)
            {
                //this is a list element
            }
            else if (par_type.find("compound") != std::string::npos)
            {
                //this is a compound element
            }
            else 
            {
                if(par_type.find("list_real") != std::string::npos)
                {
                    // std::stringstream val;
                    // mho_json obj = element[field_name];
                    // val << field_name << " = " << obj << ";\n";
                    // lines.push_back(val.str());
                }
                if(par_type.find("real") != std::string::npos)
                {
                    std::stringstream val;
                    mho_json obj = element[field_name];
                    if(obj.contains("units"))
                    {
                        val << field_name << " = " << obj["value"].get<double>() << " " << obj["units"].get<std::string>() << ";\n";
                    }
                    else
                    {
                        val << field_name << " = " << obj["value"].get<double>() << ";\n";
                    }
                    lines.push_back(val.str());
                }
                else
                {
                    std::stringstream val;
                    mho_json obj = element[field_name];
                    val << field_name << " = " << obj << ";\n";
                    lines.push_back(val.str());
                }
            }

        }
        //std::cout<<field_name<<std::endl;
    }
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