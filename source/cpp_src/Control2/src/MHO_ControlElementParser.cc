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
        mho_json elem;
        elem["value"] = ParseTokens(element_name, fElementFormats[element_name], control_statement.fTokens); //otherwise parse any of the other supported elements
        elem["name"] = fElementFormats[element_name]["name"];
        elem["statement_type"] = fElementFormats[element_name]["statement_type"];
        return elem;
    }
    else
    {
        msg_error("control", "parser error, could not load format file for: "<<element_name<<" element, skipping."<<eom);
        mho_json empty;
        return empty;
    }
}

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






}//end namespace
