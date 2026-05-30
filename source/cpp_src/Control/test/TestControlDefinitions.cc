#include <iostream>

#include "MHO_ControlDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

int main()
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // - CASE 1 - DetermineControlType maps every known type string to its enum
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("int") == control_int_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("list_int") == control_list_int_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("real") == control_real_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("list_real") == control_list_real_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("string") == control_string_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("list_string") == control_list_string_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("fixed_length_list_string") == control_fixed_length_list_string_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("conditional") == control_conditional_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("bool") == control_bool_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("compound") == control_compound_type);
    // - CASE 2 - logical_intersection_list_string aliases to plain list_string
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("logical_intersection_list_string") == control_list_string_type);
    // - CASE 3 - anything unrecognized falls back to the unknown type
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("") == control_unknown_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("deprecated") == control_unknown_type);
    REQUIRE(MHO_ControlDefinitions::DetermineControlType("not_a_real_type") == control_unknown_type);

    return 0;
}
