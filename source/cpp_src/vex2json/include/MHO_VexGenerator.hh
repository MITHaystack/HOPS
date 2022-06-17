#ifndef MHO_VexGenerator_HH__
#define MHO_VexGenerator_HH__

/*
*@file: MHO_VexGenerator.hh
*@class: MHO_VexGenerator
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


namespace hops 
{

class MHO_VexGenerator
{
    public:
        MHO_VexGenerator();
        virtual ~MHO_VexGenerator();

        void SetVexVersion(std::string version);
        void SetVexVersion(const char* version);

        void SetFilename(std::string filename);
        void GenerateVex(mho_json& root);

    private:

        std::string fFilename;

        bool fBlockFormatLoaded;
        mho_json fBlockFormat;
        void LoadBlockFormat(std::string block_name);
        std::string GetBlockFormatFileName(std::string block_name);

        void ConstructBlockLines(mho_json& root, std::string block_name, std::vector< std::string >& lines);
        void ConstructElementLines(mho_json& element, std::vector< std::string >& lines);

        //format definition 
        std::string GetFormatDirectory() const;
        std::string fFormatDirectory;
        std::string fVexVersion;
        std::vector< std::string > fBlockNames;

};

}//end namespace

#endif /* end of include guard: MHO_VexGenerator */