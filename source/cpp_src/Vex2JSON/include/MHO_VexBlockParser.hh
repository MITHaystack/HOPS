#ifndef MHO_VexBlockParser_HH__
#define MHO_VexBlockParser_HH__

#include <list>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "MHO_DirectoryInterface.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_VexDefinitions.hh"
#include "MHO_VexLine.hh"
#include "MHO_VexTokenProcessor.hh"

namespace hops
{

/*!
 *@file  MHO_VexBlockParser.hh
 *@class  MHO_VexBlockParser
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu May 26 16:55:16 2022 -0400
 *@brief Parses information in a named vex block (e.g. $FREQ, $SOURCE, etc)
 */

/**
 * @brief Class MHO_VexBlockParser
 */
class MHO_VexBlockParser
{
    public:
        MHO_VexBlockParser();
        virtual ~MHO_VexBlockParser();

        /**
         * @brief Setter for format directory
         * 
         * @param fdir New format directory path
         */
        void SetFormatDirectory(std::string fdir) { fFormatDirectory = fdir; }

        /**
         * @brief Parses block lines and returns a JSON object based on block type.
         * 
         * @param block_name Name of the block to parse
         * @param block_lines Pointer to vector of MHO_VexLine objects representing the block
         * @return mho_json object containing parsed block data or empty if parsing fails
         */
        mho_json ParseBlockLines(std::string block_name, const std::vector< MHO_VexLine >* block_lines);

    private:
        /**
         * @brief Loads block format from file using given block name.
         * 
         * @param block_name Name of the block to load format for.
         */
        void LoadBlockFormat(std::string block_name);

        /**
         * @brief Getter for block format file name
         * 
         * @param block_name Input block name as string
         * @return Formatted block file name as string
         */
        std::string GetBlockFormatFileName(std::string block_name);

        /**
         * @brief Parses a block and returns its JSON representation.
         * 
         * @return The parsed block as an mho_json object.
         */
        mho_json ParseBlock();

        /**
         * @brief Parses global block and returns its JSON representation.
         * 
         * @return mho_json representing the parsed global block.
         */
        mho_json ParseGlobalBlock();

        /**
         * @brief Checks if a given line is a start tag matching the parser's expected start tag.
         * 
         * @param line Input line to check for start tag.
         * @return True if line starts with the expected start tag, false otherwise.
         */
        bool IsStartTag(const MHO_VexLine& line);

        /**
         * @brief Checks if a given line contains the stop tag.
         * 
         * @param line Input line to check for stop tag.
         * @return True if stop tag is found, false otherwise.
         */
        bool IsStopTag(const MHO_VexLine& line);

        /**
         * @brief Checks if a given line is a reference tag.
         * 
         * @param line Input MHO_VexLine to check for reference tag.
         * @return True if line contains reference tag, false otherwise.
         */
        bool IsReferenceTag(const MHO_VexLine& line);

        /**
         * @brief Processes start tag in VEX line and updates path, file_node, format_node stacks.
         * 
         * @param line Input VEX line to process
         * @param path Stack of strings representing path
         * @param file_node Stack of mho_json pointers for file nodes
         * @param format_node Stack of mho_json objects for format nodes
         * @return True if parameters are found in block format, false otherwise
         */
        bool ProcessStartTag(const MHO_VexLine& line, std::stack< std::string >& path, std::stack< mho_json* >& file_node,
                             std::stack< mho_json >& format_node);

        /**
         * @brief Closes current block and inserts it into output json structure.
         * 
         * @param line Input line for processing (not used in this function).
         * @param path Stack of strings representing path to current object.
         * @param file_node Stack of pointers to mho_json objects representing file nodes.
         * @param format_node Stack of mho_json objects representing format nodes.
         * @return True if block is successfully closed and inserted.
         */
        bool ProcessStopTag(const MHO_VexLine& line, std::stack< std::string >& path, std::stack< mho_json* >& file_node,
                            std::stack< mho_json >& format_node);

        /**
         * @brief Parses and processes a VEX line, updating path, file_node, and format_node accordingly.
         * 
         * @param line Input VEX line to process
         * @param path Reference stack of strings representing path
         * @param file_node Pointer to mho_json object for file node
         * @param format_node Reference to mho_json object for format node
         * @return Boolean indicating successful processing of the line
         */
        bool ProcessLine(const MHO_VexLine& line, std::stack< std::string >& path, mho_json* file_node, mho_json& format_node);

        /**
         * @brief Processes a reference line in VEX format and updates path, file_node, and format_node accordingly.
         * 
         * @param line Input VEX line to process
         * @param path Stack of strings representing path navigation
         * @param file_node mho_json pointer for file node manipulation
         * @param format_node Reference to mho_json object for format node updates
         * @return Boolean indicating successful processing of the reference line
         */
        bool ProcessReference(const MHO_VexLine& line, std::stack< std::string >& path, mho_json* file_node,
                              mho_json& format_node);

        /**
         * @brief Processes tokens based on element type and format.
         * 
         * @param element_name Name of the element being processed
         * @param format Format object for processing tokens
         * @param tokens Vector of strings containing tokens to process
         * @return Processed JSON data based on element type
         */
        mho_json ProcessTokens(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);
        /**
         * @brief Processes a compound element from tokens and format, returning its data as an mho_json.
         * 
         * @param element_name Name of the compound element to process
         * @param format Reference to the formatting rules for the compound element
         * @param tokens Reference to the vector of string tokens representing the compound element
         * @return mho_json containing the processed data of the compound element
         */
        mho_json ProcessCompound(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);

        /**
         * @brief Checks if a given token matches a specified type.
         * 
         * @param token Input token to be matched against type.
         * @param type_name Type name to match against.
         * @return True if token matches the type, false otherwise.
         */
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

} // namespace hops

#endif /*! end of include guard: MHO_VexBlockParser */
