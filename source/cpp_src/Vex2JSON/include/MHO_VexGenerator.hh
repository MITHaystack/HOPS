#ifndef MHO_VexGenerator_HH__
#define MHO_VexGenerator_HH__

#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <set>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_VexElementLineGenerator.hh"
#include "MHO_VexDefinitions.hh"

namespace hops
{

/*!
*@file  MHO_VexGenerator.hh
*@class  MHO_VexGenerator
*@author  J. Barrett - barrettj@mit.edu
*@date Thu Jun 16 12:01:17 2022 -0400
*@brief
*/


class MHO_VexGenerator
{
    public:
        MHO_VexGenerator();
        virtual ~MHO_VexGenerator();

        void SetFilename(std::string filename);
        void GenerateVex(mho_json& root);

        void SetIndentPadding(std::string indent_pad){fIndentPad = indent_pad;}

    private:

        std::string fFilename;

        //format definition
        std::string fSpace;
        MHO_VexDefinitions fVexDef;
        std::string fFormatDirectory;
        std::string fVexRevisionFlag;
        std::string fVexVersion;
        std::vector< std::string > fBlockNames;
        void SetVexVersion(std::string version);
        void SetVexVersion(const char* version){ SetVexVersion( std::string(version) );};

        //for constructing the vex lines
        std::string fPad; //indentation level for lines
        std::string fIndentPad; //indentation "character"
        bool fBlockFormatLoaded;
        mho_json fBlockFormat;
        void LoadBlockFormat(std::string block_name);
        std::string GetBlockFormatFileName(std::string block_name);
        void ConstructBlockLines(mho_json& root, std::string block_name, std::vector< std::string >& lines);
        void ConstructElementLines(mho_json& element, std::vector< std::string >& lines);
        void ConstructReferenceLines(mho_json& element, std::vector< std::string >& lines);
        MHO_VexElementLineGenerator fLineGen;

        //special conditions
        bool IsExcludedOvex(std::string block_name);

};

}//end namespace

#endif /*! end of include guard: MHO_VexGenerator */
