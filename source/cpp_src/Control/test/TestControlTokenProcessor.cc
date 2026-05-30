#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "MHO_ControlDefinitions.hh"
#include "MHO_ControlTokenProcessor.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static MHO_Token tok(const std::string& v, std::size_t ln = 1)
{
    MHO_Token t;
    t.fValue = v;
    t.fLineNumber = ln;
    return t;
}

static std::vector< MHO_Token > toks(const std::vector< std::string >& vs)
{
    std::vector< MHO_Token > r;
    for(auto& v : vs)
        r.push_back(tok(v));
    return r;
}

int main()
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    MHO_ControlTokenProcessor proc;

    // CASE 1 - ProcessInt happy path
    {
        auto j = proc.ProcessInt(tok("42"));
        REQUIRE(j.is_number_integer());
        REQUIRE(j.get< int >() == 42);
    }

    // CASE 2 - ProcessInt negative
    {
        auto j = proc.ProcessInt(tok("-7"));
        REQUIRE(j.is_number_integer());
        REQUIRE(j.get< int >() == -7);
    }

    // CASE 3 - ProcessReal, standard + scientific notation
    {
        auto j = proc.ProcessReal(tok("3.14"));
        REQUIRE(j.is_number_float());
        CHECK_CLOSE(j.get< double >(), 3.14, 1e-9);
    }
    {
        auto j = proc.ProcessReal(tok("-2.5e3"));
        REQUIRE(j.is_number_float());
        CHECK_CLOSE(j.get< double >(), -2500.0, 1e-9);
    }

    // CASE 4 - ProcessString preserves value verbatim
    {
        auto j = proc.ProcessString(tok("X1-2_abc"));
        REQUIRE(j.is_string());
        REQUIRE(j.get< std::string >() == "X1-2_abc");
    }
    // check that even something like "12" stays a string
    {
        auto j = proc.ProcessString(tok("12"));
        REQUIRE(j.is_string());
        REQUIRE(j.get< std::string >() == "12");
    }

    // CASE 5 - ProcessBool both values
    {
        auto j = proc.ProcessBool(tok("true"));
        REQUIRE(j.is_boolean());
        REQUIRE(j.get< bool >() == true);
    }
    {
        auto j = proc.ProcessBool(tok("false"));
        REQUIRE(j.is_boolean());
        REQUIRE(j.get< bool >() == false);
    }

    // CASE 6 - ProcessListInt
    {
        auto j = proc.ProcessListInt(toks({"1", "2", "3", "-4"}));
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 4);
        REQUIRE(j[0].is_number_integer());
        REQUIRE(j[0].get< int >() == 1);
        REQUIRE(j[1].get< int >() == 2);
        REQUIRE(j[2].get< int >() == 3);
        REQUIRE(j[3].get< int >() == -4);
    }

    // CASE 7 - ProcessListReal
    {
        auto j = proc.ProcessListReal(toks({"1.0", "2.5", "-0.5"}));
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK_CLOSE(j[0].get< double >(), 1.0, 1e-12);
        CHECK_CLOSE(j[1].get< double >(), 2.5, 1e-12);
        CHECK_CLOSE(j[2].get< double >(), -0.5, 1e-12);
    }

    // CASE 8 - ProcessListString
    {
        auto j = proc.ProcessListString(toks({"a", "bb", "ccc"}));
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        REQUIRE(j[0].get< std::string >() == "a");
        REQUIRE(j[1].get< std::string >() == "bb");
        REQUIRE(j[2].get< std::string >() == "ccc");
    }

    // CASE 9 - ProcessFixedLengthListString
    {
        auto j = proc.ProcessFixedLengthListString(toks({"3", "alpha", "beta", "gamma"}));
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        REQUIRE(j[0].get< std::string >() == "alpha");
        REQUIRE(j[1].get< std::string >() == "beta");
        REQUIRE(j[2].get< std::string >() == "gamma");
    }

    // CASE 9b - ProcessFixedLengthListString with a count of zero (valid empty list).
    // n_elem-1 == 0 matches the declared length, so this is well-formed and yields [].
    {
        auto j = proc.ProcessFixedLengthListString(toks({"0"}));
        REQUIRE(j.is_array());
        REQUIRE(j.empty());
    }

    // CASE 10 - Empty list inputs (boundary)
    {
        auto j = proc.ProcessListInt(toks({}));
        REQUIRE(j.is_array());
        REQUIRE(j.empty());

        j = proc.ProcessListReal(toks({}));
        REQUIRE(j.is_array());
        REQUIRE(j.empty());

        j = proc.ProcessListString(toks({}));
        REQUIRE(j.is_array());
        REQUIRE(j.empty());
    }

    // NOTE: ProcessFixedLengthListString({}) calls std::exit(1) during failure, not exercised here.

    // CASE 11 - Idempotency / reuse
    {
        auto j1 = proc.ProcessInt(tok("5"));
        auto j2 = proc.ProcessInt(tok("5"));
        REQUIRE(j1.get< int >() == 5);
        REQUIRE(j2.get< int >() == 5);
    }

    // - CASE 12 - ProcessInt at the int boundaries.
    // INT_MAX / INT_MIN are valid ints and must convert ok
    {
        const int imax = std::numeric_limits< int >::max();
        const int imin = std::numeric_limits< int >::min();

        auto jmax = proc.ProcessInt(tok(std::to_string(imax)));
        REQUIRE(jmax.is_number_integer());
        REQUIRE(jmax.get< int >() == imax);

        auto jmin = proc.ProcessInt(tok(std::to_string(imin)));
        REQUIRE(jmin.is_number_integer());
        REQUIRE(jmin.get< int >() == imin);
    }

    // - CASE 13 - Malformed inputs now throw (formerly std::exit(1)).
    {
        // non-numeric integer / real
        REQUIRE_THROWS(proc.ProcessInt(tok("abc")));
        REQUIRE_THROWS(proc.ProcessReal(tok("3.1.4")));
        // out-of-range integer (beyond INT_MAX)
        REQUIRE_THROWS(proc.ProcessInt(tok("99999999999")));
        // boolean that is neither "true" nor "false"
        REQUIRE_THROWS(proc.ProcessBool(tok("maybe")));
        // fixed-length list whose declared count disagrees with the token count
        REQUIRE_THROWS(proc.ProcessFixedLengthListString(toks({"3", "only_one"})));
    }

    return 0;
}
