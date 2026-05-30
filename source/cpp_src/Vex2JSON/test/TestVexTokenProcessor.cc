#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_VexTokenProcessor.hh"
#include "MHO_TestAssertions.hh"


static double json_double(mho_json& j)
{
    return j.get< double >();
}

int main()
{
    hops::MHO_Message::GetInstance().AcceptAllKeys();
    hops::MHO_Message::GetInstance().SetMessageLevel(hops::eFatal);

    //  Test Case 1: ProcessInt happy path
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {"42"};
        auto r = tp.ProcessInt("exper_num", fmt, tokens);
        REQUIRE(r.is_number_integer());
        REQUIRE(r.get<int>() == 42);
    }

    //  Test Case 2: ProcessInt with trailing whitespace
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {" 17 "};
        auto r = tp.ProcessInt("x", fmt, tokens);
        REQUIRE(r.get<int>() == 17);
    }

    //  Test Case 3: ProcessInt negative
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {"-5"};
        auto r = tp.ProcessInt("x", fmt, tokens);
        REQUIRE(r.get<int>() == -5);
    }

    //  Test Case 4: ProcessListInt
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {"1", "2", "3", "4"};
        auto r = tp.ProcessListInt("freq_state", fmt, tokens);
        REQUIRE(r.is_array());
        REQUIRE(r.size() == 4);
        REQUIRE(r[2].get<int>() == 3);
    }

    //  Test Case 5: ProcessReal without units
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {"3.14"};
        auto r = tp.ProcessReal("x", fmt, tokens);
        double val = json_double(r["value"]);
        REQUIRE(std::fabs(val - 3.14) < 1e-12);
        // units should be absent when no unit string is provided
        REQUIRE(r.contains("units") == false);
    }

    //  Test Case 6: ProcessReal with units embedded in token
    // Note: ProcessReal only examines tokens[0]. Units must be whitespace-separated
    // within that single token (e.g., "8080.40 MHz"). A separate tokens[1] is
    // ignored by the current implementation.
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {"8080.40 MHz"};
        auto r = tp.ProcessReal("x", fmt, tokens);
        double val = json_double(r["value"]);
        REQUIRE(std::fabs(val - 8080.40) < 1e-9);
        REQUIRE(r["units"] == "MHz");
    }

    //  Test Case 7: ProcessReal exponent + sign with units
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {"-1.5e-3 sec"};
        auto r = tp.ProcessReal("x", fmt, tokens);
        double val = json_double(r["value"]);
        REQUIRE(std::fabs(val - (-1.5e-3)) < 1e-15);
        REQUIRE(r["units"] == "sec");
    }

    //  Test Case 8: ProcessListReal with units on last value
    // Note: ProcessListReal only detects units via whitespace within a token.
    // A trailing standalone unit token (e.g., "s") is treated as atof("s") = 0.
    // Units must be embedded: "3.0 s" not "3.0", "s".
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {"1.0", "2.0", "3.0 s"};
        auto r = tp.ProcessListReal("x", fmt, tokens);
        REQUIRE(r["values"].is_array());
        REQUIRE(r["values"].size() == 3);
        REQUIRE(std::fabs(json_double(r["values"][1]) - 2.0) < 1e-12);
        REQUIRE(r["units"] == "s");
    }

    //  Test Case 9: ProcessListReal no units
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {"1.0", "2.0", "3.0"};
        auto r = tp.ProcessListReal("x", fmt, tokens);
        REQUIRE(r["values"].is_array());
        REQUIRE(r["values"].size() == 3);
    }

    //  Test Case 10: ProcessListString
    {
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();

        std::vector<std::string> tokens = {"Aa", "Bb", "Cc"};
        auto r = tp.ProcessListString("x", fmt, tokens);
        REQUIRE(r.is_array());
        REQUIRE(r.size() == 3);
        REQUIRE(r[1].get<std::string>() == "Bb");
    }

    //  Test Case 11: ContainsWhitespace
    {
        hops::MHO_VexTokenProcessor tp;
        REQUIRE(tp.ContainsWhitespace("foo bar") == true);
        REQUIRE(tp.ContainsWhitespace("foo\tbar") == true);
        REQUIRE(tp.ContainsWhitespace("foo") == false);
        REQUIRE(tp.ContainsWhitespace("") == false);
    }

    return 0;
}
