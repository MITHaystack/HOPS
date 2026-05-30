#include <iostream>
#include <string>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_VexParameter.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);


    // Case 1: Default construction
    {
        MHO_VexParameter p;
        REQUIRE(p.GetTokenString() == "");
        REQUIRE(p.GetUnitsString() == "");
    }


    // Case 2: Two-arg constructor
    {
        MHO_VexParameter p("8080.40", "MHz");
        REQUIRE(p.GetTokenString() == "8080.40");
        REQUIRE(p.GetUnitsString() == "MHz");
    }


    // Case 3: Setter mutability + getters
    {
        MHO_VexParameter p;
        p.SetTokenString("64.0");
        p.SetUnitsString("Ms/sec");
        REQUIRE(p.GetTokenString() == "64.0");
        REQUIRE(p.GetUnitsString() == "Ms/sec");
    }


    // Case 4: Empty units (single-arg constructor)
    {
        MHO_VexParameter p("42");
        REQUIRE(p.GetTokenString() == "42");
        REQUIRE(p.GetUnitsString() == "");
    }

    // Case 5: Copy semantics
    {
        MHO_VexParameter a("x", "u");
        MHO_VexParameter b = a;
        REQUIRE(b.GetTokenString() == "x");
        REQUIRE(b.GetUnitsString() == "u");
        b.SetTokenString("modified");
        REQUIRE(a.GetTokenString() == "x");  // a unchanged
    }


    // Case 6: Container usage
    {
        std::vector<MHO_VexParameter> v;
        v.emplace_back("1.0", "s");
        v.emplace_back("2.0", "s");
        REQUIRE(v.size() == 2);
        REQUIRE(v[0].GetTokenString() == "1.0");
        REQUIRE(v[1].GetTokenString() == "2.0");
    }

    return 0;
}
