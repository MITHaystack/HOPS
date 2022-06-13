#ifndef MHO_VexHighLevelBlockParser_HH__
#define MHO_VexHighLevelBlockParser_HH__

/*
*@file: MHO_VexHighLevelBlockParser.hh
*@class: MHO_VexHighLevelBlockParser
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

#include "MHO_VexLine.hh"

namespace hops 
{

class MHO_VexHighLevelBlockParser
{
    public:

        MHO_VexHighLevelBlockParser();
        virtual ~MHO_VexHighLevelBlockParser();

        void SetFormatDirectory(std::string fdir){fFormatDirectory = fdir;}
        mho_json ParseBlockLines(std::string block_name, const std::vector< MHO_VexLine >* block_lines);

    private:

        mho_json ParseBlock();

        bool ContainsRefTag(const MHO_VexLine& line);

        bool ProcessLine(const MHO_VexLine& line, 
                         std::stack< std::string >& path,
                         mho_json* file_node,
                         mho_json& format_node);

        mho_json ProcessTokens(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens);
        mho_json ProcessReference(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens);


        void LoadBlockFormat(std::string block_name);
        std::string GetBlockFormatFileName(std::string block_name);

        bool MatchesType(const std::string& token, const std::string& type_name);

        bool fBlockFormatLoaded;
        mho_json fBlockFormat;
        std::string fBlockName;
        std::string fFormatDirectory;
        const std::vector< MHO_VexLine >* fBlockLines;
        
        std::string fStartTag;
        std::string fStopTag;
        std::string fRefTag;
        std::string fVexDelim;
        std::string fStartTagDelim;
        std::string fAssignmentDelim;
        std::string fWhitespaceDelim;

        MHO_Tokenizer fTokenizer;

        enum vex_element_type
        {
            vex_int_type,
            vex_list_int_type,
            vex_real_type,
            vex_string_type,
            vex_list_string_type,
            vex_epoch_type,
            vex_list_real_type,
            vex_compound_type,
            vex_list_compound_type,
            vex_link_type,
            vex_unknown_type
        };
    
        vex_element_type DetermineType(std::string etype);
        bool ContainsWhitespace(std::string value);

};


}


#endif /* end of include guard: MHO_VexHighLevelBlockParser */