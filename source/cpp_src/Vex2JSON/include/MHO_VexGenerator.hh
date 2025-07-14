#ifndef MHO_VexGenerator_HH__
#define MHO_VexGenerator_HH__

#include <list>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_VexDefinitions.hh"
#include "MHO_VexElementLineGenerator.hh"

namespace hops
{

/*!
 *@file  MHO_VexGenerator.hh
 *@class  MHO_VexGenerator
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu Jun 16 12:01:17 2022 -0400
 *@brief
 */

/**
 * @brief Class MHO_VexGenerator
 */
class MHO_VexGenerator
{
    public:
        MHO_VexGenerator();
        virtual ~MHO_VexGenerator();

        /**
         * @brief Setter for filename
         * 
         * @param filename New filename to be set
         */
        void SetFilename(std::string filename);
        /**
         * @brief Generates Vex file content and writes it to a specified filename.
         * 
         * @param root Input mho_json object containing data for Vex generation.
         */
        void GenerateVex(mho_json& root);

        /**
         * @brief Setter for indent padding
         * 
         * @param indent_pad New indent padding string to be set.
         */
        void SetIndentPadding(std::string indent_pad) { fIndentPad = indent_pad; }

    private:
        std::string fFilename;

        //format definition
        std::string fSpace;
        MHO_VexDefinitions fVexDef;
        std::string fFormatDirectory;
        std::string fVexRevisionFlag;
        std::string fVexVersion;
        std::vector< std::string > fBlockNames;
        /**
         * @brief Setter for vex version
         * 
         * @param version New vex version as string
         */
        void SetVexVersion(std::string version);

        /**
         * @brief Setter for vex version
         * 
         * @param version New version string to set
         */
        void SetVexVersion(const char* version) { SetVexVersion(std::string(version)); };

        //for constructing the vex lines
        std::string fPad;       //indentation level for lines
        std::string fIndentPad; //indentation "character"
        bool fBlockFormatLoaded;
        mho_json fBlockFormat;

        /**
         * @brief Loads block format from file using given block name.
         * 
         * @param block_name Name of the block to load format for.
         */
        void LoadBlockFormat(std::string block_name);

        /**
         * @brief Getter for block format file name
         * 
         * @param block_name Input block name string
         * @return Formatted block file name as string
         */
        std::string GetBlockFormatFileName(std::string block_name);
        void ConstructBlockLines(mho_json& root, std::string block_name, std::vector< std::string >& lines);
        void ConstructElementLines(mho_json& element, std::vector< std::string >& lines);
        void ConstructReferenceLines(mho_json& element, std::vector< std::string >& lines);
        MHO_VexElementLineGenerator fLineGen;

        //special conditions
        bool IsExcludedOvex(std::string block_name);
};

} // namespace hops

#endif /*! end of include guard: MHO_VexGenerator */
