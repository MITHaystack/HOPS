#include "MHO_ControlElementParser.hh"

#include <fstream>
#include <cctype>
#include <algorithm>
#include <stack>
#include <regex>

namespace hops
{

MHO_ControlElementParser::MHO_ControlElementParser()
{

};

MHO_ControlElementParser::~MHO_ControlElementParser(){};


void
MHO_ControlElementParser::LoadElementFormats()
{
    //read the keyword-names.json file
    fFormatDirectory = HOPS_CONTROL_FORMAT_DIR;
    fFormatDirectory += "/control/";

    std::string keyword_names_file = fFormatDirectory + "keyword-names.json";
    std::ifstream ifs;
    ifs.open( keyword_names_file.c_str(), std::ifstream::in );


    mho_json keywordNamesJSON;
    if(ifs.is_open())
    {
        keywordNamesJSON = mho_json::parse(ifs);
    }
    ifs.close();

    fKeywordNames = keywordNamesJSON["keyword_names"];

    for(auto keyIt = fKeywordNames.begin(); keyIt != fKeywordNames.end(); keyIt++ )
    {
        std::string key = *keyIt;
        std::string element_format_file = GetElementFormatFileName(key);
        std::string format_file = fFormatDirectory + element_format_file;

        //TODO should check that the file exists
        std::ifstream bf_ifs;
        bf_ifs.open( format_file.c_str(), std::ifstream::in );

        mho_json bformat;
        if(bf_ifs.is_open())
        {
            bformat = mho_json::parse(bf_ifs);
            fElementFormats[key] = bformat;
        }
        bf_ifs.close();
    }

    std::cout << fElementFormats.dump(2) << std::endl;


}

std::string
MHO_ControlElementParser::GetElementFormatFileName(std::string element_name)
{
    std::string file_name = element_name + ".json";
    return file_name;
}



mho_json 
MHO_ControlElementParser::ParseControlStatement(const MHO_ControlStatement& control_statement)
{
    //retrieve the element format
    std::string element_name = control_statement.fKeyword;
    fElementFormatLoaded = false;

    //find the element format 
    auto formatIt = fElementFormats.find(element_name);
    if(formatIt != fElementFormats.end() ){fElementFormatLoaded = true;}
    
    if(fElementFormatLoaded)
    {
        return ParseTokens(element_name, fElementFormats[element_name], control_statement.fTokens); //otherwise parse any of the other supported elements
    }
    else
    {
        msg_error("control", "parser error, could not load format file for: "<<element_name<<" element, skipping."<<eom);
        mho_json empty;
        return empty;
    }
}

// mho_json
// MHO_ControlElementParser::ParseElement()
// {
//     mho_json element_root;
// 
//     std::stack< std::string > path;
//     std::stack< mho_json* > file_node;
//     std::stack< mho_json > format_node;
// 
//     path.push(fElementName);
//     file_node.push( &element_root );
// 
//     if(fElementLines != nullptr)
//     {
//         for(auto it = ++(fElementLines->begin()); it != fElementLines->end(); it++)
//         {
//             if( !(it->fIsLiteral) ) //skip all literals
//             {
//                 bool success = false;
//                 if( IsStartTag(*it) )
//                 {
//                     success = ProcessStartTag(*it, path, file_node, format_node);
//                 }
//                 else if( IsStopTag(*it) )
//                 {
//                     success = ProcessStopTag(*it, path, file_node, format_node);
//                 }
//                 else if( IsReferenceTag(*it) && format_node.size() != 0)
//                 {
//                     success = ProcessReference(*it, path, file_node.top(), format_node.top() );
//                 }
//                 else if( format_node.size() != 0)
//                 {
//                     success = ProcessLine(*it, path, file_node.top(), format_node.top());
//                 }
//                 if(!success){msg_warn("vex", "failed to process line: "<< it->fLineNumber << eom);}
//             }
//         }
//     }
//     else
//     {
//         msg_error("vex", "failed to parse element, no lines to process."<< eom);
//     }
// 
//     return element_root;
// }

// bool
// MHO_ControlElementParser::ProcessStartTag(const MHO_VexLine& line,
//                      std::stack< std::string >& path,
//                      std::stack< mho_json* >& file_node,
//                      std::stack< mho_json >& format_node)
// {
//     fTokenizer.SetDelimiter( MHO_VexDefinitions::StartTagDelim() );
//     fTokenizer.SetUseMulticharacterDelimiterFalse();
//     fTokenizer.SetIncludeEmptyTokensFalse();
//     fTokenizer.SetPreserveQuotesFalse();
//
//     std::vector< std::string > tokens;
//     fTokenizer.SetString(&(line.fContents));
//     fTokenizer.GetTokens(&tokens);
//
//     if(tokens.size() < 2)
//     {
//         msg_error("vex", "error processing element start tag, too few tokens."<<eom);
//         return false;
//     }
//
//     //open a new element
//     msg_debug("vex", "opening a new element element with name: "<<tokens[1]<<eom);
//     path.push( tokens[1] );
//     file_node.push( new mho_json() );
//
//     if(fElementFormat.contains("parameters"))
//     {
//         format_node.push( fElementFormat["parameters"] );
//         return true;
//     }
//     else{return false;}
// }

// bool
// MHO_ControlElementParser::ProcessStopTag(const MHO_VexLine& line,
//                       std::stack< std::string >& path,
//                       std::stack< mho_json* >& file_node,
//                       std::stack< mho_json >& format_node)
// {
//     //close current element
//     mho_json* last_obj = file_node.top();
//     std::string last_obj_name = path.top();
//     file_node.pop();
//     path.pop();
//     format_node.pop();
//     //insert this object into output json structure
//     (*(file_node.top()))[last_obj_name] = *last_obj;
//     delete last_obj;
//     msg_debug("vex", "closed a element element with name: "<<last_obj_name<<eom);
//     return true;
// }
//
// bool
// MHO_ControlElementParser::ProcessLine(const MHO_VexLine& line,
//                  std::stack< std::string >& path,
//                  mho_json* obj_node,
//                  mho_json& format)
// {
//     fCurrentLineNumber = line.fLineNumber;
//     fTokenizer.SetDelimiter(MHO_VexDefinitions::AssignmentDelim());
//     fTokenizer.SetUseMulticharacterDelimiterFalse();
//     fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
//     fTokenizer.SetIncludeEmptyTokensFalse();
//
//     std::vector< std::string > tokens;
//     fTokenizer.SetString(&(line.fContents));
//     fTokenizer.GetTokens(&tokens);
//
//     //verify that n_tokens is 2 for '=' assignment statement
//     if(tokens.size() == 2)
//     {
//         std::string element_name = tokens[0];
//         //verify that the element name is present in the current format node
//         if(!(format.contains(element_name)))
//         {
//             msg_warn("vex", "could not locate element with name: "<<element_name<<" under "<<fElementName<< " element format."<<eom);
//             return false;
//         }
//         vex_element_type etype = MHO_VexDefinitions::DetermineType( format[element_name]["type"].get<std::string>() );
//
//         fTokenizer.SetPreserveQuotesTrue();
//         //One infuriating feature of vex is that single/double quotes are not
//         //only used to encapsulate strings which must preserved, but are also
//         //used to indicate minutes/seconds of arc in RA/DEC so for very specific
//         //lines (i.e. source coordinates) we need to set this to false
//         if(fElementName == "$SOURCE")
//         {
//             if(etype == vex_radec_type ){fTokenizer.SetPreserveQuotesFalse();}
//             if(element_name == "datum"){fTokenizer.SetPreserveQuotesFalse();}
//         }
//
//         //data exists to the right of '=', before the ';'
//         std::string data = tokens[1]; //everything between '=' and ";"
//         fTokenizer.SetDelimiter( MHO_VexDefinitions::ElementDelim() );
//         fTokenizer.SetIncludeEmptyTokensTrue();
//         fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
//         fTokenizer.SetString(&data);
//         fTokenizer.GetTokens(&tokens);
//
//         mho_json element = ProcessTokens(element_name, format[element_name], tokens);
//
//         //if we are processing a list of compound elements, insert them one at a time
//         if( etype == vex_list_compound_type)
//         {
//             (*obj_node)[element_name].push_back( element );
//         }
//         else
//         {
//             (*obj_node)[element_name] = element;
//         }
//         return true;
//     }
//     else
//     {
//         msg_error("vex", "expected assignment with 2 tokens, but could not determine how to parse "<<tokens.size()<<" tokens from: <"<<line.fContents<<">."<<eom);
//         return false;
//     }
//
// }
//
//
mho_json
MHO_ControlElementParser::ParseTokens(const std::string& element_name, mho_json& format, const std::vector< std::string >& tokens)
{
    control_element_type etype = DetermineControlType( format["type"].get<std::string>() );
    mho_json element_data;

    switch(etype)
    {
        case control_int_type:
            element_data = fTokenProcessor.ProcessInt(element_name, tokens[0]);
        break;
        case control_list_int_type:
            element_data = fTokenProcessor.ProcessListInt(element_name, tokens);
        break;
        case control_real_type:
            element_data = fTokenProcessor.ProcessReal(element_name, tokens[0]);
        break;
        case control_list_real_type:
            element_data = fTokenProcessor.ProcessListReal(element_name, tokens);
        break;
        case control_string_type:
            element_data = tokens[0];
        break;
        case control_list_string_type:
            element_data = fTokenProcessor.ProcessListString(element_name, tokens);
        break;
        case control_compound_type: //all compound types treated the same way
            //element_data = ProcessCompound(element_name, format, tokens);
        break;
        default:
        break;
    }
    return element_data;
}

mho_json
MHO_ControlElementParser::ProcessCompound(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens)
{
    mho_json element_data;
    return element_data;
}
// 
// 
//     mho_json fields = format["fields"];
//     std::size_t n_tokens = tokens.size();
//     std::size_t n_all_fields = fields.size();
// 
//     std::size_t token_idx = 0;
//     std::string hash = MHO_VexDefinitions::OptionalFlag();
//     std::string nothing = "";
//     for(auto it = fields.begin(); it != fields.end(); it++)
//     {
//         if(token_idx < tokens.size() )
//         {
//             if(tokens[token_idx] == nothing){token_idx++;} //empty value, skip this element
//             else
//             {
//                 std::string raw_field_name = it->get<std::string>();
//                 std::string field_name = std::regex_replace(raw_field_name,std::regex(hash),nothing);
//                 mho_json next_format =  format["parameters"][field_name];
//                 std::string type_name = next_format["type"].get<std::string>();
//                 std::vector< std::string > tmp_tokens;
// 
//                 if( type_name == "list_int" || type_name == "list_real" || type_name == "list_string")
//                 {
//                     //consume the rest of the tokens until the end
//                     for(std::size_t i = token_idx; i<tokens.size(); i++){tmp_tokens.push_back(tokens[i]);};
//                     element_data[field_name] = ProcessTokens(field_name, next_format, tmp_tokens);
//                     token_idx = tokens.size();
//                 }
//                 else if (element_name == "chan_def" && field_name == "channel_name" )
//                 {
//                     //Deal with the stupid vex 2.0 case where there is no "optional" "channel_name" field in
//                     //in the channel defintion but there is a list of "freq_state" elements following it.
//                     //Normally any valid text can be converted to a string, but in this case the only
//                     //way to tell whether the current token is the optional channel_name element or the leading integer
//                     //in the frequency sequence is to explicitly check that this token is/isn't an integer.
//                     //If someone decides to specify a channel_name which is an integer value (1, 3, etc), then
//                     //the vex standard leaves this case undefined -- but we will (mis)interpret it as the leading index
//                     //of the freq_state list.
//                     if( !MatchesType( tokens[token_idx], std::string("int") ) )
//                     {
//                         tmp_tokens.push_back(tokens[token_idx]);
//                         element_data[field_name] = ProcessTokens(field_name, next_format, tmp_tokens);
//                         token_idx++;
//                     }
//                     else
//                     {
//                         msg_warn("vex", "channel definition on line: "<<fCurrentLineNumber<<" is ambiguous, intepreting integer value as leading freq_state index, not channel_name." << eom);
//                     }
//                 }
//                 else if( MatchesType( tokens[token_idx], type_name ) )
//                 {
//                     tmp_tokens.push_back(tokens[token_idx]);
//                     element_data[field_name] = ProcessTokens(field_name, next_format, tmp_tokens);
//                     token_idx++;
//                 }
//                 else
//                 {
//                     //assume that if the current token does not match the current type,
//                     //then an optional element has been omitted in the vex file
//                     //so don't increment the token index, and see if we can process it as
//                     //the next expected field
//                     if( MHO_VexDefinitions::IsOptionalField(raw_field_name))
//                     {
//                         msg_debug("vex", "could not parse <"<<tokens[token_idx]<<"> as type: "<<type_name<<", assuming optional field: "<< field_name << " is skipped." << eom);
//                     }
//                     else
//                     {
//                         msg_warn("vex", "could not parse <"<<tokens[token_idx]<<"> as type: "<<type_name<<", for field: "<< field_name << ", skipping." << eom);
//                     }
//                 }
//             }
//         }
//     }
//     return element_data;
// }





//
// bool
// MHO_ControlElementParser::MatchesType(const std::string& token, const std::string& type_name)
// {
//     vex_element_type etype = MHO_VexDefinitions::DetermineType(type_name);
//     switch(etype)
//     {
//         case vex_int_type:
//             {
//                 std::string tmp = token; //needed for negative and explicitly positive integers
//                 if(token[0] == '-' || token[0] == '+'){tmp = token.substr(1);}
//                 if(tmp.find_first_not_of("0123456789") == std::string::npos)
//                 {
//                     return true;
//                 }
//                 return false;
//             }
//         break;
//         case vex_real_type:
//             {
//                 std::string tmp = token;
//                 std::vector< std::string > tmp_tok;
//                 if( fTokenProcessor.ContainsWhitespace(token) )  //if the value has units (we need to parse them out)
//                 {
//                     fTokenizer.SetString(&token);
//                     fTokenizer.SetDelimiter( MHO_VexDefinitions::WhitespaceDelim() );
//                     fTokenizer.SetUseMulticharacterDelimiterFalse();
//                     fTokenizer.SetIncludeEmptyTokensFalse();
//                     fTokenizer.GetTokens(&tmp_tok);
//                     if(tmp_tok.size() == 1 || tmp_tok.size() == 2){tmp = tmp_tok[0];}
//                 }
//                 if(tmp.find_first_not_of("0123456789.e+-") == std::string::npos)
//                 {
//                     return true;
//                 }
//                 return false;
//             }
//         break;
//         case vex_list_real_type:
//             return true;//check for float list with or without units
//         break;
//         case vex_string_type:
//             return true; //always convertable to a string
//         case vex_keyword_type:
//             return true; //always convertable to a string
//         break;
//         case vex_radec_type:
//             return true; //always convertable to a string
//         break;
//         case vex_epoch_type:
//             //any epoch we encounter must at the very least include a 'year'
//             //TODO FIXME -- should we make this more strict and check for day/hour etc?
//             if(token.find_first_of("y") == std::string::npos)
//             {
//                 return false;
//             }
//             return true;
//         break;
//         case vex_link_type:
//             if(token.find_first_of("&") == std::string::npos){return false;}
//             else{return true;}
//         break;
//         case vex_compound_type:
//             return true; //return true until we recurse to a simpler type
//         case vex_list_compound_type:
//             return true; //return true until we recurse to a simpler type
//         break;
//         default:
//         break;
//     }
//     return false;
// }




}//end namespace
