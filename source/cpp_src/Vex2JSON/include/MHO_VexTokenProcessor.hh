#ifndef MHO_VexTokenProcessor_HH__
#define MHO_VexTokenProcessor_HH__

/*!
*@file  MHO_VexTokenProcessor.hh
*@class  MHO_VexTokenProcessor
*@author  J. Barrett - barrettj@mit.edu 
*@date
*@brief
*/

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_DirectoryInterface.hh"

#include "MHO_VexLine.hh"
#include "MHO_VexDefinitions.hh"


namespace hops
{

class MHO_VexTokenProcessor
{
    public:
        MHO_VexTokenProcessor();
        virtual ~MHO_VexTokenProcessor();

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

#endif /*! end of include guard: MHO_VexTokenProcessor */
