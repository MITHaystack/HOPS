#ifndef MHO_ControlTokenProcessor_HH__
#define MHO_ControlTokenProcessor_HH__

/*
*@file: MHO_ControlTokenProcessor.hh
*@class: MHO_ControlTokenProcessor
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_DirectoryInterface.hh"

#include "MHO_VexLine.hh"
#include "MHO_VexDefinitions.hh"


namespace hops
{

class MHO_ControlTokenProcessor
{
    public:
        MHO_ControlTokenProcessor();
        virtual ~MHO_ControlTokenProcessor();
    
        mho_json ProcessInt(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);
        mho_json ProcessListInt(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);
        mho_json ProcessListString(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);
        mho_json ProcessReal(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);
        mho_json ProcessListReal(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);

        bool ContainsWhitespace(std::string value);

    private:

        std::string fWhitespaceDelim;
        MHO_Tokenizer fTokenizer;
};

}

#endif /* end of include guard: MHO_ControlTokenProcessor */