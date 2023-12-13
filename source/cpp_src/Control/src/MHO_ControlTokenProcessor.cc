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
MHO_ControlTokenProcessor::ProcessFixedLengthListString(const std::vector< MHO_Token >& tokens)
{
    mho_json element_data;
    std::vector< std::string > values;
    
    //check that there is at least 1 token 
    int n_elem = tokens.size();
    int length = 0;
    if(tokens.size() > 1)
    {
        //first token is the remaining length of the list
        length = std::atoi( tokens[0].fValue.c_str() );
    }

    //check that the specified length equals the number of tokens
    if(length != n_elem - 1)
    {
        msg_fatal("control", "invalid token on line: " << tokens[0].fLineNumber << ", user specified length of "<<
        tokens[0].fValue << " is inconsistent with the number of tokens: "<< (n_elem - 1) << "." << eom);
        std::exit(1);
    }
    
    for(std::size_t i=1; i<tokens.size(); i++){values.push_back( tokens[i].fValue );}
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


mho_json
MHO_ControlTokenProcessor::ProcessBool(const MHO_Token& token)
{
    mho_json element_data;
    bool value;
    bool ok = ConvertBool(token, value);
    if(ok){element_data = value;}
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


bool
MHO_ControlTokenProcessor::ConvertBool(const MHO_Token& token, bool& val)
{
    bool ok = false;
    if(token.fValue == "false"){val = false; ok = true;}
    if(token.fValue == "true"){val = true; ok = true;}
    if(!ok)
    {
        //TODO - Q: Should this be a fatal error?
        msg_fatal("control", "could not convert: "<< token.fValue << " to boolean value, only 'true' and 'false' accepted." << eom);
        std::exit(1);
    }
    
    return ok;
}




}
