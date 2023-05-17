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
    // //read the keyword-names.json file
    // fFormatDirectory = HOPS_CONTROL_FORMAT_DIR;
    // fFormatDirectory += "/control/";
    // 
    // std::string keyword_names_file = fFormatDirectory + "keyword-names.json";
    // std::ifstream ifs;
    // ifs.open( keyword_names_file.c_str(), std::ifstream::in );
    // 
    // 
    // mho_json keywordNamesJSON;
    // if(ifs.is_open())
    // {
    //     keywordNamesJSON = mho_json::parse(ifs);
    // }
    // ifs.close();
    // 
    // fKeywordNames = keywordNamesJSON["keyword_names"];
    
    

    fFormatDirectory = MHO_ControlDefinitions::GetFormatDirectory();
    fKeywordNames = MHO_ControlDefinitions::GetKeywordNames();


    for(auto keyIt = fKeywordNames.begin(); keyIt != fKeywordNames.end(); keyIt++ )
    {
        std::string key = *keyIt;
        
        std::cout<<"block name = "<< key << std::endl;
        
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

    std::cout<<"element_name = "<< element_name <<std::endl;
    
    if(fElementFormatLoaded)
    {
        std::string statement_type = fElementFormats[element_name]["statement_type"];
        if( statement_type == "unknown" || statement_type.size() == 0)
        {
            msg_error("control", "control function for: "<<element_name<<" not yet implemented, skipping."<<eom);
            mho_json empty;
            return empty;
        }

        mho_json elem;
        elem["name"] = fElementFormats[element_name]["name"];
        elem["statement_type"] = fElementFormats[element_name]["statement_type"];
        elem["value"] = ParseTokens(element_name, fElementFormats[element_name], control_statement.fTokens); //otherwise parse any of the other supported elements
        return elem;
    }
    else
    {
        msg_warn("control", "parser error, could not load format file for: "<<element_name<<" element, skipping."<<eom);
        mho_json empty;
        return empty;
    }
}

mho_json
MHO_ControlElementParser::ParseTokens(const std::string& element_name, mho_json& format, const std::vector< std::string >& tokens)
{
    control_element_type etype = MHO_ControlDefinitions::DetermineControlType( format["type"].get<std::string>() );
    mho_json element_data;

    switch(etype)
    {
        case control_int_type:
            element_data = fTokenProcessor.ProcessInt(tokens[0]);
        break;
        case control_real_type:
            element_data = fTokenProcessor.ProcessReal(tokens[0]);
        break;
        case control_string_type:
            element_data =  fTokenProcessor.ProcessString(tokens[0]);
        break;
        case control_list_int_type:
            element_data = fTokenProcessor.ProcessListInt(tokens);
        break;
        case control_list_real_type:
            element_data = fTokenProcessor.ProcessListReal(tokens);
        break;
        case control_list_string_type:
            element_data = fTokenProcessor.ProcessListString(tokens);
        break;
        case control_compound_type: //all compound types treated the same way
            element_data = ProcessCompound(element_name, format, tokens);
        break;
        default:
        break;
    }
    return element_data;
}

mho_json
MHO_ControlElementParser::ProcessCompound(const std::string& element_name, mho_json& format, const std::vector< std::string >& tokens)
{
    mho_json element_data;

    mho_json fields = format["fields"];
    std::size_t n_tokens = tokens.size();

    std::size_t token_idx = 0;
    std::string nothing = "";
    for(auto it = fields.begin(); it != fields.end(); it++)
    {
        if(token_idx < tokens.size() )
        {
            if(tokens[token_idx] == nothing){token_idx++;} //empty value, skip this element
            else
            {
                std::string field_name = it->get<std::string>();
                mho_json next_format =  format["parameters"][field_name];
                std::string type_name = next_format["type"].get<std::string>();
                std::vector< std::string > tmp_tokens;

                if( type_name == "list_int" || type_name == "list_real" || type_name == "list_string")
                {
                    //consume the rest of the tokens until the end
                    for(std::size_t i = token_idx; i<tokens.size(); i++){tmp_tokens.push_back(tokens[i]);};
                    element_data[field_name] = ParseTokens(field_name, next_format, tmp_tokens);
                    token_idx = tokens.size();
                }
                else
                {
                    tmp_tokens.push_back(tokens[token_idx]);
                    element_data[field_name] = ParseTokens(field_name, next_format, tmp_tokens);
                    token_idx++;
                }
            }
        }
    }
    return element_data;
}






}//end namespace
