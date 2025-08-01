#include "MHO_VexBlockParser.hh"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stack>

namespace hops
{

MHO_VexBlockParser::MHO_VexBlockParser()
{
    fBlockLines = nullptr;
    //defaults, but stop/start tags vary by block
    fStartTag = "def";
    fStopTag = "enddef";
};

MHO_VexBlockParser::~MHO_VexBlockParser(){};

void MHO_VexBlockParser::LoadBlockFormat(std::string block_name)
{
    fBlockFormatLoaded = false;
    std::string block_format_file = GetBlockFormatFileName(block_name);
    std::string format_file = fFormatDirectory + block_format_file;

    std::ifstream bf_ifs;
    bf_ifs.open(format_file.c_str(), std::ifstream::in);

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
        fBlockName = block_name;
        fStartTag = fBlockFormat["start_tag"].get< std::string >();
        fStopTag = fBlockFormat["stop_tag"].get< std::string >();
    }
}

std::string MHO_VexBlockParser::GetBlockFormatFileName(std::string block_name)
{
    //remove '$', and convert to lower-case
    std::string file_name = block_name.substr(1);
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    file_name += ".json";
    return file_name;
}

mho_json MHO_VexBlockParser::ParseBlockLines(std::string block_name, const std::vector< MHO_VexLine >* block_lines)
{
    //retrieve the block format
    fBlockFormatLoaded = false;
    LoadBlockFormat(block_name);
    fBlockLines = block_lines;

    if(fBlockFormatLoaded)
    {
        if(block_name == "$GLOBAL")
        {
            return ParseGlobalBlock();
        } //global block is "special"
        else
        {
            if(fBlockFormat["block_type"].get< std::string >() == "unsupported")
            {
                std::string version = fBlockFormat["version"].get< std::string >();
                msg_info("vex", "block type: " << block_name << " is not supported in format: " << fFormatDirectory
                                               << ", skipping." << eom);
                mho_json empty;
                return empty;
            }
            else
            {
                return ParseBlock(); //otherwise parse any of the other supported blocks
            }
        }
    }
    else
    {
        msg_error("vex", "parser error, could not load format file for: " << block_name << " block, skipping." << eom);
        mho_json empty;
        return empty;
    }
}

mho_json MHO_VexBlockParser::ParseBlock()
{
    mho_json block_root;

    std::stack< std::string > path;
    std::stack< mho_json* > file_node;
    std::stack< mho_json > format_node;

    path.push(fBlockName);
    file_node.push(&block_root);

    if(fBlockLines != nullptr)
    {
        for(auto it = ++(fBlockLines->begin()); it != fBlockLines->end(); it++)
        {
            if(!(it->fIsLiteral)) //skip all literals
            {
                bool success = false;
                if(IsStartTag(*it))
                {
                    success = ProcessStartTag(*it, path, file_node, format_node);
                }
                else if(IsStopTag(*it))
                {
                    success = ProcessStopTag(*it, path, file_node, format_node);
                }
                else if(IsReferenceTag(*it) && format_node.size() != 0)
                {
                    success = ProcessReference(*it, path, file_node.top(), format_node.top());
                }
                else if(format_node.size() != 0)
                {
                    success = ProcessLine(*it, path, file_node.top(), format_node.top());
                }
                if(!success)
                {
                    msg_debug("vex", "failed to process line: " << it->fLineNumber << eom);
                }
            }
        }
    }
    else
    {
        msg_error("vex", "failed to parse block, no lines to process." << eom);
    }

    return block_root;
}

mho_json MHO_VexBlockParser::ParseGlobalBlock()
{
    mho_json block_root;

    std::stack< std::string > path;
    std::stack< mho_json* > file_node;
    std::stack< mho_json > format_node;

    if(fBlockFormat.contains("parameters"))
    {
        path.push(fBlockName);
        file_node.push(&block_root);
        format_node.push(fBlockFormat["parameters"]);
    }
    else
    {
        msg_error("vex", "failed to find parameters statement in $GLOBAL format block" << eom);
        return block_root;
    }

    if(fBlockLines != nullptr)
    {
        for(auto it = ++(fBlockLines->begin()); it != fBlockLines->end(); it++)
        {
            bool success = false;
            if(IsReferenceTag(*it))
            {
                success = ProcessReference(*it, path, file_node.top(), format_node.top());
            }
            if(!success)
            {
                msg_error("vex", "failed to process line: " << it->fLineNumber << eom);
            }
        }
    }
    else
    {
        msg_error("vex", "failed to parse block, no lines to process." << eom);
    }

    return block_root;
}

