#include "MHO_ControlTokenProcessor.hh"

namespace hops
{

MHO_ControlTokenProcessor::MHO_ControlTokenProcessor(){};
MHO_ControlTokenProcessor::~MHO_ControlTokenProcessor(){};


mho_json
MHO_ControlTokenProcessor::ProcessInt(const std::string& element_name, const std::string& token)
{
    mho_json element_data = std::atoi(token.c_str());
    return element_data;
}

mho_json
MHO_ControlTokenProcessor::ProcessString(const std::string& element_name, const std::string& token)
{
    mho_json element_data = token;
    return element_data;
}


mho_json
MHO_ControlTokenProcessor::ProcessReal(const std::string& element_name, const std::string& token)
{
    mho_json element_data;
    element_data = std::atof(token.c_str());
    return element_data;
}

mho_json
MHO_ControlTokenProcessor::ProcessListInt(const std::string& element_name, const std::vector< std::string >& tokens)
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
MHO_ControlTokenProcessor::ProcessListString(const std::string& element_name, const std::vector< std::string >& tokens)
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
MHO_ControlTokenProcessor::ProcessListReal(const std::string& element_name, const std::vector< std::string >& tokens)
{
    mho_json element_data;
    std::vector< double > values;
    for(std::size_t i=0; i<tokens.size(); i++)
    {
        values.push_back( std::atof(tokens[i].c_str() ) );
    }
    element_data = values;
    return element_data;
}


}
