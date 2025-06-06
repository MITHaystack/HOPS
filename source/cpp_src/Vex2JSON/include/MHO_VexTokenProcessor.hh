#ifndef MHO_VexTokenProcessor_HH__
#define MHO_VexTokenProcessor_HH__

#include "MHO_DirectoryInterface.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_VexDefinitions.hh"
#include "MHO_VexLine.hh"

namespace hops
{

/*!
 *@file  MHO_VexTokenProcessor.hh
 *@class  MHO_VexTokenProcessor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Mon Jun 13 22:27:21 2022 -0400
 *@brief
 */

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

} // namespace hops

#endif /*! end of include guard: MHO_VexTokenProcessor */
