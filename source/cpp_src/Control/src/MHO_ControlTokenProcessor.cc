#include "MHO_ControlTokenProcessor.hh"

namespace hops
{

MHO_ControlTokenProcessor::MHO_ControlTokenProcessor(){};
MHO_ControlTokenProcessor::~MHO_ControlTokenProcessor(){};


mho_json
MHO_ControlTokenProcessor::ProcessInt(const MHO_Token& token)
{
    mho_json element_data;
    int value;
    bool ok = ConvertInteger(token, value);
    if(ok){ element_data = value;}
    return element_data;
}

mho_json
MHO_ControlTokenProcessor::ProcessString(const MHO_Token& token)
{
    mho_json element_data = token.fValue;
    return element_data;
}


mho_json
MHO_ControlTokenProcessor::ProcessReal(const MHO_Token& token)
{
    mho_json element_data;
    double value;
    bool ok = ConvertFloat(token, value);
    if(ok){ element_data = value;}
    return element_data;
}

mho_json
MHO_ControlTokenProcessor::ProcessListInt(const std::vector< MHO_Token >& tokens)
{
    mho_json element_data;
    std::vector< int > values;
    for(std::size_t i=0; i<tokens.size(); i++)
    {
        int value;
        bool ok = ConvertInteger(tokens[i], value);
        if(ok){ values.push_back(value); }
        else{ values.push_back( std::numeric_limits<int>::max() ); } //this should never happen
    }
    element_data = values;
    return element_data;
}

mho_json
MHO_ControlTokenProcessor::ProcessListString(const std::vector< MHO_Token >& tokens)
{
    mho_json element_data;
    std::vector< std::string > values;
    for(std::size_t i=0; i<tokens.size(); i++){values.push_back( tokens[i].fValue );}
    element_data = values;
    return element_data;
}


mho_json
MHO_ControlTokenProcessor::ProcessListReal(const std::vector< MHO_Token >& tokens)
{
    mho_json element_data;
    std::vector< double > values;
    for(std::size_t i=0; i<tokens.size(); i++)
    {
        double value;
        bool ok = ConvertFloat(tokens[i], value);
        if(ok){values.push_back(value);}
        else{values.push_back( std::numeric_limits<double>::quiet_NaN() ); } //this should never happen
    }
    element_data = values;
    return element_data;
}


bool
MHO_ControlTokenProcessor::ConvertFloat(const MHO_Token& token, double& val)
{
    char* pend;
    val = std::strtod(token.fValue.c_str(), &pend);
    bool ok = (*pend == '\0');
    if(!ok)
    {
        //TODO - Q: Should this be a fatal error?
        msg_fatal("control", "invalid token on line: " << token.fLineNumber << ", could not convert: "<< token.fValue << " to float. " << eom);
        std::exit(1);
    }
    return ok;
}


bool
MHO_ControlTokenProcessor::ConvertInteger(const MHO_Token& token, int& val)
{
    char* pend;
    long int lval = std::strtol(token.fValue.c_str(), &pend, 10);
    bool ok = (*pend == '\0');
    if(!ok)
    {
        //TODO - Q: Should this be a fatal error?
        msg_fatal("control", "invalid token on line: " << token.fLineNumber << ", could not convert: "<< token.fValue << " to integer. " << eom);
        std::exit(1);
    }
    else
    {
        if( lval < std::numeric_limits<int>::max() )
        {
            //narrow to regular int
            val = (int) lval;
        }
        else
        {
            //TODO - Q: Should this be a fatal error?
            msg_fatal("control", "could not convert: "<< token.fValue << " to integer, out of range." << eom);
            std::exit(1);
        }
    }
    return ok;
}


}
