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
    fSetString = "";
    fProcessedControlFileText = "";
}

MHO_ControlFileParser::~MHO_ControlFileParser(){};

void MHO_ControlFileParser::SetControlFile(std::string filename)
{
    fControlFileName = filename;
}

mho_json MHO_ControlFileParser::ParseControl()
{
    bool ok;
    mho_json root;

    ok = ReadFile();
    if(!ok)
    {
        return root;
    }

    RemoveComments();
    FixSymbols();
    TokenizeLines();
    MergeTokens();

    //exports the cleaned-up tokens to fProcessedControlFileText
    //this is only for record keeping
    ExportTokens();

    FindKeywords();
    FormStatements();

    root = ConstructControlObjects();
    return root;
}

bool MHO_ControlFileParser::ReadFile()
{
    //the set string needs to be handled here
    //this can be tricky because the set string may contain conditional statements
    //like 'if station G'
    //the problem with this is that if we prepend the control file with the set string
    //then we may be accidentally prepending the entire control file contents with an
    //'if' statement which does not apply. So, what we have to do is check if the
    //set string contains an 'if' and if so, we need to split its contents on the first
    //instance of 'if'. Anything before this if statement should be prepended to the
    //control file, while anything after it should be appended to the end of the control file

    std::string prepend = "";
    std::string append = "";
    SplitSetString(fSetString, prepend, append);

    //prepend applicable portion of the set string (give it line #0)
    //appends empty string "" if nothing in set string
    MHO_ControlLine tmp_line;
    tmp_line.fLineNumber = 0;
    tmp_line.fContents = prepend;
    fLines.push_back(tmp_line);

    //nothing special, just read in the entire file line by line and stash in memory
    bool status = false;
    std::size_t line_count = 1;
    if(fControlFileName != "")
    {
        //open input/output files
        std::ifstream vfile(fControlFileName.c_str(), std::ifstream::in);
        if(vfile.is_open())
        {
            std::string contents;
            while(getline(vfile, contents))
            {
                MHO_ControlLine current_line;
                current_line.fLineNumber = line_count;
                current_line.fContents = contents;
                fLines.push_back(current_line);
                line_count++;
            }
            vfile.close();
            status = true;
        }
        else
        {
            msg_fatal("control", "could not open control file: " << fControlFileName << eom);
            std::exit(1);
        }
    }

    //append applicable portion of the set string
    //this append an empty string "" if  there is nothing in the set string
    tmp_line.fLineNumber = line_count;
    tmp_line.fContents = append;
    fLines.push_back(tmp_line);

    return status;
}

void MHO_ControlFileParser::RemoveComments()
{
    std::string flag = fCommentFlag;
    for(auto it = fLines.begin(); it != fLines.end();)
    {
        std::size_t com_pos = it->fContents.find_first_of(flag);
        // if(com_pos != std::string::npos || it->fContents.size() == 0)
        if(com_pos != std::string::npos) //this allows empty lines to pass through
        {
            if(com_pos == 0)
            {
                it = fLines.erase(it);
            } //this entire line is a comment
            else
            {
                std::string trimmed = it->fContents.substr(0, com_pos);
                if(trimmed.size() != 0)
                {
                    it->fContents = trimmed;
                    it++;
                }
                else
                {
                    it = fLines.erase(it);
                }
            }
        }
        else
        {
            it++;
        }
    }
}

void MHO_ControlFileParser::FixSymbols()
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

void MHO_ControlFileParser::TokenizeLines()
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
        fTokenizer.SetString(&(it->fContents));
        fTokenizer.GetTokens(&workspace);

        it->fTokens.clear();
        for(std::size_t i = 0; i < workspace.size(); i++)
        {
            MHO_Token tmp;
            tmp.fValue = workspace[i];
            tmp.fLineNumber = it->fLineNumber;
            it->fTokens.push_back(tmp);
        }
        it++;
    }
}

void MHO_ControlFileParser::MergeTokens()
{
    fFileTokens.clear();
    fLegacyFileTokens.clear();
    auto it = fLines.begin();
    line_itr it2;
    while(it != fLines.end())
    {
        fFileTokens.insert(fFileTokens.end(), it->fTokens.begin(), it->fTokens.end());
        it2 = it;
        it2++;
        //exclude the very first and last lines (set string lines) from the legacy tokens
        if(it != fLines.begin() && it2 != fLines.end())
        {
            fLegacyFileTokens.insert(fLegacyFileTokens.end(), it->fTokens.begin(), it->fTokens.end());
        }
        it = it2;
    }
}

