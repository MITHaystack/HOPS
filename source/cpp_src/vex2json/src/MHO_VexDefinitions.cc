#include "MHO_VexDefinitions.hh"
#include "MHO_JSONHeaderWrapper.hh"

#include <fstream>

namespace hops 
{

MHO_VexDefinitions::MHO_VexDefinitions()
{
    fStartTag = "def";
    fStopTag = "enddef";
    fBlockStartFlag = "$";
    fChanDefTag = "chan_def";
    fIFDefTag = "if_def";
    fRefTag = "ref";
    fAssignmentDelim = "=;";
    fWhitespaceDelim = " \t\r\n";
    fVexDelim = ":";
    fStatementEndFlag = ";";
    fVexRevisionFlag = "VEX_rev";
    fStartTagDelim = fWhitespaceDelim + ";";
    fStartLiteralFlag = "start_literal";
    fEndLiteralFlag = "end_literal";
    fCommentFlag = "*";
    //default set-up: vex-1.5
    SetVexVersion("1.5");

}

MHO_VexDefinitions::~MHO_VexDefinitions(){}

void MHO_VexDefinitions::SetVexVersion(std::string version)
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
MHO_VexDefinitions::SetVexVersion(const char* version)
{
    std::string vers(version);
    SetVexVersion(vers);
}

std::string 
MHO_VexDefinitions::GetFormatDirectory() const
{
    std::string format_dir = VEX_FORMAT_DIR;
    format_dir += "/vex-" + fVexVersion + "/";
    return format_dir;
}

vex_element_type 
MHO_VexDefinitions::DetermineType(std::string etype) const
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