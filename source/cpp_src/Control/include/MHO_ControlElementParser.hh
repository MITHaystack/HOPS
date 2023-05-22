#ifndef MHO_ControlElementParser_HH__
#define MHO_ControlElementParser_HH__

/*
*@file: MHO_ControlElementParser.hh
*@class: MHO_ControlElementParser
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <stack>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_DirectoryInterface.hh"

#include "MHO_ControlDefinitions.hh"
#include "MHO_ControlTokenProcessor.hh"


namespace hops
{

class MHO_ControlElementParser
{
    public:

        MHO_ControlElementParser();
        virtual ~MHO_ControlElementParser();

        mho_json ParseControlStatement(const MHO_ControlStatement& control_statement);

    private:

        mho_json ParseTokens(const std::string& element_name, mho_json& format, const std::vector< MHO_Token >& tokens);
        mho_json ProcessCompound(const std::string& element_name, mho_json& format, const std::vector< MHO_Token >& tokens);

        mho_json fElementFormats;

        MHO_ControlTokenProcessor fTokenProcessor;
};


}


#endif /* end of include guard: MHO_ControlElementParser */
