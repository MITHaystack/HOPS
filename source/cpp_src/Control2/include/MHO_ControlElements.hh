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
    std::string keyword;
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
    control_conditional_type
};


}

#endif /* end of include guard: MHO_ControlElements */
