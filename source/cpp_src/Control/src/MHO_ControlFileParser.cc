#include "MHO_ControlFileParser.hh"

#include <fstream>


namespace hops
{

MHO_ControlFileParser::MHO_ControlFileParser()
{
    //read the block-names.json file
    fFormatDirectory = MHO_ControlDefinitions::GetFormatDirectory();

    // std::string block_names_file = fFormatDirectory + "keyword-names.json";
    // std::ifstream ifs;
    // ifs.open( block_names_file.c_str(), std::ifstream::in );
    // 
    // if(ifs.is_open())
    // {
    //     fKeywordNamesJSON = mho_json::parse(ifs);
    // }
    // ifs.close();
    // 
    // 
    fWhitespace = MHO_ControlDefinitions::WhitespaceDelim();
    fCommentFlag = MHO_ControlDefinitions::CommentFlag();
    fKeywordNames = MHO_ControlDefinitions::GetKeywordNames();
    
    
    // fKeywordNamesJSON["keyword_names"];

    for(auto blockIt = fKeywordNames.begin(); blockIt != fKeywordNames.end(); blockIt++)
    {
        std::cout<<"block name = "<< *blockIt << std::endl;
    }


}


MHO_ControlFileParser::~MHO_ControlFileParser(){};

void
MHO_ControlFileParser::SetControlFile(std::string filename)
{
    fControlFileName = filename;
}

mho_json
MHO_ControlFileParser::ParseControl()
{
    mho_json root;

    ReadFile(); //read file into memory
    RemoveComments(); //excise all comments
    TokenizeLines();
    MergeTokens();
    FindKeywords();

    for(auto it = fLines.begin(); it != fLines.end(); it++)
    {
        std::cout<<it->fContents<<std::endl;
    }

    //split the tokens into sections governed by a single keyword
    fStatements.clear();
    if(fKeywordLocations.size() > 0)
    {
        for(std::size_t i=0; i<fKeywordLocations.size(); i++)
        {
            std::size_t start = fKeywordLocations[i];
            std::size_t stop = fFileTokens.size();
            if(i < fKeywordLocations.size() - 1 )
            {
                stop = fKeywordLocations[i+1];
            }
            std::vector< std::string > tokens;
            for(std::size_t j = start+1; j < stop; j++){tokens.push_back(fFileTokens[j]);}
            MHO_ControlStatement stmt;
            stmt.fKeyword = fFileTokens[start];
            stmt.fTokens = tokens;
            fStatements.push_back(stmt);
        }
    }
    
    std::vector< mho_json > block_statements;
    mho_json empty_condition;
    empty_condition["name"] = "if";
    empty_condition["statement_type"] = "conditional";
    
    root["conditions"].push_back(empty_condition);

    for(std::size_t i=0; i<fStatements.size(); i++)
    {
        mho_json tmp = fElementParser.ParseControlStatement(fStatements[i]);
        if(tmp["statement_type"] == "conditional" || i == fStatements.size()-1)
        {
            root["conditions"].back()["statements"] = block_statements;
            root["conditions"].push_back(tmp);
            block_statements.clear();
        }
        else 
        {
            block_statements.push_back(tmp);
        }
    }

    std::cout<< root.dump(2) << std::endl;

    return root;
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
    auto it = fLines.begin();
    while(it != fLines.end())
    {
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
        for(auto blockIt = fKeywordNames.begin(); blockIt != fKeywordNames.end(); blockIt++)
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
