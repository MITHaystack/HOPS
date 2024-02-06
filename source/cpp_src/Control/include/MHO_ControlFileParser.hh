#ifndef MHO_ControlFileParser_HH__
#define MHO_ControlFileParser_HH__

/*
*@file: MHO_ControlFileParser.hh
*@class: MHO_ControlFileParser
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <set>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_JSONHeaderWrapper.hh"

#include "MHO_ControlDefinitions.hh"
#include "MHO_ControlElementParser.hh"

namespace hops
{

class MHO_ControlFileParser
{
    public:
        MHO_ControlFileParser();
        virtual ~MHO_ControlFileParser();

        void PassSetString(std::string set_string){fSetString = set_string;};
        void SetControlFile(std::string filename);

        mho_json ParseControl();
        
        //all of the tokens that make it into the control flow
        std::string GetProcessedControlFileText() const {return fProcessedControlFileText;}
        
        //just the control file tokens without set-string additions
        //needed for backwards compatible type_222 records
        std::string GetLegacyProcessedControlFileText() const {return fLegacyProcessedControlFileText;}

    private:

        bool ReadFile();
        void RemoveComments();
        void FixSymbols();
        void TokenizeLines();
        void MergeTokens();
        void ExportTokens();
        void FindKeywords();
        void FormStatements();
        mho_json ConstructControlObjects();
        
        void SplitSetString(const std::string& set_string, std::string& prepend, std::string& append);

        void FindAndReplace(const std::string& find_str, const std::string& regex_str, const std::string& replace_str, std::string& text);

        std::string fSetString;
        std::string fControlFileName;

        //token/delimiter definitions
        std::string fWhitespace;
        std::string fCommentFlag;

        //workspace
        std::string fLine; //the line from the input file
        std::list< MHO_ControlLine > fLines;
        using line_itr = std::list< MHO_ControlLine >::iterator;

        //format definition
        std::string fFormatDirectory;
        std::string fControlVersion;
        std::vector< std::string > fKeywordNames;
        mho_json fKeywordNamesJSON;

        //token processing
        MHO_Tokenizer fTokenizer;
        std::vector< MHO_Token > fFileTokens; //all tokens from file and set string
        std::vector< std::size_t > fKeywordLocations; //index of each keyword token
        std::vector< MHO_ControlStatement > fStatements; //collection of tokens for each logical statement

        MHO_ControlElementParser fElementParser;

        //output (the parsed text of the control file + set_string with comments removed )
        std::string fProcessedControlFileText;
        std::string fLegacyProcessedControlFileText;
        std::vector< MHO_Token > fLegacyFileTokens; //only tokens from file
};

}//end namespace

#endif /* end of include guard: MHO_ControlFileParser */
