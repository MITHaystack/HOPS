#ifndef MHO_VexBlockParser_HH__
#define MHO_VexBlockParser_HH__

/*
*@file: MHO_VexBlockParser.hh
*@class: MHO_VexBlockParser
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

class MHO_VexBlockParser
{
    public:

        MHO_VexBlockParser();
        virtual ~MHO_VexBlockParser();

        void SetFormatDirectory(std::string fdir){fFormatDirectory = fdir;}
        mho_json ParseBlockLines(std::string block_name, const std::vector< MHO_VexLine >* block_lines);

    private:

        mho_json ParseBlock();

        bool IsStartTag(const MHO_VexLine& line);
        bool IsStopTag(const MHO_VexLine& line);
        bool IsReferenceTag(const MHO_VexLine& line);

        bool ProcessStartTag(const MHO_VexLine& line, 
                             std::stack< std::string >& path,
                             std::stack< mho_json* >& file_node,
                             std::stack< mho_json >& format_node);

        bool ProcessStopTag(const MHO_VexLine& line, 
                              std::stack< std::string >& path,
                              std::stack< mho_json* >& file_node,
                              std::stack< mho_json >& format_node);

        bool ProcessLine(const MHO_VexLine& line, 
                         std::stack< std::string >& path,
                         mho_json* file_node,
                         mho_json& format_node);

        bool ProcessReference(const MHO_VexLine& line, 
                         std::stack< std::string >& path,
                         mho_json* file_node,
                         mho_json& format_node);


        mho_json ProcessTokens(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens);


        mho_json ProcessInt(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens);
        mho_json ProcessListInt(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens);
        mho_json ProcessListString(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens);
        mho_json ProcessReal(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens);
        mho_json ProcessListReal(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens);
        mho_json ProcessCompound(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens);

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
        std::string fChanDefTag;
        std::string fIFDefTag;
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
            vex_keyword_type,
            vex_reference_type,
            vex_unknown_type
            //TODO FIXME -- add type for RA and Dec, to handle special treatment of "
        };
    
        vex_element_type DetermineType(std::string etype);
        bool ContainsWhitespace(std::string value);

        

};


}


#endif /* end of include guard: MHO_VexBlockParser */