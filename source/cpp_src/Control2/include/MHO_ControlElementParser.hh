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

        void SetFormatDirectory(std::string fdir){fFormatDirectory = fdir;}
        void LoadElementFormats();
        mho_json ParseControlStatement(const MHO_ControlStatement& control_statement);

    private:

        std::string GetElementFormatFileName(std::string element_name);

        mho_json ParseElement();

        mho_json ParseTokens(const std::string& element_name, mho_json& format, const std::vector< std::string >& tokens);
        mho_json ProcessCompound(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);

        bool fElementFormatLoaded;
        mho_json fElementFormats;
        std::string fFormatDirectory;
        std::vector< std::string > fKeywordNames;

        MHO_Tokenizer fTokenizer;
        MHO_ControlTokenProcessor fTokenProcessor;
};


}


#endif /* end of include guard: MHO_ControlElementParser */
