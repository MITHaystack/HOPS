#ifndef MHO_VexParser_HH__
#define MHO_VexParser_HH__

#include <list>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_VexBlockParser.hh"
#include "MHO_VexDefinitions.hh"
#include "MHO_VexLine.hh"

namespace hops
{

/*!
 *@file  MHO_VexParser.hh
 *@class  MHO_VexParser
 *@author  J. Barrett - barrettj@mit.edu
 *@date Wed May 25 17:04:24 2022 -0400
 *@brief
 */

/**
 * @brief Class MHO_VexParser
 */
class MHO_VexParser
{
    public:
        MHO_VexParser();
        virtual ~MHO_VexParser();

        /**
         * @brief Setter for vex file
         * 
         * @param filename Vex file path as string
         */
        void SetVexFile(std::string filename);
        /**
         * @brief Setter for vex version
         * 
         * @param version New vex version as string
         */
        void SetVexVersion(std::string version);

        /**
         * @brief Setter for vex version
         * 
         * @param version New vex version as string
         */
        void SetVexVersion(const char* version) { SetVexVersion(std::string(version)); };

        /**
         * @brief Parses Vex file, processes blocks and returns JSON object with Vex revision flag.
         * 
         * @return mho_json object containing Vex revision flag
         */
        mho_json ParseVex();

    private:
        /**
         * @brief Determines and sets the version of the VEX file.
         */
        void DetermineFileVersion();
        /**
         * @brief Reads a VEX file into memory and stores each line as an MHO_VexLine object.
         */
        void ReadFile();
        /**
         * @brief Removes all comment lines from fLines vector.
         */
        void RemoveComments();
        /**
         * @brief Marks all literal sections in fLines by setting fIsLiteral to true.
         */
        void MarkLiterals();
        /**
         * @brief Indexes statements in fLines vector by assigning statement numbers.
         */
        void IndexStatements();
        /**
         * @brief Splits multiple ';' separated statements in a single line into separate lines.
         */
        void SplitStatements();
        /**
         * @brief Concatenates incomplete statements split across multiple lines into a single statement.
         */
        void JoinLines();
        /**
         * @brief Marks major parsable sections in VexParser lines.
         */
        void MarkBlocks();
        /**
         * @brief Collects all VexLines for a given block name.
         * 
         * @param block_name Name of the block to collect lines from.
         * @return Vector of MHO_VexLine objects representing collected lines.
         */
        std::vector< MHO_VexLine > CollectBlockLines(std::string block_name);
        /**
         * @brief Processes found blocks and adds them to the root json object.
         * 
         * @param root Reference to the root mho_json object where blocks will be added.
         */
        void ProcessBlocks(mho_json& root);

        /**
         * @brief Checks if a line is potentially the start of a block by looking for '$' and ensuring 'ref' is not present.
         * 
         * @param line Input string representing a line from the file.
         * @return Boolean indicating whether the line could be the start of a block.
         */
        bool IsPotentialBlockStart(std::string line);
        /**
         * @brief Checks if a given line starts with a specific block name and returns true if it's an exact match.
         * 
         * @param line Input string representing a single line from the file.
         * @param blk_name String containing the expected block name to search for.
         * @return Boolean indicating whether the line starts with the specified block name (true) or not (false).
         */
        bool IsBlockStart(std::string line, std::string blk_name);

        std::string fVexFileName;

        //token/delimiter definitions
        MHO_VexDefinitions fVexDef;

        std::string fVexRevisionFlag;
        std::string fVexDelim;
        std::string fWhitespace;
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

} // namespace hops

#endif /*! end of include guard: MHO_VexParser */
