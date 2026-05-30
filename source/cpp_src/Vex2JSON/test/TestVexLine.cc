#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_VexLine.hh"
#include "MHO_TestAssertions.hh"


using namespace hops;

int main()
{

    MHO_Message::GetInstance().SetMessageLevel(eFatal);


    // Case 1: Default construction
    {
        MHO_VexLine l{};
        REQUIRE(l.fLineNumber == 0u);
        REQUIRE(l.fStatementNumber == 0u);
        REQUIRE(l.fContents == "");
        REQUIRE(l.fIsLiteral == false);
    }


    // Case 2: Aggregate-init
    {
        MHO_VexLine l{7u, 3u, std::string("def NORMAL_MODE"), false};
        REQUIRE(l.fLineNumber == 7u);
        REQUIRE(l.fStatementNumber == 3u);
        REQUIRE(l.fContents == "def NORMAL_MODE");
        REQUIRE(l.fIsLiteral == false);
    }


    // Case 3: Mutability
    {
        MHO_VexLine l{1u, 1u, "abc", false};
        l.fIsLiteral = true;
        REQUIRE(l.fIsLiteral == true);
        l.fContents += " ;";
        REQUIRE(l.fContents == "abc ;");
    }


    // Case 4: Copy semantics
    {
        MHO_VexLine a{1u, 1u, "abc", false};
        MHO_VexLine b = a;
        b.fContents = "xyz";
        REQUIRE(a.fContents == "abc");
        REQUIRE(b.fContents == "xyz");
    }


    // Case 5: Container compatibility
    {
        std::vector<MHO_VexLine> v;
        v.push_back(MHO_VexLine{1u, 1u, "x", false});
        v.push_back(MHO_VexLine{2u, 2u, "y", true});
        REQUIRE(v.size() == 2);
        REQUIRE(v[1].fIsLiteral);
        REQUIRE(v[0].fContents == "x");
    }


    // Case 6: Type-shape pin (compile-time static_assert)
    {
        static_assert(std::is_same<decltype(MHO_VexLine{}.fLineNumber), std::size_t>::value, "fLineNumber type changed");
        static_assert(std::is_same<decltype(MHO_VexLine{}.fStatementNumber), std::size_t>::value, "fStatementNumber type changed");
        static_assert(std::is_same<decltype(MHO_VexLine{}.fContents), std::string>::value, "fContents type changed");
        static_assert(std::is_same<decltype(MHO_VexLine{}.fIsLiteral), bool>::value, "fIsLiteral type changed");
    }

    return 0;
}
