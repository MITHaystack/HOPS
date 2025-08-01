#ifndef MHO_ControlElements_HH__
#define MHO_ControlElements_HH__

#include <string>
#include <vector>

#include "MHO_DirectoryInterface.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
 *@file  MHO_ControlElements.hh
 *@class  MHO_ControlElements
 *@author  J. Barrett - barrettj@mit.edu
 *@date Mon May 15 16:14:11 2023 -0400
 *@brief
 */

/**
 * @brief Class MHO_Token
 */
struct MHO_Token
{
        std::string fValue;
        std::size_t fLineNumber;
};

/**
 * @brief Class MHO_ControlLine
 */
struct MHO_ControlLine
{
        std::size_t fLineNumber;
        std::string fContents;
        std::vector< MHO_Token > fTokens;
};

/**
 * @brief Class MHO_ControlStatement
 */
struct MHO_ControlStatement
{
        std::size_t fStartLineNumber;
        std::string fKeyword;
        std::vector< MHO_Token > fTokens;
};

enum control_element_type
{
    control_int_type,
    control_list_int_type,
    control_real_type,
    control_string_type,
    control_list_string_type,
    control_fixed_length_list_string_type,
    control_list_real_type,
    control_conditional_type,
    control_compound_type,
    control_bool_type,
    control_unknown_type
};

/**
 * @brief Class MHO_ControlDefinitions
 */
class MHO_ControlDefinitions
{
    public:
        MHO_ControlDefinitions(){};
        virtual ~MHO_ControlDefinitions(){};

        /**
         * @brief Getter for the control (json) format directory
         *
         * @return std::string representing the format directory.
         * @note This is a static function.
         */
        static std::string GetFormatDirectory();
        /**
         * @brief Getter for control keyword names
         *
         * @return Vector of string keyword names
         * @note This is a static function.
         */
        static std::vector< std::string > GetKeywordNames();

        /**
         * @brief Returns a string containing whitespace characters.
         *
         * @return A std::string containing whitespace characters (space, tab, carriage return, and newline).
         * @note This is a static function.
         */
        static std::string WhitespaceDelim() { return std::string(" \t\r\n"); };

        /**
         * @brief Returns a string containing an asterisk (*).
         *
         * @return A string containing an asterisk (*)
         * @note This is a static function.
         */
        static std::string CommentFlag() { return std::string("*"); };

        /**
         * @brief Determines control type based on input string type.
         *
         * @param etype Input string representing control element type.
         * @return control_element_type corresponding to input string.
         * @note This is a static function.
         */
        static control_element_type DetermineControlType(std::string etype);

        /**
         * @brief Getter for composite/complete control format as a json object
         *
         * @return mho_json object containing all element formats keyed by keyword names
         * @note This is a static function.
         */
        static mho_json GetControlFormat();

    private:
        //mho_json fControlFormat;
};

} // namespace hops

#endif /*! end of include guard: MHO_ControlElements */
