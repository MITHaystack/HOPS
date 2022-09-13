#include "MHO_VexDefinitions.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Tokenizer.hh"

#include <fstream>

namespace hops 
{

MHO_VexDefinitions::MHO_VexDefinitions()
{
    //default set-up: vex-1.5
    SetVexVersion("1.5");

}

MHO_VexDefinitions::~MHO_VexDefinitions(){}

void MHO_VexDefinitions::SetVexVersion(std::string version)
{
    fVexVersion = "1.5";
    if(version.find("1.5") != std::string::npos ){fVexVersion = "1.5";}
    else if(version.find("2.0") != std::string::npos ){fVexVersion = "2.0";}
    else if(version.find("ovex") != std::string::npos){fVexVersion = "ovex";}
    else if(version.find("OVEX") != std::string::npos){fVexVersion = "ovex";}
    else
    {
        msg_error("vex", "version string: "<< version << " not understood, defaulting to vex version 1.5." << eom );
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
MHO_VexDefinitions::SetVexVersion(const char* version)
{
    std::string vers(version);
    SetVexVersion(vers);
}

std::string 
MHO_VexDefinitions::GetFormatDirectory() const
{
    std::string format_dir = VEX_FORMAT_DIR;
    if(fVexVersion == "ovex")
    {
        format_dir += "/ovex/";
    }
    else 
    {
        format_dir += "/vex-" + fVexVersion + "/";
    }
    return format_dir;
}

std::string 
MHO_VexDefinitions::DetermineFileVersion(std::string filename)
{
    //read the first line to determine the vex revision
    //vex standard states the revision statement must start at 1st char of 1st line
    std::string revision = "unknown";
    //open input/output files
    std::ifstream vfile(filename.c_str(), std::ifstream::in);
    if(vfile.is_open() )
    {
        std::size_t line_count = 1;
        std::string contents;
        getline(vfile, contents);
        if(contents.find( VexRevisionFlag() ) != std::string::npos) //check for vex
        {
            std::size_t start_pos = contents.find_first_of("=");
            std::size_t end_pos = contents.find_first_of(";");
            if(start_pos != std::string::npos && end_pos != std::string::npos)
            {
                std::string rev = contents.substr(start_pos+1, end_pos-start_pos-1);
                rev = MHO_Tokenizer::TrimLeadingAndTrailingWhitespace(rev);

                //sanitize the version statment (only pass 1.5 or 2.0)
                if(rev.find("1.5") != std::string::npos ){revision = "1.5";}
                else if(rev.find("2.0") != std::string::npos ){revision = "2.0";}
                else if(rev.find("ovex") != std::string::npos ){revision = "ovex";}
                else if(rev.find("OVEX") != std::string::npos ){revision = "ovex";}
                else 
                {
                    msg_error("vex", "version string: "<< rev << " not understood." << eom );
                }
            }
        }
        else if (contents.find( OVexRevisionFlag() ) != std::string::npos) //check for ovex
        {
            revision = "ovex";
        }
        else 
        {
            msg_error("vex", "could not determine vex version of file: "<< filename << "." << eom );
        }
        vfile.close();
    }
    else 
    {
        msg_error("vex", "could not open file: "<<filename<<eom);
    }
    return revision;
}


vex_element_type 
MHO_VexDefinitions::DetermineType(std::string etype)
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
MHO_VexDefinitions::IsOptionalField(std::string& field_name)
{
    if( field_name.find_first_of( OptionalFlag() ) != std::string::npos){return true;}
    return false;
}

}