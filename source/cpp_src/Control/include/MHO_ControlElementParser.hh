#ifndef MHO_ControlElementParser_HH__
#define MHO_ControlElementParser_HH__

#include <list>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "MHO_DirectoryInterface.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_ControlDefinitions.hh"
#include "MHO_ControlTokenProcessor.hh"

namespace hops
{

/*!
 *@file  MHO_ControlElementParser.hh
 *@class  MHO_ControlElementParser
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu May 26 16:55:16 2022 -0400
 *@brief
 */

/**
 * @brief Class MHO_ControlElementParser
 */
class MHO_ControlElementParser
{
    public:
        MHO_ControlElementParser();
        virtual ~MHO_ControlElementParser();

        /**
         * @brief Parses a control statement into a JSON object, handling different formats and providing warnings for unsupported or deprecated elements.
         *
         * @param control_statement Input control statement to parse
         * @return mho_json object representing the parsed control statement
         */
        mho_json ParseControlStatement(const MHO_ControlStatement& control_statement);

    private:
        /**
         * @brief Parses tokens based on element name and format, returning processed data as mho_json.
         *
         * @param element_name Name of the control element being parsed
         * @param format Reference to the format definition for the control element
         * @param tokens Vector of MHO_Tokens containing the input tokens
         * @return Processed data as mho_json based on the control element type
         */
        mho_json ParseTokens(const std::string& element_name, mho_json& format, const std::vector< MHO_Token >& tokens);
        /**
         * @brief Parses compound elements from tokens and formats them into a JSON object.
         *
         * @param element_name Name of the element being processed
         * @param format Format definition for parsing the element
         * @param tokens Vector of MHO_Tokens containing the element data
         * @return JSON object representing the parsed compound element
         */
        mho_json ProcessCompound(const std::string& element_name, mho_json& format, const std::vector< MHO_Token >& tokens);

        mho_json fElementFormats;

        MHO_ControlTokenProcessor fTokenProcessor;
};

} // namespace hops

#endif /*! end of include guard: MHO_ControlElementParser */
