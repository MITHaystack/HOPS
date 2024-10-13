#include "MHO_AFileDefinitions.hh"

#include <fstream>
#include <iostream>

namespace hops
{

std::string MHO_AFileDefinitions::GetFormatDirectory(const std::string& file_type)
{
    //allowed file types are: root, frng, and cor
    std::string format_dir = HOPS_AFILE_FORMAT_DIR;
    format_dir += "/afio/" + file_type + "/";
    return format_dir;
}

std::vector< std::string > MHO_AFileDefinitions::GetKeywordNames(const std::string& file_type)
{

    std::vector< std::string > keywords;
    std::string format_dir = GetFormatDirectory(file_type);
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

mho_json MHO_AFileDefinitions::GetAFileFormat(const std::string& file_type)
{
    //load all of the .json format files here so they are in memory
    //TODO -- we may also want to combine all the format files into a single file

    std::string format_dir = GetFormatDirectory(file_type);
    std::vector< std::string > keywords = GetKeywordNames(file_type);
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

    // std::cout<<"format = "<<format_obj.dump(2)<<std::endl;

    return format_obj;
}

} // namespace hops
