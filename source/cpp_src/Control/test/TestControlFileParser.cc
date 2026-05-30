#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "MHO_ControlFileParser.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static int temp_file_counter = 0;

static std::string make_temp_file(const std::string& content)
{
    std::string path = "/tmp/test_cf_" + std::to_string(::getpid()) + "_" + std::to_string(temp_file_counter++) + ".ctrl";
    std::ofstream(path) << content;
    return path;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug); //eFatal);

    //  CF1 fixture
    const std::string CF1 = "* full line comment, should be removed entirely\n"
                            "if station G\n"
                            "pc_delay_l 1.5      * trailing comment trimmed here\n"
                            "sb_win -10.0 10.0\n"
                            "if station X\n"
                            "sb_win -5.0 5.0\n";

    std::string path1 = make_temp_file(CF1);
    REQUIRE(!path1.empty());

    // CASE 1: Top-level shape / implicit leading "if true" block
    {
        MHO_ControlFileParser parser;
        parser.SetControlFile(path1);
        mho_json J = parser.ParseControl();

        REQUIRE(J.contains("conditions") || J.find("conditions") != J.end());
        REQUIRE(J["conditions"].is_array());
        REQUIRE(J["conditions"].size() == 3); // implicit-true + 2 "if" blocks
        REQUIRE(J["conditions"][0]["statement_type"] == "conditional");
        REQUIRE(J["conditions"][0]["value"] == mho_json({"true"}));
        REQUIRE(J["conditions"][0]["line_number"] == 0);
    }

    // CASE 2: Statements grouped under correct conditional block
    {
        MHO_ControlFileParser parser;
        parser.SetControlFile(path1);
        mho_json J = parser.ParseControl();

        // Block 0: no statements (everything is under "if")
        REQUIRE(J["conditions"][0]["statements"].size() == 0);

        // Block 1: "if station G" has 2 statements
        REQUIRE(J["conditions"][1]["value"] == mho_json({"station", "G"}));
        REQUIRE(J["conditions"][1]["statements"].size() == 2);

        // Block 2: "if station X" has 1 statement
        REQUIRE(J["conditions"][2]["value"] == mho_json({"station", "X"}));
        REQUIRE(J["conditions"][2]["statements"].size() == 1);
    }

    // CASE 3: Element value typing (real, list_real)
    {
        MHO_ControlFileParser parser;
        parser.SetControlFile(path1);
        mho_json J = parser.ParseControl();

        auto& stmts = J["conditions"][1]["statements"];
        // pc_delay_l
        REQUIRE(stmts[0]["name"] == "pc_delay_l");
        REQUIRE(stmts[0]["statement_type"] == "operator");
        REQUIRE(stmts[0]["value"].is_number());
        CHECK_CLOSE(stmts[0]["value"].get< double >(), 1.5, 1e-12);

        // sb_win
        REQUIRE(stmts[1]["name"] == "sb_win");
        REQUIRE(stmts[1]["value"].is_array());
        REQUIRE(stmts[1]["value"].size() == 2);
        CHECK_CLOSE(stmts[1]["value"][0].get< double >(), -10.0, 1e-12);
        CHECK_CLOSE(stmts[1]["value"][1].get< double >(), 10.0, 1e-12);
    }

    // CASE 4: RemoveComments: full-line and trailing
    {
        MHO_ControlFileParser parser;
        parser.SetControlFile(path1);
        parser.ParseControl();
        std::string s = parser.GetProcessedControlFileText();

        REQUIRE(s.find("full line comment") == std::string::npos);
        REQUIRE(s.find("trailing comment") == std::string::npos);
        REQUIRE(s.find("pc_delay_l 1.5") != std::string::npos);
    }

    // CASE 5: test FixSymbols padding (parentheses)
    {
        std::string CF2 = "if (station G or station X)\npc_delay_l 1.0\n";
        std::string path2 = make_temp_file(CF2);
        REQUIRE(!path2.empty());

        MHO_ControlFileParser parser;
        parser.SetControlFile(path2);
        mho_json J = parser.ParseControl();

        // Check the value array
        // Regression test for the FixSymbols fix: parentheses are now padded with
        // spaces (the replace pattern was previously the regex-escaped "\\(" / "\\)",
        // which never matched under the literal-string replacement fallback), so
        // they tokenize as standalone "(" and ")".
        auto& block1 = J["conditions"][1]["value"];
        REQUIRE(block1 == mho_json({"(", "station", "G", "or", "station", "X", ")"}));

        ::remove(path2.c_str());
    }

    // CASE 6: Legacy processed text excludes set-string lines
    {
        MHO_ControlFileParser parser;
        parser.SetControlFile(path1);
        parser.PassSetString("pc_delay_r 2.0");
        parser.ParseControl();

        std::string full = parser.GetProcessedControlFileText();
        std::string legacy = parser.GetLegacyProcessedControlFileText();

        REQUIRE(full.find("pc_delay_r 2.0") != std::string::npos);
        REQUIRE(legacy.find("pc_delay_r 2.0") == std::string::npos);
        REQUIRE(legacy.find("pc_delay_l 1.5") != std::string::npos);
    }

    // CASE 7: Set-string injection adds an applicable block
    {
        MHO_ControlFileParser parser;
        parser.SetControlFile(path1);
        parser.PassSetString("pc_delay_r 2.0");
        mho_json J = parser.ParseControl();

        REQUIRE(J["conditions"].size() == 4);
        auto& lastBlock = J["conditions"][3];
        REQUIRE(lastBlock["value"] == mho_json({"true"}));
        REQUIRE(lastBlock["statements"].size() >= 1);
        auto& stmt = lastBlock["statements"][0];
        REQUIRE(stmt["name"] == "pc_delay_r");
        CHECK_CLOSE(stmt["value"].get< double >(), 2.0, 1e-12);
    }

    // CASE 8: Empty / missing file behavior
    {
        MHO_ControlFileParser parser;
        parser.SetControlFile("");
        mho_json J = parser.ParseControl();

        // Empty filename returns null/empty
        REQUIRE(J.is_null() || J.empty());
    }

    // CASE 9: check parser object reuse / idempotency
    {
        MHO_ControlFileParser parser;
        parser.SetControlFile(path1);
        mho_json J1 = parser.ParseControl();
        mho_json J2 = parser.ParseControl();

        // Regression test for the ReadFile fLines.clear() fix: re-parsing the same
        // file on the same object now yields an identical result (previously the
        // second parse appended a second copy of the lines and doubled them).
        REQUIRE(J2 == J1);
    }

    // CASE 10: statements before any "if" land in the implicit-true block 0
    {
        std::string CF3 = "pc_delay_l 1.0\nif station G\nsb_win -1.0 1.0\n";
        std::string path3 = make_temp_file(CF3);
        REQUIRE(!path3.empty());

        MHO_ControlFileParser parser;
        parser.SetControlFile(path3);
        mho_json J = parser.ParseControl();

        REQUIRE(J["conditions"].size() == 2); // implicit-true + 1 "if"
        REQUIRE(J["conditions"][0]["value"] == mho_json({"true"}));
        REQUIRE(J["conditions"][0]["statements"].size() == 1);
        REQUIRE(J["conditions"][0]["statements"][0]["name"] == "pc_delay_l");
        REQUIRE(J["conditions"][1]["value"] == mho_json({"station", "G"}));
        REQUIRE(J["conditions"][1]["statements"].size() == 1);

        ::remove(path3.c_str());
    }

    // CASE 11: FixSymbols pads "<" / ">" so they tokenize standalone
    {
        std::string CF4 = "if scan < 288-210300\npc_delay_l 1.0\n";
        std::string path4 = make_temp_file(CF4);
        REQUIRE(!path4.empty());

        MHO_ControlFileParser parser;
        parser.SetControlFile(path4);
        mho_json J = parser.ParseControl();

        REQUIRE(J["conditions"][1]["value"] == mho_json({"scan", "<", "288-210300"}));

        ::remove(path4.c_str());
    }

    // CASE 12: comments-only file yields just the implicit-true block, no keywords
    {
        std::string CF5 = "* just a comment\n* another comment\n";
        std::string path5 = make_temp_file(CF5);
        REQUIRE(!path5.empty());

        MHO_ControlFileParser parser;
        parser.SetControlFile(path5);
        mho_json J = parser.ParseControl();

        REQUIRE(J["conditions"].size() == 1);
        REQUIRE(J["conditions"][0]["value"] == mho_json({"true"}));

        ::remove(path5.c_str());
    }

    // CASE 13: a non-empty but unopenable filename throws (formerly std::exit(1))
    {
        MHO_ControlFileParser parser;
        parser.SetControlFile("/no/such/dir/definitely_missing_control_file.ctrl");
        REQUIRE_THROWS(parser.ParseControl());
    }

    // Cleanup
    ::remove(path1.c_str());

    return 0;
}
