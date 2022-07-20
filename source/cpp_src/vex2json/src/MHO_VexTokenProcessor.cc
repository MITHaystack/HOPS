#include "MHO_VexTokenProcessor.hh"

namespace hops 
{

MHO_VexTokenProcessor::MHO_VexTokenProcessor()
{
    fWhitespaceDelim = MHO_VexDefinitions::WhitespaceDelim();
};

MHO_VexTokenProcessor::~MHO_VexTokenProcessor(){};


mho_json 
MHO_VexTokenProcessor::ProcessInt(const std::string& element_name,
                                  mho_json& format, 
                                  std::vector< std::string >& tokens)
{
    mho_json element_data = std::atoi(tokens[0].c_str());
    return element_data;
}

mho_json 
MHO_VexTokenProcessor::ProcessListInt(const std::string& element_name, 
                                      mho_json& format, 
                                      std::vector< std::string >& tokens)
{
    mho_json element_data;
    std::vector< int > values; 
    for(std::size_t i=0; i<tokens.size(); i++)
    {
        values.push_back( std::atoi(tokens[i].c_str() ) );
    }
    element_data = values;
    return element_data;
}

mho_json 
MHO_VexTokenProcessor::ProcessListString(const std::string& element_name, 
                                         mho_json& format, 
                                         std::vector< std::string >& tokens)
{
    mho_json element_data;
    std::vector< std::string > values; 
    for(std::size_t i=0; i<tokens.size(); i++)
    {
        values.push_back( tokens[i] );
    }
    element_data = values;
    return element_data;
}

mho_json 
MHO_VexTokenProcessor::ProcessReal(const std::string& element_name,
                                   mho_json& format,
                                   std::vector< std::string >& tokens)
{
    mho_json element_data;

    if( ContainsWhitespace(tokens[0]) )  //if the value has units (we need to parse them out)
    {
        std::vector< std::string > tmp_tok;
        fTokenizer.SetString(&(tokens[0]));
        fTokenizer.SetDelimiter(fWhitespaceDelim);
        fTokenizer.SetUseMulticharacterDelimiterFalse();
        fTokenizer.SetIncludeEmptyTokensFalse();
        fTokenizer.GetTokens(&tmp_tok);
        if(tmp_tok.size() == 1)
        {
            element_data["value"] = std::atof(tmp_tok[0].c_str());
        }
        else if(tmp_tok.size() == 2)
        {
            element_data["value"] = std::atof(tmp_tok[0].c_str());
            element_data["units"] = tmp_tok[1];
        }
        else 
        {
            msg_error("vex", "could not parse parameter: "<<element_name<<", as (numerical) value from: <"<<tokens[0]<<">."<<eom);
        }
    }
    else 
    {
        element_data["value"] = std::atof(tokens[0].c_str());
    }
    return element_data;
}

mho_json 
MHO_VexTokenProcessor::ProcessListReal(const std::string& element_name,
                                       mho_json& format, 
                                       std::vector< std::string >& tokens)
{
    mho_json element_data;
    std::vector< double > values; 
    for(std::size_t i=0; i<tokens.size(); i++)
    {
        if( ContainsWhitespace(tokens[i]) )  //if the value has units (we need to parse them out)
        {
            std::vector< std::string > tmp_tok;
            fTokenizer.SetString(&(tokens[i]));
            fTokenizer.SetDelimiter(fWhitespaceDelim);
            fTokenizer.SetUseMulticharacterDelimiterFalse();
            fTokenizer.SetIncludeEmptyTokensFalse();
            fTokenizer.GetTokens(&tmp_tok);
            if(tmp_tok.size() == 1)
            {
                values.push_back( std::atof(tokens[i].c_str() ) );
            }
            else if(tmp_tok.size() == 2)
            {
                values.push_back( std::atof(tokens[i].c_str() ) );
                element_data["units"] = tmp_tok[1];
            }
            else 
            {
                msg_error("vex", "could not parse parameter: "<<element_name<<", as (numerical) value from: <"<<tokens[i]<<">."<<eom);
            }
        }
        else 
        {
            values.push_back( std::atof(tokens[i].c_str() ) );
        }
    }
    element_data["values"] = values;
    return element_data;
}

bool 
MHO_VexTokenProcessor::ContainsWhitespace(std::string value)
{
    auto start = value.find_first_of(fWhitespaceDelim);
    if(start == std::string::npos){return false;}
    return true;
}


}