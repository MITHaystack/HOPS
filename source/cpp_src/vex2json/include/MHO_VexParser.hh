#ifndef MHO_VexParser_HH__
#define MHO_VexParser_HH__

/*
*@file: MHO_VexParser.hh
*@class: MHO_VexParser
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

#include "MHO_VexLine.hh"
#include "MHO_VexBlockParser.hh"

namespace hops 
{

class MHO_VexParser
{
    public:
        MHO_VexParser();
        virtual ~MHO_VexParser();

        void SetVexFile(std::string filename);
        void SetVexVersion(std::string version);
        void SetVexVersion(const char* version);

        mho_json ParseVex();
        
    private:

        void DetermineFileVersion();
        void ReadFile();
        void RemoveComments();
        void MarkLiterals();
        void JoinLines();
        void MarkBlocks();
        std::vector< MHO_VexLine > CollectBlockLines(std::string block_name);
        void ProcessBlocks(mho_json& root);


        std::string GetFormatDirectory();

        bool IsPotentialBlockStart(std::string line);
        bool IsBlockStart(std::string line, std::string blk_name);

        std::string fVexFileName;

        //token/delimiter definitions
        std::string fVexRevisionFlag;
        std::string fVexDelim;
        std::string fWhitespace;
        std::string fCommentFlag;
        std::string fBlockStartFlag;
        std::string fStatementEndFlag;
        std::string fRefFlag;

        std::string fStartLiteralFlag;
        std::string fEndLiteralFlag;

        //workspace
        std::string fLine; //the line from the input vex file 
        std::list< MHO_VexLine > fLines;
        using line_itr = std::list< MHO_VexLine >::iterator;
        std::set< std::string > fFoundBlocks; 
        std::map< std::string, line_itr > fBlockStartLines;
        std::map< std::string, line_itr > fBlockStopLines;

        //format definition 
        std::string fFormatDirectory;
        std::string fVexVersion;
        std::vector< std::string > fBlockNames;

        //block parser 
        MHO_VexBlockParser fBlockParser;

};

}//end namespace

#endif /* end of include guard: MHO_VexParser */