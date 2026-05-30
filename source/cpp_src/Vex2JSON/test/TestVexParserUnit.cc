#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_VexParser.hh"
#include "MHO_TestAssertions.hh"

int main()
{
    hops::MHO_Message::GetInstance().AcceptAllKeys();
    hops::MHO_Message::GetInstance().SetMessageLevel(hops::eFatal);

#ifndef VEX_FIXTURE_DIR
#error "VEX_FIXTURE_DIR must be defined at compile time"
#endif

    //  Test Case 1: Construct and destruct
    {
        hops::MHO_VexParser parser;
        // Verify no exceptions on construction
    }

    //  Test Case 2: Parse minimal_v15.vex
    {
        hops::MHO_VexParser parser;
        parser.SetVexFile(VEX_FIXTURE_DIR "/minimal_v15.vex");
        auto j = parser.ParseVex();

        // Contains expected top-level keys
        REQUIRE(j.count("$GLOBAL") > 0);
        REQUIRE(j.count("$EXPER") > 0);
        REQUIRE(j.count("$FREQ") > 0);
        REQUIRE(j.count("VEX_rev") > 0);

        // VEX_rev is stored as a string version identifier
        std::string vexRev = (std::string)j["VEX_rev"];
        REQUIRE(vexRev == "1.5");

        // exper_name in $EXPER block
        std::string expName = (std::string)j["$EXPER"]["TEST01"]["exper_name"];
        REQUIRE(expName == "TEST01");

        // sample_rate value ~64.0, units "Ms/sec"
        double sr = (double)j["$FREQ"]["TESTFREQ"]["sample_rate"]["value"];
        REQUIRE(std::fabs(sr - 64.0) < 1e-9);

        std::string units = (std::string)j["$FREQ"]["TESTFREQ"]["sample_rate"]["units"];
        REQUIRE(units == "Ms/sec");

        // chan_def array has one entry with expected band_id
        REQUIRE(j["$FREQ"]["TESTFREQ"]["chan_def"].is_array());
        REQUIRE(j["$FREQ"]["TESTFREQ"]["chan_def"].size() == 1);
        REQUIRE(j["$FREQ"]["TESTFREQ"]["chan_def"][0]["band_id"] == "&X");

        // sky_frequency in chan_def
        double skyFreq = (double)j["$FREQ"]["TESTFREQ"]["chan_def"][0]["sky_frequency"]["value"];
        REQUIRE(std::fabs(skyFreq - 8080.4) < 1e-9);

        // $GLOBAL contains $EXPER reference
        REQUIRE(j["$GLOBAL"].count("$EXPER") > 0);
        REQUIRE(j["$GLOBAL"]["$EXPER"].is_array());
    }

    //  Test Case 3: Comment removal
    {
        hops::MHO_VexParser parser;
        parser.SetVexFile(VEX_FIXTURE_DIR "/minimal_v15.vex");
        auto j = parser.ParseVex();
        std::string dumped = j.dump();
        REQUIRE(dumped.find("a comment line") == std::string::npos);
    }

    //  Test Case 4: Multi-statement-per-line
    {
        hops::MHO_VexParser parser;
        parser.SetVexFile(VEX_FIXTURE_DIR "/minimal_v15_multistmt.vex");
        auto j = parser.ParseVex();
        REQUIRE(j.count("$EXPER") > 0);
        std::string name = (std::string)j["$EXPER"]["TEST01"]["exper_name"];
        REQUIRE(name == "TEST01");
    }

    //  Test Case 5: Literal handling
    // The MHO_VexParser marks start_literal/end_literal sections and skips
    // them during block parsing (literal text is not stored in JSON).
    // We verify that: (a) parsing doesn't crash with literal blocks,
    // (b) non-literal fields are still correctly parsed,
    // (c) the literal text does NOT appear in the JSON output.
    {
        hops::MHO_VexParser parser;
        parser.SetVexFile(VEX_FIXTURE_DIR "/minimal_with_literal.vex");
        auto j = parser.ParseVex();

        // Non-literal data should be parsed correctly despite the literal block
        REQUIRE(j.count("$EXPER") > 0);
        std::string name = (std::string)j["$EXPER"]["TEST01"]["exper_name"];
        REQUIRE(name == "TEST01");

        // Literal text should NOT appear in the JSON (it's skipped by design)
        std::string dumped = j.dump();
        REQUIRE(dumped.find("*=;:&") == std::string::npos);
        REQUIRE(dumped.find("This raw text") == std::string::npos);
    }

    //  Test Case 6: OVEX version
    {
        hops::MHO_VexParser parser;
        parser.SetVexFile(VEX_FIXTURE_DIR "/minimal_ovex.ovex");
        auto j = parser.ParseVex();
        REQUIRE(j.count("$OVEX_REV") > 0);
        // OVEX files default to VEX 1.5 format directory
        REQUIRE(j.count("VEX_rev") > 0);
    }

    //  Test Case 7: Missing VEX_rev
    {
        hops::MHO_VexParser parser;
        parser.SetVexFile(VEX_FIXTURE_DIR "/no_vex_rev.vex");
        // Should not throw; parser defaults to VEX 1.5 format directory when version is undetectable
        auto j = parser.ParseVex();
        // When the first line contains no VEX_rev or $OVEX_REV, DetermineFileVersion
        // returns "unknown". The MHO_VexParser stores this raw string as VEX_rev,
        // while the format directory internally defaults to "1.5".
        REQUIRE(j.count("VEX_rev") > 0);
        std::string rev = (std::string)j["VEX_rev"];
        REQUIRE(rev == "unknown");
        // $GLOBAL block should still be parsed using the default VEX 1.5 format
        REQUIRE(j.count("$GLOBAL") > 0);
    }

    //  Test Case 8: Re-use parser instance, make sure parser state is cleared from one file to the next
    {
        hops::MHO_VexParser parser;
        // First parse
        parser.SetVexFile(VEX_FIXTURE_DIR "/minimal_ovex.ovex");
        auto j1 = parser.ParseVex();
        //shoould have $OVEX_REV
        REQUIRE(j1.count("$OVEX_REV") > 0);
        // Second parse (different file)
        parser.SetVexFile(VEX_FIXTURE_DIR "/minimal_v15.vex");
        auto j2 = parser.ParseVex();
        REQUIRE(j2.count("VEX_rev") > 0); //show have VEX_rev
        REQUIRE(j2.count("$FREQ") > 0); //should have a FREQ section
        REQUIRE(j2.count("$OVEX_REV") == 0); // OVEX-specific block should NOT be present,
    }

    return 0;
}
