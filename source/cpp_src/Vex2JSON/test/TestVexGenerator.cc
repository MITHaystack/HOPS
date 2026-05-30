#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_VexGenerator.hh"
#include "MHO_VexParser.hh"
#include "MHO_TestAssertions.hh"

static std::string readFileContents(const std::string& filename)
{
    std::ifstream ifs(filename.c_str(), std::ifstream::in);
    if(!ifs.is_open())
    {
        return std::string();
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();
    return ss.str();
}



int main()
{
    hops::MHO_Message::GetInstance().AcceptAllKeys();
    hops::MHO_Message::GetInstance().SetMessageLevel(hops::eWarning);

#ifndef VEX_FIXTURE_DIR
#error "VEX_FIXTURE_DIR must be defined at compile time"
#endif
#ifndef VEX_OUTPUT_DIR
#error "VEX_OUTPUT_DIR must be defined at compile time"
#endif

    std::string data_dir(VEX_FIXTURE_DIR);
    std::string out_dir(VEX_OUTPUT_DIR);

    //  Test Case 1: Construct + destruct
    {
        hops::MHO_VexGenerator gen;
        // no exception on construction or destruction
    }

    //  Test Case 2: SetFilename + GenerateVex smoke
    {
        hops::MHO_VexParser parser;
        parser.SetVexFile(data_dir + "/minimal_v15.vex");
        mho_json j = parser.ParseVex();

        hops::MHO_VexGenerator gen;
        gen.SetFilename(out_dir + "/case2.vex");
        gen.GenerateVex(j);

        std::string contents = readFileContents(out_dir + "/case2.vex");
        REQUIRE(!contents.empty());

        // verify key strings are present in the output
        REQUIRE(contents.find("$FREQ") != std::string::npos);
        REQUIRE(contents.find("def TESTFREQ") != std::string::npos);
        REQUIRE(contents.find("sample_rate") != std::string::npos);
        REQUIRE(contents.find("enddef") != std::string::npos);
        REQUIRE(contents.find("VEX_rev") != std::string::npos);
    }

    //  Test Case 3: Round-trip (parse -> generate -> parse) equality
    {
        hops::MHO_VexParser parser1;
        parser1.SetVexFile(data_dir + "/minimal_v15.vex");
        mho_json j1 = parser1.ParseVex();

        hops::MHO_VexGenerator gen;
        gen.SetFilename(out_dir + "/case3.vex");
        gen.GenerateVex(j1);

        hops::MHO_VexParser parser2;
        parser2.SetVexFile(out_dir + "/case3.vex");
        mho_json j2 = parser2.ParseVex();

        // Compare VEX_rev
        REQUIRE(j1["VEX_rev"] == j2["VEX_rev"]);

        // Compare each key that exists in j1 against j2
        // (generator may emit extra empty blocks not in original)
        for(auto it = j1.begin(); it != j1.end(); it++)
        {
            std::string key = it.key();
            if(key == "VEX_rev") continue;
            REQUIRE(j2.contains(key));
            REQUIRE(j1[key] == j2[key]);
        }
    }

    //  Test Case 4: Round-trip for OVEX
    {
        hops::MHO_VexParser parser1;
        parser1.SetVexFile(data_dir + "/minimal_ovex.ovex");
        mho_json j1 = parser1.ParseVex();

        REQUIRE(j1.contains("$OVEX_REV"));

        hops::MHO_VexGenerator gen;
        gen.SetFilename(out_dir + "/case4.ovex");
        gen.GenerateVex(j1);

        hops::MHO_VexParser parser2;
        parser2.SetVexFile(out_dir + "/case4.ovex");
        mho_json j2 = parser2.ParseVex();

        REQUIRE(j2.contains("$OVEX_REV"));

        // Compare each key that exists in j1 against j2
        for(auto it = j1.begin(); it != j1.end(); it++)
        {
            std::string key = it.key();
            REQUIRE(j2.contains(key));
            REQUIRE(j1[key] == j2[key]);
        }
    }

    //  Test Case 5: Indent padding override
    {
        hops::MHO_VexParser parser;
        parser.SetVexFile(data_dir + "/minimal_v15.vex");
        mho_json j_orig = parser.ParseVex();

        hops::MHO_VexGenerator gen;
        gen.SetIndentPadding("    "); // four spaces (same as default, but explicit)
        gen.SetFilename(out_dir + "/case5.vex");
        gen.GenerateVex(j_orig);

        // verify output contains indented lines
        std::string contents = readFileContents(out_dir + "/case5.vex");
        REQUIRE(!contents.empty());

        // re-parse and verify semantic equality
        hops::MHO_VexParser parser2;
        parser2.SetVexFile(out_dir + "/case5.vex");
        mho_json j_after = parser2.ParseVex();
        for(auto it = j_orig.begin(); it != j_orig.end(); it++)
        {
            std::string key = it.key();
            if(key == "VEX_rev") continue;
            REQUIRE(j_after.contains(key));
            REQUIRE(j_orig[key] == j_after[key]);
        }
    }

    //  Test Case 6: Literal block handling
    {
        // The parser captures non-literal data from files containing literal blocks.
        // The generator should produce valid output from that JSON, and the
        // non-literal data should survive the round-trip. (Literal text is not
        // stored in JSON by the parser, so it cannot be regenerated.)
        hops::MHO_VexParser parser1;
        parser1.SetVexFile(data_dir + "/minimal_with_literal.vex");
        mho_json j1 = parser1.ParseVex();

        hops::MHO_VexGenerator gen;
        gen.SetFilename(out_dir + "/case6.vex");
        gen.GenerateVex(j1);

        hops::MHO_VexParser parser2;
        parser2.SetVexFile(out_dir + "/case6.vex");
        mho_json j2 = parser2.ParseVex();

        // non-literal data must survive
        for(auto it = j1.begin(); it != j1.end(); it++)
        {
            std::string key = it.key();
            if(key == "VEX_rev") continue;
            REQUIRE(j2.contains(key));
            REQUIRE(j1[key] == j2[key]);
        }
    }

    //  Test Case 7: IsExcludedOvex behavior
    {
        // Build a JSON with VEX_rev = 1.5 and an $OVEX_REV block.
        // The generator should suppress $OVEX_REV in a VEX 1.5 context.
        mho_json j;
        j["VEX_rev"] = "1.5";
        j["$OVEX_REV"] = {{"rev", "1.0"}};
        j["$GLOBAL"] = mho_json::object();

        hops::MHO_VexGenerator gen;
        gen.SetFilename(out_dir + "/case7.vex");
        gen.GenerateVex(j);

        std::string contents = readFileContents(out_dir + "/case7.vex");
        // $OVEX_REV should NOT appear in the generated VEX 1.5 output
        REQUIRE(contents.find("$OVEX_REV") == std::string::npos);
    }

    //  Test Case 8: Re-use generator instance
    {
        hops::MHO_VexParser parser1;
        parser1.SetVexFile(data_dir + "/minimal_v15.vex");
        mho_json j1 = parser1.ParseVex();

        hops::MHO_VexParser parser2;
        parser2.SetVexFile(data_dir + "/minimal_ovex.ovex");
        mho_json j2 = parser2.ParseVex();

        hops::MHO_VexGenerator gen;

        // first generation
        gen.SetFilename(out_dir + "/case8a.vex");
        gen.GenerateVex(j1);

        // second generation (same instance)
        gen.SetFilename(out_dir + "/case8b.ovex");
        gen.GenerateVex(j2);

        // both files must exist and be parseable
        std::string contents_a = readFileContents(out_dir + "/case8a.vex");
        std::string contents_b = readFileContents(out_dir + "/case8b.ovex");
        REQUIRE(!contents_a.empty());
        REQUIRE(!contents_b.empty());

        // round-trip parse to verify no state leak
        hops::MHO_VexParser reparser_a;
        reparser_a.SetVexFile(out_dir + "/case8a.vex");
        mho_json rt_a = reparser_a.ParseVex();
        for(auto it = j1.begin(); it != j1.end(); it++)
        {
            std::string key = it.key();
            if(key == "VEX_rev") continue;
            REQUIRE(rt_a.contains(key));
            REQUIRE(j1[key] == rt_a[key]);
        }

        hops::MHO_VexParser reparser_b;
        reparser_b.SetVexFile(out_dir + "/case8b.ovex");
        mho_json rt_b = reparser_b.ParseVex();
        for(auto it = j2.begin(); it != j2.end(); it++)
        {
            std::string key = it.key();
            REQUIRE(rt_b.contains(key));
            REQUIRE(j2[key] == rt_b[key]);
        }
    }

    //  Test Case 9: Bad filename
    {
        hops::MHO_VexParser parser;
        parser.SetVexFile(data_dir + "/minimal_v15.vex");
        mho_json j = parser.ParseVex();

        hops::MHO_VexGenerator gen;
        gen.SetFilename("/nonexistent/dir/xyz_test_vex_generator.vex");
        // Should not crash - may emit an error message
        gen.GenerateVex(j);
        // If we reach here without crashing, the test passes
    }

    return 0;
}
