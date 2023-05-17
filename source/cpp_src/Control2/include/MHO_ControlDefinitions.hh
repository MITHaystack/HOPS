#ifndef MHO_ControlElements_HH__
#define MHO_ControlElements_HH__

/*
*@file: MHO_ControlElements.hh
*@class: MHO_ControlElements
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <string>
#include <vector>

#include "MHO_DirectoryInterface.hh"

namespace hops
{

struct MHO_ControlLine
{
    std::size_t fLineNumber;
    std::string fContents;
    std::vector< std::string > fTokens;
};

struct MHO_ControlStatement
{
    std::string fKeyword;
    std::vector< std::string > fTokens;
};

enum control_element_type
{
    control_int_type,
    control_list_int_type,
    control_real_type,
    control_string_type,
    control_list_string_type,
    control_list_real_type,
    control_conditional_type,
    control_compound_type,
    control_unknown_type
};



class MHO_ControlDefinitions
{
    public:
        MHO_ControlDefinitions(){};
        virtual ~MHO_ControlDefinitions(){};

        static std::string GetFormatDirectory();
        static std::vector< std::string > GetKeywordNames();

        static std::string WhitespaceDelim() {return std::string(" \t\r\n");};
        static std::string CommentFlag() {return std::string("*");};

        static control_element_type DetermineControlType(std::string etype);

    private:

};






}

#endif /* end of include guard: MHO_ControlElements */
