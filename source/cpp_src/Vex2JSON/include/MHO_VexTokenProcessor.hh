#ifndef MHO_VexTokenProcessor_HH__
#define MHO_VexTokenProcessor_HH__

#include "MHO_DirectoryInterface.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_VexDefinitions.hh"
#include "MHO_VexLine.hh"

namespace hops
{

/*!
 *@file  MHO_VexTokenProcessor.hh
 *@class  MHO_VexTokenProcessor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Mon Jun 13 22:27:21 2022 -0400
 *@brief Converts vex tokens (strings) into values of a specific type
 */

/**
 * @brief Class MHO_VexTokenProcessor
 */
class MHO_VexTokenProcessor
{
    public:
        MHO_VexTokenProcessor();
        virtual ~MHO_VexTokenProcessor();

        /**
         * @brief Converts first token to integer and returns as mho_json.
         * 
         * @param element_name Name of the element being processed
         * @param format Reference to the format object (not used in this function)
         * @param tokens Vector of strings containing tokens for processing
         * @return First token converted to integer as mho_json
         */
        mho_json ProcessInt(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);

        /**
         * @brief Converts a list of string tokens into integers and stores them in an mho_json object.
         * 
         * @param element_name Name of the element being processed
         * @param format Reference to an mho_json object for formatting purposes (not used)
         * @param tokens Vector of strings containing integer values to be converted
         * @return mho_json object containing the converted integers as a vector of ints
         */
        mho_json ProcessListInt(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);

        /**
         * @brief Processes a list string into a JSON object and stores tokens.
         * 
         * @param element_name The name of the element to process.
         * @param format Reference to an mho_json object for formatting.
         * @param tokens Reference to a vector of strings containing tokens.
         * @return A JSON object containing the processed list string.
         */
        mho_json ProcessListString(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);

        /**
         * @brief Processes a real number value from tokens and returns it as an mho_json object.
         * 
         * @param element_name Name of the element being processed
         * @param format Reference to the format object for processing
         * @param tokens Vector of strings containing token values
         * @return mho_json object containing 'value' and optionally 'units'
         */
        mho_json ProcessReal(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);

        /**
         * @brief Processes a list of real numbers as strings and converts them into a JSON object with values and optional units.
         * 
         * @param element_name The name of the element being processed.
         * @param format An mho_json reference used internally for formatting.
         * @param tokens A vector of string tokens representing numerical values or value-unit pairs.
         * @return An mho_json object containing 'values' (a vector of doubles) and optionally 'units'.
         */
        mho_json ProcessListReal(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens);

        /**
         * @brief Checks if a given string contains whitespace characters.
         * 
         * @param value Input string to be checked for whitespace.
         * @return True if string contains whitespace, false otherwise.
         */
        bool ContainsWhitespace(std::string value);

    private:
        std::string fWhitespaceDelim;
        MHO_Tokenizer fTokenizer;
};

} // namespace hops

#endif /*! end of include guard: MHO_VexTokenProcessor */
