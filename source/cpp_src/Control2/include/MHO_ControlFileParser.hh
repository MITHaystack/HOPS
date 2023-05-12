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
        void IndexStatements();
        void SplitStatements();
        void JoinLines();
        // void MarkBlocks();
        // std::vector< MHO_ControlLine > CollectBlockLines(std::string block_name);
        // void ProcessBlocks(mho_json& root);

        // bool IsPotentialBlockStart(std::string line);
        // bool IsBlockStart(std::string line, std::string blk_name);

        std::string fControlFileName;

        //token/delimiter definitions
        //MHO_ControlDefinitions fControlDef;

        std::string fControlRevisionFlag;
        std::string fControlDelim;
        std::string fWhitespace;
        std::string fBlockStartFlag;
        std::string fStatementEndFlag;
        std::string fRefFlag;

        //workspace
        std::string fLine; //the line from the input file 
        std::list< MHO_ControlLine > fLines;
        using line_itr = std::list< MHO_ControlLine >::iterator;
        std::set< std::string > fFoundBlocks; 
        std::map< std::string, line_itr > fBlockStartLines;
        std::map< std::string, line_itr > fBlockStopLines;

        //format definition 
        std::string fFormatDirectory;
        std::string fControlVersion;
        std::vector< std::string > fBlockNames;
        mho_json fBlockNamesJSON;

        //block parser 
        //MHO_ControlBlockParser fBlockParser;
        
        //tokenizer 
        MHO_Tokenizer fTokenizer;

};

}//end namespace

#endif /* end of include guard: MHO_ControlFileParser */
