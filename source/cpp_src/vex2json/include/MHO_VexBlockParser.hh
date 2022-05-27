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
        void SetBlockLines(std::string block_name, const std::vector< MHO_VexLine >* block_lines);
        
        void ParseBlock();

    private:

        void LoadBlockFormat(std::string block_name);
        std::string GetBlockFormatFileName(std::string block_name);

        bool fBlockFormatLoaded;
        mho_json fBlockFormat;
        std::string fFormatDirectory;
        const std::vector< MHO_VexLine >* fBlockLines;
        
        std::string fStartTag;
        std::string fStopTag;

};


}


#endif /* end of include guard: MHO_VexBlockParser */