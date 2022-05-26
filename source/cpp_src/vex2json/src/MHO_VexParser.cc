#include "MHO_VexParser.hh"

#include <fstream>


namespace hops 
{

MHO_VexParser::MHO_VexParser()
{
    fVexDelim = " :;\t\r\n";
    fWhitespace = " \t\r\n";
    fCommentFlag = "*";
    fVexVersion = "1.5";
    fFormatDirectory = VEX_FORMAT_DIR;
}


MHO_VexParser::~MHO_VexParser(){};

void 
MHO_VexParser::SetVexFile(std::string filename){fVexFileName = filename;}

void 
MHO_VexParser::ReadFile()
{
    //nothing special, just read in the entire file line by line and stash in memory
    if(fVexFileName != "")
    {
        //open input/output files
        std::ifstream vfile(fVexFileName.c_str(), std::ifstream::in);
        if(vfile.is_open() )
        {
            while( getline(vfile, fLine) )
            {
                fLines.push_back(fLine);
            }
            vfile.close();
        }
    }
}

void 
MHO_VexParser::RemoveComments()
{
    for(auto it = fLines.begin(); it != fLines.end();)
    {
        std::size_t com_pos = it->find_first_of(fCommentFlag);
        if(com_pos != std::string::npos)
        {
            if(com_pos == 0){it = fLines.erase(it);} //this entire line is a comment
            else
            {
                std::string trimmed = it->substr(0,com_pos);
                if(trimmed.size() != 0)
                {
                    *it = trimmed;
                    it++;
                }
                else{it = fLines.erase(it);}
            }
        }
        else{it++;}
    }
}

void 
MHO_VexParser::ParseVex()
{

}


void 
MHO_VexParser::SetVexVersion(std::string version)
{
    fVexVersion = "1.5";
    if(version == "1.5")
    {
        fVexVersion = version;
        return;
    }

    if(version == "2.0")
    {
        fVexVersion = version;
        return;
    }

    msg_error("vex", "version string: "<< version << "not understood, defaulting to vex version 1.5." << eom );

}

void 
MHO_VexParser::SetVexVersion(const char* version)
{
    std::string vers(version);
    SetVexVersion(vers);
}

std::string 
MHO_VexParser::GetFormatDirectory()
{
    std::string format_dir = VEX_FORMAT_DIR;
    format_dir += "/vex-" + fVexVersion + "/";
    return format_dir;
}

}