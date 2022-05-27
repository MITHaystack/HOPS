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


namespace hops 
{

class MHO_VexBlockParser
{
    public:

        MHO_VexBlockParser();
        virtual ~MHO_VexBlockParser();

        void SetBlockFormat(mho_json block_def);
        void SetBlockLines(std::string block_name, const std::vector< std::string >* block_lines);
        
        void ParseBlock();

    private:

        mho_json fBlockFormat;
        const std::vector< std::string >* fBlockLines;

        

        std::string fStartTag;
        std::string fStopTag;

};


}


#endif /* end of include guard: MHO_VexBlockParser */