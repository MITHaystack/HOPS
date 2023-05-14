#include "MHO_ControlFileParser.hh"

#include <fstream>


namespace hops
{

MHO_ControlFileParser::MHO_ControlFileParser()
{
    //read the block-names.json file
    fFormatDirectory = HOPS_CONTROL_FORMAT_DIR;
    fFormatDirectory += "/control/";

    std::string block_names_file = fFormatDirectory + "keyword-names.json";
    std::ifstream ifs;
    ifs.open( block_names_file.c_str(), std::ifstream::in );

    if(ifs.is_open())
    {
        fBlockNamesJSON = mho_json::parse(ifs);
    }
    ifs.close();

    fBlockNames = fBlockNamesJSON["keyword_names"];

    for(auto blockIt = fBlockNames.begin(); blockIt != fBlockNames.end(); blockIt++)
    {
        std::cout<<"block name = "<< *blockIt << std::endl;
    }

    fWhitespace = " \t\r\n";
    fCommentFlag = "*";
}


MHO_ControlFileParser::~MHO_ControlFileParser(){};

void
MHO_ControlFileParser::SetControlFile(std::string filename)
{
    fControlFileName = filename;
}

//mho_json
void
MHO_ControlFileParser::ParseControl()
{
    ReadFile(); //read file into memory
    RemoveComments(); //excise all comments
    TokenizeLines();
    MergeTokens();
    FindKeywords();

    for(auto it = fLines.begin(); it != fLines.end(); it++)
    {
        std::cout<<it->fContents<<std::endl;
    }

    for(std::size_t i=0; i<fKeywordLocations.size(); i++)
    {
        std::cout<<"keyword: "<<fFileTokens[fKeywordLocations[i]]<<" at index: "<<fKeywordLocations[i]<<std::endl;
    }

    //SplitStatements(); //split multiple ";" on one line into as many statements as needed
    // JoinLines(); //join incomplete segments split across multiple lines into a single statement
    // IndexStatements();
    // MarkBlocks(); //mark the major parsable sections

    // mho_json root;
    // root[fControlDef.ControlRevisionFlag()] = fControlVersion;
    // ProcessBlocks(root);

    // return root;
}

void
MHO_ControlFileParser::ReadFile()
{
    //nothing special, just read in the entire file line by line and stash in memory
    if(fControlFileName != "")
    {
        //open input/output files
        std::ifstream vfile(fControlFileName.c_str(), std::ifstream::in);
        if(vfile.is_open() )
        {
            std::size_t line_count = 1;
            std::string contents;
            while( getline(vfile, contents) )
            {
                MHO_ControlLine current_line;
                current_line.fLineNumber = line_count;
                current_line.fContents = contents;
                fLines.push_back(current_line);
                line_count++;
            }
            vfile.close();
        }
        else
        {
            msg_error("control", "could not open file: "<<fControlFileName<<eom);
        }
    }
}


void
MHO_ControlFileParser::RemoveComments()
{
    std::string flag = fCommentFlag;
    for(auto it = fLines.begin(); it != fLines.end();)
    {
        std::size_t com_pos = it->fContents.find_first_of(flag);
        if(com_pos != std::string::npos || it->fContents.size() == 0)
        {
            if(com_pos == 0){it = fLines.erase(it);} //this entire line is a comment
            else
            {
                std::string trimmed = it->fContents.substr(0,com_pos);
                if(trimmed.size() != 0)
                {
                    it->fContents = trimmed;
                    it++;
                }
                else{it = fLines.erase(it);}
            }
        }
        else{it++;}
    }
}

void
MHO_ControlFileParser::TokenizeLines()
{
    fTokenizer.SetDelimiter(fWhitespace);
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    fTokenizer.SetIncludeEmptyTokensFalse();

    auto it = fLines.begin();
    while(it != fLines.end())
    {
        it->fTokens.clear();
        fTokenizer.SetString( &(it->fContents) );
        fTokenizer.GetTokens( &(it->fTokens) );
        it++;
    }
}


void
MHO_ControlFileParser::MergeTokens()
{
    fFileTokens.clear();
    fLineStartLocations.clear();
    auto it = fLines.begin();
    while(it != fLines.end())
    {
        fLineStartLocations.push_back(fFileTokens.size());
        fFileTokens.insert( fFileTokens.end(), it->fTokens.begin(), it->fTokens.end() );
        it++;
    }
}

void
MHO_ControlFileParser::FindKeywords()
{
    //brute force search
    fKeywordLocations.clear();
    for(auto tokenIt = fFileTokens.begin(); tokenIt != fFileTokens.end(); tokenIt++)
    {
        for(auto blockIt = fBlockNames.begin(); blockIt != fBlockNames.end(); blockIt++)
        {
            if(*tokenIt == * blockIt)
            {
                std::cout<<*tokenIt<<" : "<<*blockIt<<std::endl;
                fKeywordLocations.push_back( std::distance(fFileTokens.begin(), tokenIt) );
                break;
            }
        }
    }
}


}//end namespace
