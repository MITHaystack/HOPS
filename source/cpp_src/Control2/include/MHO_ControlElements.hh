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


inline control_element_type 
DetermineControlType(std::string etype)
{
    if(etype == "int"){return control_int_type;}
    if(etype == "list_int"){return control_list_int_type;}
    if(etype == "real"){return control_real_type;}
    if(etype == "list_real"){return control_list_real_type;}
    if(etype == "string"){return control_string_type;}
    if(etype == "list_string"){return control_list_string_type;}
    if(etype == "conditional"){return control_conditional_type;}
    if(etype == "compound"){return control_compound_type;}
    return control_unknown_type;
}



}

#endif /* end of include guard: MHO_ControlElements */
