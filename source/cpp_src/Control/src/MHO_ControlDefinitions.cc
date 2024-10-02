#include "MHO_ControlDefinitions.hh"

#include <fstream>
#include <iostream>

namespace hops
{

std::string MHO_ControlDefinitions::GetFormatDirectory()
{
    std::string format_dir = HOPS_CONTROL_FORMAT_DIR;
    format_dir += "/control/";
    return format_dir;
}

std::vector< std::string > MHO_ControlDefinitions::GetKeywordNames()
{

    std::vector< std::string > keywords;
    std::string format_dir = GetFormatDirectory();
    MHO_DirectoryInterface dirInterface;

    dirInterface.SetCurrentDirectory(format_dir);
    dirInterface.ReadCurrentDirectory();
    dirInterface.GetFilesMatchingExtention(keywords, "json");

    for(std::size_t i = 0; i < keywords.size(); i++)
    {
        std::string tmp = MHO_DirectoryInterface::GetBasename(keywords[i]);
        keywords[i] = MHO_DirectoryInterface::StripExtensionFromBasename(tmp);
    }

    return keywords;
}

control_element_type MHO_ControlDefinitions::DetermineControlType(std::string etype)
{
    if(etype == "int")
    {
        return control_int_type;
    }
    if(etype == "list_int")
    {
        return control_list_int_type;
    }
    if(etype == "real")
    {
        return control_real_type;
    }
    if(etype == "list_real")
    {
        return control_list_real_type;
    }
    if(etype == "string")
    {
        return control_string_type;
    }
    if(etype == "list_string")
    {
        return control_list_string_type;
    }
    if(etype == "fixed_length_list_string")
    {
        return control_fixed_length_list_string_type;
    }
    if(etype == "conditional")
    {
        return control_conditional_type;
    }
    if(etype == "bool")
    {
        return control_bool_type;
    }
    if(etype == "compound")
    {
        return control_compound_type;
    }
    return control_unknown_type;
}

mho_json MHO_ControlDefinitions::GetControlFormat()
{
    //load all of the .json format files here so they are in memory
    //TODO -- we may also want to combine all the format files into a single file

    std::string format_dir = GetFormatDirectory();
    std::vector< std::string > keywords = GetKeywordNames();
    mho_json format_obj;

    for(auto keyIt = keywords.begin(); keyIt != keywords.end(); keyIt++)
    {
        std::string key = *keyIt;
        std::string element_format_file = key + ".json";
        std::string format_file = format_dir + element_format_file;

        //TODO should check that the file exists
        std::ifstream bf_ifs;
        bf_ifs.open(format_file.c_str(), std::ifstream::in);

        mho_json bformat;
        if(bf_ifs.is_open())
        {
            bformat = mho_json::parse(bf_ifs);
            format_obj[key] = bformat;
        }
        bf_ifs.close();
    }

    return format_obj;
}

} // namespace hops
