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

#include "MHO_ControlLine.hh"
// #include "MHO_ControlBlockParser.hh"
// #include "MHO_ControlDefinitions.hh"

namespace hops
{

class MHO_ControlFileParser
{
    public:
        MHO_ControlFileParser();
        virtual ~MHO_ControlFileParser();

        void SetControlFile(std::string filename);

        //mho_json
        void ParseControl();

    private:

        void ReadFile();
        void RemoveComments();
        void TokenizeLines();
        void MergeTokens();
        void FindKeywords();

        // void ProcessBlocks(mho_json& root);
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

        //tokenizer
        MHO_Tokenizer fTokenizer;

        std::vector< std::string > fFileTokens; //all tokens from file
        std::vector< std::size_t > fLineStartLocations; //index of each token which starts a line
        std::vector< std::size_t > fKeywordLocations; //index of each keyword token

        std::vector< std::vector< std::string > > fKeywordSections;


};

}//end namespace

#endif /* end of include guard: MHO_ControlFileParser */
