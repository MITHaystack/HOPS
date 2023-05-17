#include "MHO_ControlDefinitions.hh"

#include <iostream>

namespace hops
{

std::string 
MHO_ControlDefinitions::GetFormatDirectory()
{
    std::string format_dir = HOPS_CONTROL_FORMAT_DIR;
    format_dir += "/control/";
    return format_dir;
}

std::vector< std::string > 
MHO_ControlDefinitions::GetKeywordNames()
{
    
    std::vector< std::string > keywords;
    std::string format_dir = GetFormatDirectory();
    MHO_DirectoryInterface dirInterface;

    dirInterface.SetCurrentDirectory(format_dir);
    
    std::cout<<"format_dir = "<<format_dir<<std::endl;
    
    dirInterface.ReadCurrentDirectory();
    dirInterface.GetFilesMatchingExtention(keywords, "json");
    
    std::cout<<"n keyword files = "<<keywords.size()<<std::endl;
    
    for(std::size_t i=0; i<keywords.size(); i++)
    {
        std::string tmp = MHO_DirectoryInterface::GetBasename( keywords[i] );
        std::cout<<"tmp = "<<keywords[i]<<std::endl;
        keywords[i] = MHO_DirectoryInterface::StripExtensionFromBasename(tmp);
    }
    
    return keywords;
}


control_element_type 
MHO_ControlDefinitions::DetermineControlType(std::string etype)
{
    if(etype == "int"){return control_int_type;}
    if(etype == "list_int"){return control_list_int_type;}
    if(etype == "real"){return control_real_type;}
    if(etype == "list_real"){return control_list_real_type;}
    if(etype == "string"){return control_string_type;}
    if(etype == "list_string"){return control_list_string_type;}
    if(etype == "conditional"){return control_conditional_type;}
    if(etype == "compound"){return control_compound_type;}
    return control_unknown_type;
}


}
