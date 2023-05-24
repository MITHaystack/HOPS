#include "MHO_ControlFileParser.hh"

#include <fstream>
#include <regex>


namespace hops
{

MHO_ControlFileParser::MHO_ControlFileParser()
{
    //read the block-names.json file
    fFormatDirectory = MHO_ControlDefinitions::GetFormatDirectory();
    fWhitespace = MHO_ControlDefinitions::WhitespaceDelim();
    fCommentFlag = MHO_ControlDefinitions::CommentFlag();
    fKeywordNames = MHO_ControlDefinitions::GetKeywordNames();
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
    bool ok;
    mho_json root;

    ok = ReadFile();
    if(!ok){return root;}

    RemoveComments();
    FixSymbols();
    TokenizeLines();
    MergeTokens();
    FindKeywords();
    FormStatements();

    root = ConstructControlObjects();
    return root;
}

bool
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
            return true;
        }
        else
        {
            msg_error("control", "could not open file: "<<fControlFileName<<eom);
        }
    }
    return false;
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
MHO_ControlFileParser::FixSymbols()
{
    //makes sure that if we have a parentheses, or ! or <, > symbol these get padded with space
    //so that they are treated as a separate token
    std::string bare_open_paren("(");
    std::string bare_close_paren(")");
    std::string open_paren("\\(");
    std::string close_paren("\\)");
    std::string lt("<");
    std::string gt(">");

    std::string fixed_open_paren(" ( ");
    std::string fixed_close_paren(" ) ");
    std::string fixed_lt(" < ");
    std::string fixed_gt(" > ");

    auto it = fLines.begin();
    while(it != fLines.end())
    {
        FindAndReplace(bare_open_paren, open_paren, fixed_open_paren, it->fContents);
        FindAndReplace(bare_close_paren, close_paren, fixed_close_paren, it->fContents);
        FindAndReplace(lt, lt, fixed_lt, it->fContents);
        FindAndReplace(gt, gt, fixed_gt, it->fContents);
        it++;
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
    std::vector< std::string > workspace;
    while(it != fLines.end())
    {
        workspace.clear();
        fTokenizer.SetString( &(it->fContents) );
        fTokenizer.GetTokens( &workspace );

        it->fTokens.clear();
        for(std::size_t i=0; i<workspace.size(); i++)
        {
            MHO_Token tmp;
            tmp.fValue = workspace[i];
            tmp.fLineNumber = it->fLineNumber;
            it->fTokens.push_back(tmp);
        }
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
            if(tokenIt->fValue == * blockIt)
            {
                fKeywordLocations.push_back( std::distance(fFileTokens.begin(), tokenIt) );
                break;
            }
        }
    }
}

void
MHO_ControlFileParser::FormStatements()
{
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
            std::vector< MHO_Token > tokens;
            for(std::size_t j = start+1; j < stop; j++){tokens.push_back(fFileTokens[j]);}
            MHO_ControlStatement stmt;
            stmt.fStartLineNumber = fFileTokens[start].fLineNumber;
            stmt.fKeyword = fFileTokens[start].fValue;
            stmt.fTokens = tokens;
            fStatements.push_back(stmt);
        }
    }
}


mho_json
MHO_ControlFileParser::ConstructControlObjects()
{
    mho_json root;
    std::vector< mho_json > block_statements;
    mho_json empty_condition;
    empty_condition["name"] = "if";
    std::vector< std::string > dummy; dummy.push_back( std::string("true") );
    empty_condition["value"] = dummy;
    empty_condition["statement_type"] = "conditional";
    empty_condition["line_number"] = 0;

    root["conditions"].push_back(empty_condition);

    for(std::size_t i=0; i<fStatements.size(); i++)
    {
        mho_json tmp = fElementParser.ParseControlStatement(fStatements[i]);
        if(tmp["statement_type"] == "conditional" )
        {
            tmp["line_number"] = fStatements[i].fStartLineNumber;
            root["conditions"].back()["statements"] = block_statements;
            root["conditions"].push_back(tmp);
            block_statements.clear();
        }
        else if (i == fStatements.size()-1)
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
    return root;
}


void
MHO_ControlFileParser::FindAndReplace(const std::string& find_str, const std::string& regex_str, const std::string& replace_str, std::string& text)
{
    if( text.find(find_str) != std::string::npos )
    {
        std::string fixed_line = std::regex_replace(text, std::regex(regex_str), replace_str);
        text = fixed_line;
    }
}



}//end namespace