void MHO_ControlFileParser::ExportTokens()
{
    fProcessedControlFileText = "";

    for(auto it = fFileTokens.begin(); it != fFileTokens.end();)
    {
        fProcessedControlFileText += it->fValue;
        it++;
        if(it != fFileTokens.end())
        {
            fProcessedControlFileText += " ";
        }
    }

    fLegacyProcessedControlFileText = "";
    for(auto it = fLegacyFileTokens.begin(); it != fLegacyFileTokens.end();)
    {
        fLegacyProcessedControlFileText += it->fValue;
        it++;
        if(it != fLegacyFileTokens.end())
        {
            fLegacyProcessedControlFileText += " ";
        }
    }
}

void MHO_ControlFileParser::FindKeywords()
{
    //brute force search
    fKeywordLocations.clear();
    for(auto tokenIt = fFileTokens.begin(); tokenIt != fFileTokens.end(); tokenIt++)
    {
        for(auto blockIt = fKeywordNames.begin(); blockIt != fKeywordNames.end(); blockIt++)
        {
            if(tokenIt->fValue == *blockIt)
            {
                fKeywordLocations.push_back(std::distance(fFileTokens.begin(), tokenIt));
                break;
            }
        }
    }
}

void MHO_ControlFileParser::FormStatements()
{
    //split the tokens into sections governed by a single keyword
    fStatements.clear();
    if(fKeywordLocations.size() > 0)
    {
        for(std::size_t i = 0; i < fKeywordLocations.size(); i++)
        {
            std::size_t start = fKeywordLocations[i];
            std::size_t stop = fFileTokens.size();
            if(i < fKeywordLocations.size() - 1)
            {
                stop = fKeywordLocations[i + 1];
            }
            std::vector< MHO_Token > tokens;
            for(std::size_t j = start + 1; j < stop; j++)
            {
                tokens.push_back(fFileTokens[j]);
            }
            MHO_ControlStatement stmt;
            stmt.fStartLineNumber = fFileTokens[start].fLineNumber;
            stmt.fKeyword = fFileTokens[start].fValue;
            stmt.fTokens = tokens;
            fStatements.push_back(stmt);
        }
    }
}

mho_json MHO_ControlFileParser::ConstructControlObjects()
{
    mho_json root;
    std::vector< mho_json > block_statements;
    mho_json empty_condition;
    empty_condition["name"] = "if";
    std::vector< std::string > dummy;
    dummy.push_back(std::string("true"));
    empty_condition["value"] = dummy;
    empty_condition["statement_type"] = "conditional";
    empty_condition["line_number"] = 0;

    root["conditions"].push_back(empty_condition);

    for(std::size_t i = 0; i < fStatements.size(); i++)
    {
        mho_json tmp = fElementParser.ParseControlStatement(fStatements[i]);
        if(tmp["statement_type"] == "conditional") //enter new conditional block 'if'
        {
            tmp["line_number"] = fStatements[i].fStartLineNumber;
            root["conditions"].back()["statements"] = block_statements;
            root["conditions"].push_back(tmp);
            block_statements.clear();
        }
        else if(i == fStatements.size() - 1) //reached the last statement in the CF, close out
        {
            block_statements.push_back(tmp);
            root["conditions"].back()["statements"] = block_statements;
            block_statements.clear();
        }
        else
        {
            block_statements.push_back(tmp); //just another statement in current conditional block, add it
        }
    }
    return root;
}

void MHO_ControlFileParser::FindAndReplace(const std::string& find_str, const std::string& regex_str,
                                           const std::string& replace_str, std::string& text)
{
    if(text.find(find_str) != std::string::npos)
    {
        std::string fixed_line = std::regex_replace(text, std::regex(regex_str), replace_str);
        text = fixed_line;
    }
}

void MHO_ControlFileParser::SplitSetString(const std::string& set_string, std::string& prepend, std::string& append)
{
    prepend = "";
    append = "";
    //(new behavior) prepend is always empty
    if(set_string != "")
    {
        prepend = "";
        append = " if true " + set_string;
    }

    //original behavior below, however this is probably undesirable because
    //usually the user expects 'set' arguments to take precedence over any
    //control file statements, but the behavior below allows any non-if'd statements
    //in the set string to get masked by a control file statement

    // if(set_string != "")
    // {
    //     std::string if_flag = "if";
    //     std::size_t first_if_pos = set_string.find(if_flag);
    //     if(first_if_pos == std::string::npos)
    //     {
    //         //no 'if' statement in the set string, add everything to prepend
    //         prepend = set_string;
    //     }
    //     else
    //     {
    //         //need to split the set_string into two parts at the location of the first 'if'
    //         std::string part1 = set_string.substr(0, first_if_pos);
    //         std::string part2 = set_string.substr(first_if_pos);
    //         prepend = part1;
    //         append = part2;
    //     }
    // }
}

} // namespace hops
