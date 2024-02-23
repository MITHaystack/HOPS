#ifndef MHO_VexBlockParser_HH__
#define MHO_VexBlockParser_HH__

/*!
*@file  MHO_VexBlockParser.hh
*@class  MHO_VexBlockParser
*@author  J. Barrett - barrettj@mit.edu 
*
*@date 
*@brief 
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
#include "MHO_VexTokenProcessor.hh"
#include "MHO_VexDefinitions.hh"

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

        void LoadBlockFormat(std::string block_name);
        std::string GetBlockFormatFileName(std::string block_name);

        mho_json ParseBlock();
        mho_json ParseGlobalBlock();

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


        mho_json ProcessTokens(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);
        mho_json ProcessCompound(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);

        bool MatchesType(const std::string& token, const std::string& type_name);

        bool fBlockFormatLoaded;
        mho_json fBlockFormat;
        std::string fBlockName;
        std::string fFormatDirectory;
        const std::vector< MHO_VexLine >* fBlockLines;
        std::size_t fCurrentLineNumber;

        std::string fStartTag;
        std::string fStopTag;

        MHO_Tokenizer fTokenizer;
        MHO_VexTokenProcessor fTokenProcessor;
};


}


#endif /*! end of include guard: MHO_VexBlockParser */