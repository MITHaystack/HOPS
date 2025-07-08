#ifndef MHO_ControlFileParser_HH__
#define MHO_ControlFileParser_HH__

#include <list>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_ControlDefinitions.hh"
#include "MHO_ControlElementParser.hh"

namespace hops
{

/*!
 *@file  MHO_ControlFileParser.hh
 *@class  MHO_ControlFileParser
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu May 11 15:00:11 2023 -0400
 *@brief
 */

/**
 * @brief Class MHO_ControlFileParser
 */
class MHO_ControlFileParser
{
    public:
        MHO_ControlFileParser();
        virtual ~MHO_ControlFileParser();

        /**
         * @brief Sets the global set_string (control statement) variable.
         * 
         * @param set_string The new value to be assigned to the global set_string.
         */
        void PassSetString(std::string set_string) { fSetString = set_string; };

        /**
         * @brief Setter for control file
         * 
         * @param filename New control file name
         */
        void SetControlFile(std::string filename);

        /**
         * @brief Parses control file and constructs JSON object representing control objects.
         * 
         * @return mho_json object containing constructed control objects.
         */
        mho_json ParseControl();

        /**
         * @brief Getter for processed control file text (i.e. all of the tokens that make it into the control flow)
         * 
         * @return Processed control file text as std::string
         */
        std::string GetProcessedControlFileText() const { return fProcessedControlFileText; }

        /**
         * @brief Getter for legacy processed control file text 
         * (just the control file tokens without set-string additions, needed for backwards compatible type_222 records)
         * 
         * @return std::string containing legacy processed control file text
         */
        std::string GetLegacyProcessedControlFileText() const { return fLegacyProcessedControlFileText; }

    private:
        /**
         * @brief Reads a control file and appends/set string portions to it.
         * 
         * @return True if file read successfully, false otherwise.
         */
        bool ReadFile();
        /**
         * @brief Removes comment lines and trims leading comments from non-empty lines in fLines.
         */
        void RemoveComments();
        /**
         * @brief Fixes symbols in lines by padding parentheses, ! and <,  with spaces.
         */
        void FixSymbols();
        /**
         * @brief Tokenizes lines in the control file parser using specified delimiters and settings.
         */
        void TokenizeLines();
        /**
         * @brief Merges tokens from all lines in fLines, excluding first and last lines for legacy tokens.
         */
        void MergeTokens();
        /**
         * @brief Exports tokens from control files and stores them in respective processed text variables.
         */
        void ExportTokens();
        /**
         * @brief Searches for keywords in file tokens and stores their locations.
         */
        void FindKeywords();
        /**
         * @brief Forms control statements by splitting tokens into sections governed by keywords.
         */
        void FormStatements();
        /**
         * @brief Constructs control objects from statements and adds them to a JSON root.
         * 
         * @return mho_json representing the constructed control objects.
         */
        mho_json ConstructControlObjects();

        /**
         * @brief Splits a set string into prepend and append parts based on 'if' statement presence.
         * 
         * @param set_string Input set string to be split
         * @param prepend Output: part of set_string before first 'if' statement
         * @param append Output: part of set_string after first 'if' statement
         */
        void SplitSetString(const std::string& set_string, std::string& prepend, std::string& append);

        /**
         * @brief Replaces occurrences of find_str in text with replace_str using regex_str as pattern.
         * 
         * @param find_str String to search for and replace
         * @param regex_str Regular expression pattern for matching
         * @param replace_str String to replace matches with
         * @param text Input/output string where replacements occur
         */
        void FindAndReplace(const std::string& find_str, const std::string& regex_str, const std::string& replace_str,
                            std::string& text);

        std::string fSetString;
        std::string fControlFileName;

        //token/delimiter definitions
        std::string fWhitespace;
        std::string fCommentFlag;

        //workspace
        std::string fLine; //the line from the input file
        std::list< MHO_ControlLine > fLines;
        using line_itr = std::list< MHO_ControlLine >::iterator;

        //format definition
        std::string fFormatDirectory;
        std::string fControlVersion;
        std::vector< std::string > fKeywordNames;
        mho_json fKeywordNamesJSON;

        //token processing
        MHO_Tokenizer fTokenizer;
        std::vector< MHO_Token > fFileTokens;            //all tokens from file and set string
        std::vector< std::size_t > fKeywordLocations;    //index of each keyword token
        std::vector< MHO_ControlStatement > fStatements; //collection of tokens for each logical statement

        MHO_ControlElementParser fElementParser;

        //output (the parsed text of the control file + set_string with comments removed )
        std::string fProcessedControlFileText;
        std::string fLegacyProcessedControlFileText;
        std::vector< MHO_Token > fLegacyFileTokens; //only tokens from file
};

} // namespace hops

#endif /*! end of include guard: MHO_ControlFileParser */
