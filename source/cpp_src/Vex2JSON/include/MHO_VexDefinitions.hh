#ifndef MHO_VexDefinitions_HH__
#define MHO_VexDefinitions_HH__

/*!
 *@file  MHO_VexDefinitions.hh
 *@class  MHO_VexDefinitions
 *@author  J. Barrett - barrettj@mit.edu
 *@date Tue Jun 21 11:08:31 2022 -0400
 *@brief  Basic definitions of various tokens and utilities
 */

#include "MHO_Message.hh"
#include <string>
#include <vector>
#include <regex>

namespace hops
{

inline std::string string_pattern_replace(const std::string& value, const std::string& pattern, const std::string& replacement)
{
    //original implementation is via std::regex, however, std::regex is not implemented for <GCC 4.9
    //return std::regex_replace(value, std::regex(pattern), replacement);
    if(pattern == replacement){return value;}
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

class MHO_VexDefinitions
{
    public:
        MHO_VexDefinitions();
        virtual ~MHO_VexDefinitions();

        void SetVexVersion(std::string version);

        std::string GetFormatDirectory() const;

        std::vector< std::string > GetBlockNames() const { return fBlockNames; }

        static std::string DetermineFileVersion(std::string filename);

        static std::string BlockStartFlag() { return std::string("$"); };

        static std::string RefTag() { return std::string("ref"); }

        static std::string WhitespaceDelim() { return std::string(" \t\r\n"); };

        static std::string AssignmentOp() { return std::string("="); };

        static std::string AssignmentDelim() { return std::string("=;"); };

        static std::string StartTagDelim() { return std::string(" \t\r\n;"); };

        static std::string ElementDelim() { return ":"; };

        static std::string VexRevisionFlag() { return std::string("VEX_rev"); };

        static std::string OVexRevisionFlag() { return std::string("$OVEX_REV"); };

        static std::string StartLiteralFlag() { return std::string("start_literal"); };

        static std::string EndLiteralFlag() { return std::string("end_literal"); };

        static std::string CommentFlag() { return std::string("*"); };

        static std::string StatementEndFlag() { return std::string(";"); };

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
