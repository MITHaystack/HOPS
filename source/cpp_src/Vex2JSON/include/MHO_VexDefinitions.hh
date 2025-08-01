#ifndef MHO_VexDefinitions_HH__
#define MHO_VexDefinitions_HH__

/*!
 *@file  MHO_VexDefinitions.hh
 *@class  MHO_VexDefinitions
 *@author  J. Barrett - barrettj@mit.edu
 *@date Tue Jun 21 11:08:31 2022 -0400
 *@brief  Basic definitions of various tokens and and patterns special for parsing VEX files
 */

#include "MHO_Message.hh"
#include <regex>
#include <string>
#include <vector>

namespace hops
{

/**
 * @brief Replaces occurrences of pattern in value with replacement.
 *
 * @param value Input string where patterns will be replaced.
 * @param pattern Pattern to search for within input string.
 * @param replacement String to replace found patterns.
 * @return Modified string after replacing all occurrences of pattern.
 */
inline std::string string_pattern_replace(const std::string& value, const std::string& pattern, const std::string& replacement)
{
    //original implementation is via std::regex, however, std::regex is not implemented for <GCC 4.9
    //return std::regex_replace(value, std::regex(pattern), replacement);
    if(pattern == replacement)
    {
        return value;
    }
    std::string tmp = value;
    std::size_t loc = std::string::npos;
    do
    {
        loc = tmp.find(pattern);
        if(loc != std::string::npos)
        {
            tmp.replace(loc, pattern.length(), replacement);
        }
    }
    while(loc != std::string::npos);
    return tmp;
}

enum vex_element_type
{
    vex_int_type,
    vex_list_int_type,
    vex_real_type,
    vex_string_type,
    vex_list_string_type,
    vex_epoch_type,
    vex_radec_type,
    vex_list_real_type,
    vex_compound_type,
    vex_list_compound_type,
    vex_link_type,
    vex_keyword_type,
    vex_reference_type,
    vex_unknown_type
};

/**
 * @brief Class MHO_VexDefinitions
 */
class MHO_VexDefinitions
{
    public:
        MHO_VexDefinitions();
        virtual ~MHO_VexDefinitions();

        /**
         * @brief Setter for vex version
         *
         * @param version New vex version as string
         */
        void SetVexVersion(std::string version);

        /**
         * @brief Getter for format directory
         *
         * @return std::string - The format directory path.
         */
        std::string GetFormatDirectory() const;

        /**
         * @brief Getter for block names
         *
         * @return std::vector<std::string containing block names.
         */
        std::vector< std::string > GetBlockNames() const { return fBlockNames; }

        /**
         * @brief Determines and returns the version of a VEX file given its filename.
         *
         * @param filename The path to the VEX file.
         * @return A string representing the file's version (e.g., '1.5', '2.0', or 'ovex'), or 'unknown' if not determined.
         * @note This is a static function.
         */
        static std::string DetermineFileVersion(std::string filename);

        /**
         * @brief Returns a static string representing the start block flag.
         *
         * @return A std::string containing the character '$'.
         * @note This is a static function.
         */
        static std::string BlockStartFlag() { return std::string("$"); };

        /**
         * @brief Returns a static string 'ref'.
         *
         * @return std::string containing 'ref'
         * @note This is a static function.
         */
        static std::string RefTag() { return std::string("ref"); }

        /**
         * @brief Returns a string containing whitespace characters.
         *
         * @return A std::string containing space (' '), tab (\t), carriage return (\r), and newline (\n) characters.
         * @note This is a static function.
         */
        static std::string WhitespaceDelim() { return std::string(" \t\r\n"); };

        /**
         * @brief Returns a string literal '='.
         *
         * @return A std::string containing the assignment operator ('=').
         * @note This is a static function.
         */
        static std::string AssignmentOp() { return std::string("="); };

        /**
         * @brief Returns a string '=;' as the assignment delimiter.
         *
         * @return A std::string containing '=;'.
         * @note This is a static function.
         */
        static std::string AssignmentDelim() { return std::string("=;"); };

        /**
         * @brief Returns a string containing whitespace characters.
         *
         * @return A std::string containing whitespace
         * @note This is a static function.
         */
        static std::string StartTagDelim() { return std::string(" \t\r\n;"); };

        /**
         * @brief Returns a static string ':' as an element delimiter.
         *
         * @return A std::string containing ':'
         * @note This is a static function.
         */
        static std::string ElementDelim() { return ":"; };

        /**
         * @brief Returns a static string 'VEX_rev' for revision flag.
         *
         * @return std::string containing 'VEX_rev'
         * @note This is a static function.
         */
        static std::string VexRevisionFlag() { return std::string("VEX_rev"); };

        /**
         * @brief Returns a string containing the OVEX revision flag.
         *
         * @return A std::string containing '$OVEX_REV'.
         * @note This is a static function.
         */
        static std::string OVexRevisionFlag() { return std::string("$OVEX_REV"); };

        /**
         * @brief Returns a static string 'start_literal'.
         *
         * @return A std::string containing 'start_literal'
         * @note This is a static function.
         */
        static std::string StartLiteralFlag() { return std::string("start_literal"); };

        /**
         * @brief Returns a static string 'end_literal'.
         *
         * @return std::string containing 'end_literal'
         * @note This is a static function.
         */
        static std::string EndLiteralFlag() { return std::string("end_literal"); };

        /**
         * @brief Returns a string containing an asterisk (*). This is a static function.
         *
         * @return A string containing an asterisk (*)
         * @note This is a static function.
         */
        static std::string CommentFlag() { return std::string("*"); };

        /**
         * @brief Returns a string representing the statement end flag.
         *
         * @return A std::string containing ';'.
         * @note This is a static function.
         */
        static std::string StatementEndFlag() { return std::string(";"); };

        /**
         * @brief Returns a string representing the end of a statement line.
         *
         * @return A std::string containing ';
'.
         * @note This is a static function.
         */
        static std::string StatementLineEnd() { return std::string(";\n"); };

        static std::string OptionalFlag() { return std::string("!"); };

        static vex_element_type DetermineType(std::string etype);

        static bool IsOptionalField(std::string& field_name);

    private:
        std::string fFormatDirectory;
        std::string fVexVersion;
        std::vector< std::string > fBlockNames;
};

} // namespace hops

#endif /*! end of include guard: MHO_VexDefinitions */
