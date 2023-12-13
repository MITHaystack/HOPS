#include "MHO_ControlElementParser.hh"

namespace hops
{

MHO_ControlElementParser::MHO_ControlElementParser()
{
    fElementFormats = MHO_ControlDefinitions::GetControlFormat();
};

MHO_ControlElementParser::~MHO_ControlElementParser(){};

mho_json
MHO_ControlElementParser::ParseControlStatement(const MHO_ControlStatement& control_statement)
{
    //retrieve the element format
    std::string element_name = control_statement.fKeyword;
    bool format_loaded= false;

    //find the element format
    auto formatIt = fElementFormats.find(element_name);
    if(formatIt != fElementFormats.end() ){format_loaded= true;}
    mho_json elem;
    elem["name"] = element_name;
    elem["statement_type"] = "unknown";

    if(format_loaded)
    {
        std::string statement_type = fElementFormats[element_name]["statement_type"];

        if( statement_type != "unknown" && statement_type.size() != 0)
        {
            elem["statement_type"] = statement_type;
            elem["value"] = ParseTokens(element_name, fElementFormats[element_name], control_statement.fTokens); //otherwise parse any of the other supported elements
        }
        else
        {
            msg_warn("control", "control function for: "<<element_name<<" not yet implemented, skipping."<<eom);
        }
    }
    else
    {
        msg_warn("control", "parser error, could not load format file for: "<<element_name<<" element, skipping."<<eom);
    }

    return elem;
}

mho_json
MHO_ControlElementParser::ParseTokens(const std::string& element_name, mho_json& format, const std::vector< MHO_Token >& tokens)
{
    control_element_type etype = MHO_ControlDefinitions::DetermineControlType( format["type"].get<std::string>() );
    mho_json element_data;

    if(tokens.size() == 0)
    {
        msg_fatal("control", "missing tokens when parsing a statement for keyword " << element_name << "." << eom );
        std::exit(1);
    }

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
        case control_fixed_length_list_string_type:
            element_data = fTokenProcessor.ProcessFixedLengthListString(tokens);
        break;
        case control_bool_type:
            element_data = fTokenProcessor.ProcessBool(tokens[0]);
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
MHO_ControlElementParser::ProcessCompound(const std::string& element_name, mho_json& format, const std::vector< MHO_Token >& tokens)
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
            if(tokens[token_idx].fValue == nothing){token_idx++;} //empty value, skip this element
            else
            {
                std::string field_name = it->get<std::string>();
                mho_json next_format =  format["parameters"][field_name];
                std::string type_name = next_format["type"].get<std::string>();
                std::vector< MHO_Token > tmp_tokens;

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
