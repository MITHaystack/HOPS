#include "MHO_VexGenerator.hh"
#include <fstream>

namespace hops 
{
    

MHO_VexGenerator::MHO_VexGenerator(){}
MHO_VexGenerator::~MHO_VexGenerator(){};

void MHO_VexGenerator::SetFilename(std::string filename)
{
    fFilename = filename;
}

void MHO_VexGenerator::GenerateVex(mho_json& root)
{
    std::vector< std::string > all_lines;
    
    //open block-names file for this version 

    //loop over blocks, and extract the data from from the root and append them
    for(auto blk_it = fBlockNames.begin(); blk_it != fBlockNames.end(); blk_it++)
    {
        std::string block_name = *blk_it;
        std::vector< std::string > block_lines;
        ConstructBlockLines(root, block_name, block_lines);
        std::string block_opening = block_name + ";\n";
        all_lines.push_back(block_opening);
        all_lines.insert(all_lines.end(), block_lines.begin(), block_lines.end());
    }

    //open file and write out lines 
    for(auto lit = all_lines.begin(); lit != all_lines.end(); lit++)
    {
        std::cout<< *lit ;
    }


    //close file

}


void 
MHO_VexGenerator::ConstructBlockLines(mho_json& root, std::string block_name, std::vector< std::string >& lines)
{
    lines.clear();
    LoadBlockFormat(block_name);
    if( fBlockFormat["block_type"].get<std::string>() == "primitive" ) //only do primitive blocks for now
    {
        std::string start_tag = fBlockFormat["start_tag"].get<std::string>();
        std::string stop_tag = fBlockFormat["stop_tag"].get<std::string>();

        if( root.contains(block_name) )
        {
            mho_json block = root[block_name];
            for(auto& element : block.items())
            {
                std::string element_key = element.key();
                std::string start_line = start_tag + " " + element_key + ";\n";
                lines.push_back(start_line);
                //ConstructElementLines(element, lines);
                std::string stop_line = stop_tag + ";\n";
                lines.push_back(stop_line);
            }
        }
    }
}



// void 
// MHO_VexGenerator::ConstructElementLines(mho_json& element, std::vector< std::string >& lines)
// {
//     //loop over items in format, and extract from element
// 
// }


void 
MHO_VexGenerator::SetVexVersion(std::string version)
{
    fVexVersion = "1.5";
    if(version.find("1.5") != std::string::npos ){fVexVersion = "1.5";}
    else if(version.find("2.0") != std::string::npos ){fVexVersion = "2.0";}
    else 
    {
        msg_error("vex", "version string: "<< version << "not understood, defaulting to vex version 1.5." << eom );
    }

    fFormatDirectory = GetFormatDirectory();
    std::string bnames_file = fFormatDirectory + "block-names.json";
    msg_debug("vex", "block name file is: "<< bnames_file << eom);

    std::ifstream bn_ifs;
    bn_ifs.open( bnames_file.c_str(), std::ifstream::in );

    json bnames;
    if(bn_ifs.is_open())
    {
        bnames = mho_ordered_json::parse(bn_ifs);
    }
    bn_ifs.close();

    fBlockNames.clear();
    for(auto it = bnames["block_names"].begin(); it != bnames["block_names"].end(); it++)
    {
        fBlockNames.push_back(*it);
    }
}

void 
MHO_VexGenerator::SetVexVersion(const char* version)
{
    std::string vers(version);
    SetVexVersion(vers);
}


std::string 
MHO_VexGenerator::GetFormatDirectory() const
{
    std::string format_dir = VEX_FORMAT_DIR;
    format_dir += "/vex-" + fVexVersion + "/";
    return format_dir;
}



void 
MHO_VexGenerator::LoadBlockFormat(std::string block_name)
{
    fBlockFormatLoaded = false;
    std::string block_format_file = GetBlockFormatFileName(block_name);
    std::string format_file = fFormatDirectory + block_format_file;

    std::ifstream bf_ifs;
    bf_ifs.open( format_file.c_str(), std::ifstream::in );

    mho_json bformat;
    if(bf_ifs.is_open())
    {
        bformat = mho_json::parse(bf_ifs);
        fBlockFormatLoaded = true;
    }
    bf_ifs.close();

    if(fBlockFormatLoaded)
    {
        fBlockFormat = bformat;
    }
}

std::string 
MHO_VexGenerator::GetBlockFormatFileName(std::string block_name)
{
    //remove '$', and convert to lower-case
    std::string file_name = block_name.substr(1);
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    file_name += ".json";
    return file_name;
}


}//end of namespace