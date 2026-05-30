#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_ControlElementParser.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static MHO_ControlStatement makeStmt(const std::string& kw, const std::vector< std::string >& vals)
{
    MHO_ControlStatement s;
    s.fStartLineNumber = 1;
    s.fKeyword = kw;
    for(std::size_t i = 0; i < vals.size(); ++i)
    {
        MHO_Token t;
        t.fValue = vals[i];
        t.fLineNumber = 1;
        s.fTokens.push_back(t);
    }
    return s;
}

int main()
{
    // Suppress all messages so deprecated/unknown warnings don't pollute output
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Construct once - fElementFormats loaded from disk in constructor
    MHO_ControlElementParser parser;

    // CASE 1 - Real scalar (pc_delay_l)
    {
        auto stmt = makeStmt("pc_delay_l", {"1.5"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["name"] == "pc_delay_l");
        REQUIRE(e["statement_type"] == "operator");
        REQUIRE(e["value"].is_number_float());
        CHECK_CLOSE(e["value"].get< double >(), 1.5, 1e-12);
    }

    // CASE 2 - list_real (sb_win) with negatives
    {
        auto stmt = makeStmt("sb_win", {"-10.0", "10.0"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["statement_type"] == "parameter");
        REQUIRE(e["value"].is_array());
        REQUIRE(e["value"].size() == 2);
        CHECK_CLOSE(e["value"][0].get< double >(), -10.0, 1e-12);
        CHECK_CLOSE(e["value"][1].get< double >(), 10.0, 1e-12);
    }

    // CASE 3 - logical_intersection_list_string maps to list_string (freqs)
    {
        auto stmt = makeStmt("freqs", {"a", "b", "c"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["value"].is_array());
        REQUIRE(e["value"].size() == 3);
        REQUIRE(e["value"][0].get< std::string >() == "a");
        REQUIRE(e["value"][1].get< std::string >() == "b");
        REQUIRE(e["value"][2].get< std::string >() == "c");
    }

    // CASE 4 - bool (gen_cf_record) - true and false
    {
        auto stmtA = makeStmt("gen_cf_record", {"true"});
        mho_json eA = parser.ParseControlStatement(stmtA);
        REQUIRE(eA["value"].is_boolean());
        REQUIRE(eA["value"].get< bool >() == true);

        auto stmtB = makeStmt("gen_cf_record", {"false"});
        mho_json eB = parser.ParseControlStatement(stmtB);
        REQUIRE(eB["value"].is_boolean());
        REQUIRE(eB["value"].get< bool >() == false);
    }

    // CASE 5 - Compound (pc_phases)
    {
        auto stmt = makeStmt("pc_phases", {"abcd", "10.0", "20.0", "30.0", "40.0"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["statement_type"] == "operator");
        REQUIRE(e["value"]["channel_names"] == "abcd");
        REQUIRE(e["value"]["pc_phases"].is_array());
        REQUIRE(e["value"]["pc_phases"].size() == 4);
        CHECK_CLOSE(e["value"]["pc_phases"][0].get< double >(), 10.0, 1e-12);
        CHECK_CLOSE(e["value"]["pc_phases"][1].get< double >(), 20.0, 1e-12);
        CHECK_CLOSE(e["value"]["pc_phases"][2].get< double >(), 30.0, 1e-12);
        CHECK_CLOSE(e["value"]["pc_phases"][3].get< double >(), 40.0, 1e-12);
    }

    // CASE 6 - Conditional passthrough (if)
    {
        auto stmt = makeStmt("if", {"station", "G"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["statement_type"] == "conditional");
        REQUIRE(e["value"].is_array());
        REQUIRE(e["value"].size() == 2);
        REQUIRE(e["value"][0].get< std::string >() == "station");
        REQUIRE(e["value"][1].get< std::string >() == "G");
    }

    // CASE 7 - Unknown keyword (no format file)
    {
        auto stmt = makeStmt("this_keyword_does_not_exist", {"x"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["statement_type"] == "unknown");
        REQUIRE(e.find("value") == e.end());
    }

    // CASE 9 - Reuse / idempotency
    {
        auto stmt = makeStmt("pc_delay_l", {"1.5"});
        mho_json e1 = parser.ParseControlStatement(stmt);
        mho_json e2 = parser.ParseControlStatement(stmt);
        REQUIRE(e1 == e2);
    }

    // CASE 10 - int scalar type (start)
    {
        auto stmt = makeStmt("start", {"42"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["statement_type"] == "parameter");
        REQUIRE(e["value"].is_number_integer());
        REQUIRE(e["value"].get< int >() == 42);
    }

    // CASE 11 - string scalar type (pc_mode) - kept verbatim, not coerced to a number
    {
        auto stmt = makeStmt("pc_mode", {"manual"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["statement_type"] == "parameter");
        REQUIRE(e["value"].is_string());
        REQUIRE(e["value"].get< std::string >() == "manual");
    }

    // CASE 12 - fixed_length_list_string type (samplers): leading count is consumed
    {
        auto stmt = makeStmt("samplers", {"2", "ab", "cd"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["statement_type"] == "parameter");
        REQUIRE(e["value"].is_array());
        REQUIRE(e["value"].size() == 2);
        REQUIRE(e["value"][0].get< std::string >() == "ab");
        REQUIRE(e["value"][1].get< std::string >() == "cd");
    }

    // CASE 13 - compound with a list_int field (pc_tonemask): the trailing list
    // field consumes all remaining tokens.
    {
        auto stmt = makeStmt("pc_tonemask", {"abcd", "1", "2", "3"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["statement_type"] == "parameter");
        REQUIRE(e["value"]["channel_names"] == "abcd");
        REQUIRE(e["value"]["tone_masks"].is_array());
        REQUIRE(e["value"]["tone_masks"].size() == 3);
        REQUIRE(e["value"]["tone_masks"][0].get< int >() == 1);
        REQUIRE(e["value"]["tone_masks"][1].get< int >() == 2);
        REQUIRE(e["value"]["tone_masks"][2].get< int >() == 3);
    }

    // CASE 14 - deprecated keyword (dec_offset): warned and left as "unknown",
    // with no "value" key populated.
    {
        auto stmt = makeStmt("dec_offset", {"1.0"});
        mho_json e = parser.ParseControlStatement(stmt);
        REQUIRE(e["statement_type"] == "unknown");
        REQUIRE(e.find("value") == e.end());
    }

    // CASE 15 - A known keyword with zero tokens throws (formerly std::exit(1)).
    {
        auto stmt = makeStmt("pc_delay_l", {});
        REQUIRE_THROWS(parser.ParseControlStatement(stmt));
    }

    return 0;
}
