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

struct MHO_Token
{
        std::string fValue;
        std::size_t fLineNumber;
};

struct MHO_ControlLine
{
        std::size_t fLineNumber;
        std::string fContents;
        std::vector< MHO_Token > fTokens;
};

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

class MHO_ControlDefinitions
{
    public:
        MHO_ControlDefinitions(){};
        virtual ~MHO_ControlDefinitions(){};

        static std::string GetFormatDirectory();
        static std::vector< std::string > GetKeywordNames();

        static std::string WhitespaceDelim() { return std::string(" \t\r\n"); };

        static std::string CommentFlag() { return std::string("*"); };

        static control_element_type DetermineControlType(std::string etype);

        static mho_json GetControlFormat();

    private:
        //mho_json fControlFormat;
};

} // namespace hops

#endif /*! end of include guard: MHO_ControlElements */