bool MHO_VexBlockParser::IsStartTag(const MHO_VexLine& line)
{
    std::vector< std::string > tokens;
    fTokenizer.SetDelimiter(MHO_VexDefinitions::StartTagDelim());
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetIncludeEmptyTokensFalse();
    fTokenizer.SetPreserveQuotesFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    fTokenizer.SetString(&(line.fContents));
    fTokenizer.GetTokens(&tokens);
    if(tokens.size() > 1)
    {
        if(tokens[0] == fStartTag)
        {
            return true;
        }
    }
    return false;
}

bool MHO_VexBlockParser::IsStopTag(const MHO_VexLine& line)
{
    if(line.fContents.find(fStopTag) != std::string::npos)
    {
        return true;
    }
    return false;
}

bool MHO_VexBlockParser::IsReferenceTag(const MHO_VexLine& line)
{
    if(line.fContents.find(MHO_VexDefinitions::RefTag()) != std::string::npos)
    {
        std::vector< std::string > tokens;
        fTokenizer.SetDelimiter(MHO_VexDefinitions::StartTagDelim());
        fTokenizer.SetUseMulticharacterDelimiterFalse();
        fTokenizer.SetIncludeEmptyTokensFalse();
        fTokenizer.SetPreserveQuotesFalse();
        fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        fTokenizer.SetString(&(line.fContents));
        fTokenizer.GetTokens(&tokens);
        if(tokens.size() > 1)
        {
            if(tokens[0] == MHO_VexDefinitions::RefTag())
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

bool MHO_VexBlockParser::ProcessStartTag(const MHO_VexLine& line, std::stack< std::string >& path,
                                         std::stack< mho_json* >& file_node, std::stack< mho_json >& format_node)
{
    fTokenizer.SetDelimiter(MHO_VexDefinitions::StartTagDelim());
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetIncludeEmptyTokensFalse();
    fTokenizer.SetPreserveQuotesFalse();

    std::vector< std::string > tokens;
    fTokenizer.SetString(&(line.fContents));
    fTokenizer.GetTokens(&tokens);

    if(tokens.size() < 2)
    {
        msg_error("vex", "error processing block start tag, too few tokens." << eom);
        return false;
    }

    //open a new block
    //msg_debug("vex", "opening a new block element with name: "<<tokens[1]<<eom);
    path.push(tokens[1]);
    file_node.push(new mho_json());

    if(fBlockFormat.contains("parameters"))
    {
        format_node.push(fBlockFormat["parameters"]);
        return true;
    }
    else
    {
        return false;
    }
}

bool MHO_VexBlockParser::ProcessStopTag(const MHO_VexLine& line, std::stack< std::string >& path,
                                        std::stack< mho_json* >& file_node, std::stack< mho_json >& format_node)
{
    //close current block
    mho_json* last_obj = file_node.top();
    std::string last_obj_name = path.top();
    file_node.pop();
    path.pop();
    format_node.pop();
    //insert this object into output json structure
    (*(file_node.top()))[last_obj_name] = *last_obj;
    delete last_obj;
    //msg_debug("vex", "closed a block element with name: "<<last_obj_name<<eom);
    return true;
}

bool MHO_VexBlockParser::ProcessLine(const MHO_VexLine& line, std::stack< std::string >& path, mho_json* obj_node,
                                     mho_json& format)
{
    fCurrentLineNumber = line.fLineNumber;
    fTokenizer.SetDelimiter(MHO_VexDefinitions::AssignmentDelim());
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    fTokenizer.SetIncludeEmptyTokensFalse();

    std::vector< std::string > tokens;
    fTokenizer.SetString(&(line.fContents));
    fTokenizer.GetTokens(&tokens);

    //verify that n_tokens is 2 for '=' assignment statement
    if(tokens.size() == 2)
    {
        std::string element_name = tokens[0];
        //verify that the element name is present in the current format node
        if(!(format.contains(element_name)))
        {
            if(element_name != "exper_num")
            {
                msg_info("vex", "could not locate element with name: " << element_name << " under " << fBlockName
                                                                       << " block format." << eom);
            }
            return false;
        }
        vex_element_type etype = MHO_VexDefinitions::DetermineType(format[element_name]["type"].get< std::string >());

        fTokenizer.SetPreserveQuotesTrue();
        //One infuriating feature of vex is that single/double quotes are not
        //only used to encapsulate strings which must preserved, but are also
        //used to indicate minutes/seconds of arc in RA/DEC so for very specific
        //lines (i.e. source coordinates) we need to set this to false
        if(fBlockName == "$SOURCE")
        {
            if(etype == vex_radec_type)
            {
                fTokenizer.SetPreserveQuotesFalse();
            }
            if(element_name == "datum")
            {
                fTokenizer.SetPreserveQuotesFalse();
            }
        }

        //data exists to the right of '=', before the ';'
        std::string data = tokens[1]; //everything between '=' and ";"
        fTokenizer.SetDelimiter(MHO_VexDefinitions::ElementDelim());
        fTokenizer.SetIncludeEmptyTokensTrue();
        fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        fTokenizer.SetString(&data);
        fTokenizer.GetTokens(&tokens);

        mho_json element = ProcessTokens(element_name, format[element_name], tokens);

        //if we are processing a list of compound elements, insert them one at a time
        if(etype == vex_list_compound_type)
        {
            (*obj_node)[element_name].push_back(element);
        }
        else
        {
            (*obj_node)[element_name] = element;
        }
        return true;
    }
    else
    {
        msg_error("vex", "expected assignment with 2 tokens, but could not determine how to parse "
                             << tokens.size() << " tokens from: <" << line.fContents << ">." << eom);
        return false;
    }
}

bool MHO_VexBlockParser::ProcessReference(const MHO_VexLine& line, std::stack< std::string >& path, mho_json* file_node,
                                          mho_json& format_node)
{
    fTokenizer.SetDelimiter(MHO_VexDefinitions::AssignmentDelim());
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    fTokenizer.SetIncludeEmptyTokensFalse();

    std::vector< std::string > tokens;
    fTokenizer.SetString(&(line.fContents));
    fTokenizer.GetTokens(&tokens);
    std::string element_block_name = "";
    mho_json element;

    //verify that n_tokens is 2 for '=' assignment statement
    if(tokens.size() == 2)
    {
        //split first token on whitespace, verify 'ref' is present and extract the block name
        std::vector< std::string > ref_tokens;
        fTokenizer.SetDelimiter(MHO_VexDefinitions::WhitespaceDelim());
        fTokenizer.SetIncludeEmptyTokensFalse();
        fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        fTokenizer.SetString(&(tokens[0]));
        fTokenizer.GetTokens(&ref_tokens);
        if(ref_tokens.size() == 2)
        {
            if(ref_tokens[0] == MHO_VexDefinitions::RefTag())
            {
                element_block_name = ref_tokens[1];
                if(!(format_node.contains(element_block_name)))
                {
                    msg_info("vex", "could not locate element with name: " << element_block_name << " under " << fBlockName
                                                                           << " block format." << eom);
                    return false;
                }

                //split second token on ':' for parsable elements
                std::vector< std::string > kq_tokens;
                fTokenizer.SetDelimiter(MHO_VexDefinitions::ElementDelim());
                fTokenizer.SetIncludeEmptyTokensFalse();
                fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
                fTokenizer.SetString(&(tokens[1]));
                fTokenizer.GetTokens(&kq_tokens);
                //add keyword
                if(kq_tokens.size() >= 1)
                {
                    element["keyword"] = kq_tokens[0];
                }
                //the rest are qualifiers
                if(kq_tokens.size() >= 2)
                {
                    for(std::size_t i = 1; i < kq_tokens.size(); i++)
                    {
                        element["qualifiers"].push_back(kq_tokens[i]);
                    }
                }
            }
            else
            {
                msg_error("vex", "could not process a reference from: " << ref_tokens[0] << eom);
                return false;
            }
        }

        (*file_node)[element_block_name].push_back(element);
        return true;
    }
    else
    {
        msg_error("vex", "expected assignment with 2 tokens, but could not determine how to parse "
                             << tokens.size() << " tokens from: <" << line.fContents << ">." << eom);
        return false;
    }
}

mho_json MHO_VexBlockParser::ProcessTokens(const std::string& element_name, mho_json& format,
                                           std::vector< std::string >& tokens)
{
    vex_element_type etype = MHO_VexDefinitions::DetermineType(format["type"].get< std::string >());
    mho_json element_data;

    switch(etype)
    {
        case vex_int_type:
            element_data = fTokenProcessor.ProcessInt(element_name, format, tokens);
            break;
        case vex_list_int_type:
            element_data = fTokenProcessor.ProcessListInt(element_name, format, tokens);
            break;
        case vex_real_type:
            element_data = fTokenProcessor.ProcessReal(element_name, format, tokens);
            break;
        case vex_list_real_type:
            element_data = fTokenProcessor.ProcessListReal(element_name, format, tokens);
            break;
        case vex_keyword_type:
            element_data = tokens[0];
            break;
        case vex_string_type:
            element_data = tokens[0];
            break;
        case vex_list_string_type:
            element_data = fTokenProcessor.ProcessListString(element_name, format, tokens);
            break;
        case vex_epoch_type:
            element_data = tokens[0];
            break;
        case vex_radec_type:
            element_data = tokens[0]; //leave source coordinates as strings, can convert later
            break;
        case vex_link_type:
            element_data = tokens[0];
            break;
        case vex_compound_type: //all compound types treated the same way
            element_data = ProcessCompound(element_name, format, tokens);
        case vex_list_compound_type:
            element_data = ProcessCompound(element_name, format, tokens);
            break;
        default:
            break;
    }
    return element_data;
}

mho_json MHO_VexBlockParser::ProcessCompound(const std::string& element_name, mho_json& format,
                                             std::vector< std::string >& tokens)
{
    mho_json element_data;
    mho_json fields = format["fields"];
    std::size_t n_tokens = tokens.size();
    std::size_t n_all_fields = fields.size();

    std::size_t token_idx = 0;
    std::string bang = MHO_VexDefinitions::OptionalFlag();
    std::string nothing = "";
    for(auto it = fields.begin(); it != fields.end(); it++)
    {
        if(token_idx < tokens.size())
        {
            if(tokens[token_idx] == nothing)
            {
                token_idx++;
            } //empty value, skip this element
            else
            {
                std::string raw_field_name = it->get< std::string >();
                std::string field_name = string_pattern_replace(raw_field_name, bang, nothing);
                //std::regex_replace(raw_field_name, std::regex(bang), nothing);
                mho_json next_format = format["parameters"][field_name];
                std::string type_name = next_format["type"].get< std::string >();
                std::vector< std::string > tmp_tokens;

                if(type_name == "list_int" || type_name == "list_real" || type_name == "list_string")
                {
                    //consume the rest of the tokens until the end
                    for(std::size_t i = token_idx; i < tokens.size(); i++)
                    {
                        tmp_tokens.push_back(tokens[i]);
                    };
                    element_data[field_name] = ProcessTokens(field_name, next_format, tmp_tokens);
                    token_idx = tokens.size();
                }
                else if(element_name == "chan_def" && field_name == "channel_name")
                {
                    //Deal with the stupid vex 2.0 case where there is no "optional" "channel_name" field in
                    //in the channel defintion but there is a list of "freq_state" elements following it.
                    //Normally any valid text can be converted to a string, but in this case the only
                    //way to tell whether the current token is the optional channel_name element or the leading integer
                    //in the frequency sequence is to explicitly check that this token is/isn't an integer.
                    //If someone decides to specify a channel_name which is an integer value (1, 3, etc), then
                    //the vex standard leaves this case undefined -- but we will (mis)interpret it as the leading index
                    //of the freq_state list.
                    if(!MatchesType(tokens[token_idx], std::string("int")))
                    {
                        tmp_tokens.push_back(tokens[token_idx]);
                        element_data[field_name] = ProcessTokens(field_name, next_format, tmp_tokens);
                        token_idx++;
                    }
                    else
                    {
                        msg_warn(
                            "vex",
                            "channel definition on line: "
                                << fCurrentLineNumber
                                << " is ambiguous, intepreting integer value as leading freq_state index, not channel_name."
                                << eom);
                    }
                }
                else if(MatchesType(tokens[token_idx], type_name))
                {
                    tmp_tokens.push_back(tokens[token_idx]);
                    element_data[field_name] = ProcessTokens(field_name, next_format, tmp_tokens);
                    token_idx++;
                }
                else
                {
                    //assume that if the current token does not match the current type,
                    //then an optional element has been omitted in the vex file
                    //so don't increment the token index, and see if we can process it as
                    //the next expected field
                    if(MHO_VexDefinitions::IsOptionalField(raw_field_name))
                    {
                        msg_debug("vex", "could not parse <" << tokens[token_idx] << "> as type: " << type_name
                                                             << ", assuming optional field: " << field_name << " is skipped."
                                                             << eom);
                    }
                    else
                    {
                        msg_warn("vex", "could not parse <" << tokens[token_idx] << "> as type: " << type_name
                                                            << ", for field: " << field_name << ", skipping." << eom);
                    }
                }
            }
        }
    }
    return element_data;
}

bool MHO_VexBlockParser::MatchesType(const std::string& token, const std::string& type_name)
{
    vex_element_type etype = MHO_VexDefinitions::DetermineType(type_name);
    switch(etype)
    {
        case vex_int_type:
            {
                std::string tmp = token; //needed for negative and explicitly positive integers
                if(token[0] == '-' || token[0] == '+')
                {
                    tmp = token.substr(1);
                }
                if(tmp.find_first_not_of("0123456789") == std::string::npos)
                {
                    return true;
                }
                return false;
            }
            break;
        case vex_real_type:
            {
                std::string tmp = token;
                std::vector< std::string > tmp_tok;
                if(fTokenProcessor.ContainsWhitespace(token)) //if the value has units (we need to parse them out)
                {
                    fTokenizer.SetString(&token);
                    fTokenizer.SetDelimiter(MHO_VexDefinitions::WhitespaceDelim());
                    fTokenizer.SetUseMulticharacterDelimiterFalse();
                    fTokenizer.SetIncludeEmptyTokensFalse();
                    fTokenizer.GetTokens(&tmp_tok);
                    if(tmp_tok.size() == 1 || tmp_tok.size() == 2)
                    {
                        tmp = tmp_tok[0];
                    }
                }
                if(tmp.find_first_not_of("0123456789.e+-") == std::string::npos)
                {
                    return true;
                }
                return false;
            }
            break;
        case vex_list_real_type:
            return true; //check for float list with or without units
            break;
        case vex_string_type:
            return true; //always convertable to a string
        case vex_keyword_type:
            return true; //always convertable to a string
            break;
        case vex_radec_type:
            return true; //always convertable to a string
            break;
        case vex_epoch_type:
            //any epoch we encounter must at the very least include a 'year'
            //TODO FIXME -- should we make this more strict and check for day/hour etc?
            if(token.find_first_of("y") == std::string::npos)
            {
                return false;
            }
            return true;
            break;
        case vex_link_type:
            if(token.find_first_of("&") == std::string::npos)
            {
                return false;
            }
            else
            {
                return true;
            }
            break;
        case vex_compound_type:
            return true; //return true until we recurse to a simpler type
        case vex_list_compound_type:
            return true; //return true until we recurse to a simpler type
            break;
        default:
            break;
    }
    return false;
}

} // namespace hops
