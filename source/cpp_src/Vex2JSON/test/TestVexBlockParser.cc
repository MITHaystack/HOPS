#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_VexBlockParser.hh"
#include "MHO_VexLine.hh"
#include "MHO_TestAssertions.hh"


// Helpers

static hops::MHO_VexLine MakeLine(std::size_t ln, std::size_t stmt, const std::string& contents, bool is_literal = false)
{
    hops::MHO_VexLine l;
    l.fLineNumber = ln;
    l.fStatementNumber = stmt;
    l.fContents = contents;
    l.fIsLiteral = is_literal;
    return l;
}

static bool json_double_close(mho_json& j, double expected, double eps = 1e-9)
{
    double val = j.get< double >();
    return std::fabs(val - expected) < eps;
}




int main()
{
    hops::MHO_Message::GetInstance().AcceptAllKeys();
    hops::MHO_Message::GetInstance().SetMessageLevel(hops::eFatal);

#ifndef VEX_FORMAT_DIR
#error "VEX_FORMAT_DIR must be defined at compile time"
#endif

    //  Test Case 1: Parse $FREQ block (single def)
    {
        std::vector<hops::MHO_VexLine> lines = {
            MakeLine(1, 0, "$FREQ"),
            MakeLine(2, 1, "def TESTFREQ"),
            MakeLine(3, 2, "chan_def = &X : 8080.40 MHz : U : 32.00 MHz : &CH01 : &BBC01 : &U_Cal"),
            MakeLine(4, 3, "sample_rate = 64.00 Ms/sec"),
            MakeLine(5, 4, "enddef"),
        };

        hops::MHO_VexBlockParser parser;
        parser.SetFormatDirectory(VEX_FORMAT_DIR "/vex-1.5/");
        auto j = parser.ParseBlockLines("$FREQ", &lines);

        REQUIRE(j.contains("TESTFREQ"));

        auto& def = j["TESTFREQ"];

        // sample_rate
        REQUIRE(def.contains("sample_rate"));
        REQUIRE(json_double_close(def["sample_rate"]["value"], 64.0));
        REQUIRE(def["sample_rate"]["units"] == "Ms/sec");

        // chan_def (list_compound -> array)
        REQUIRE(def.contains("chan_def"));
        REQUIRE(def["chan_def"].is_array());
        REQUIRE(def["chan_def"].size() == 1);

        auto& chan = def["chan_def"][0];
        REQUIRE(chan["band_id"] == "&X");
        REQUIRE(json_double_close(chan["sky_frequency"]["value"], 8080.40));
        REQUIRE(chan["net_sideband"] == "U");
    }

    //  Test Case 2: Reference line in $MODE block
    {
        std::vector<hops::MHO_VexLine> lines = {
            MakeLine(1, 0, "$MODE"),
            MakeLine(2, 1, "def NORMAL_MODE"),
            MakeLine(3, 2, "ref $FREQ = TESTFREQ : Aa : Bb"),
            MakeLine(4, 3, "enddef"),
        };

        hops::MHO_VexBlockParser parser;
        parser.SetFormatDirectory(VEX_FORMAT_DIR "/vex-1.5/");
        auto j = parser.ParseBlockLines("$MODE", &lines);

        REQUIRE(j.contains("NORMAL_MODE"));

        auto& mode = j["NORMAL_MODE"];
        REQUIRE(mode.contains("$FREQ"));
        REQUIRE(mode["$FREQ"].is_array());
        REQUIRE(mode["$FREQ"].size() == 1);

        auto& ref = mode["$FREQ"][0];
        REQUIRE(ref["keyword"] == "TESTFREQ");
        REQUIRE(ref.contains("qualifiers"));
        REQUIRE(ref["qualifiers"].size() == 2);
        REQUIRE(ref["qualifiers"][0] == "Aa");
        REQUIRE(ref["qualifiers"][1] == "Bb");
    }

    //  Test Case 3: List-of-int element ($FREQ chan_def with freq_state)
    {
        std::vector<hops::MHO_VexLine> lines = {
            MakeLine(1, 0, "$FREQ"),
            MakeLine(2, 1, "def TESTFREQ"),
            MakeLine(3, 2, "chan_def = &X : 8080.40 MHz : U : 32.00 MHz : &CH01 : &BBC01 : &U_Cal : 1 : 2 : 3"),
            MakeLine(4, 3, "sample_rate = 64.00 Ms/sec"),
            MakeLine(5, 4, "enddef"),
        };

        hops::MHO_VexBlockParser parser;
        parser.SetFormatDirectory(VEX_FORMAT_DIR "/vex-1.5/");
        auto j = parser.ParseBlockLines("$FREQ", &lines);

        REQUIRE(j.contains("TESTFREQ"));
        auto& def = j["TESTFREQ"];

        auto& chan = def["chan_def"][0];
        REQUIRE(chan.contains("freq_state"));
        REQUIRE(chan["freq_state"].is_array());
        REQUIRE(chan["freq_state"].size() == 3);
        REQUIRE(chan["freq_state"][0] == 1);
        REQUIRE(chan["freq_state"][1] == 2);
        REQUIRE(chan["freq_state"][2] == 3);
    }

    //  Test Case 4: Compound element ($SOURCE with ra/dec)
    {
        std::vector<hops::MHO_VexLine> lines = {
            MakeLine(1, 0, "$SOURCE"),
            MakeLine(2, 1, "def 3C273"),
            MakeLine(3, 2, "source_name = 3C273"),
            MakeLine(4, 3, "ra = 12h 29m 06.6960s"),
            MakeLine(5, 4, "dec = +02d 03m 08.599s"),
            MakeLine(6, 5, "enddef"),
        };

        hops::MHO_VexBlockParser parser;
        parser.SetFormatDirectory(VEX_FORMAT_DIR "/vex-1.5/");
        auto j = parser.ParseBlockLines("$SOURCE", &lines);

        REQUIRE(j.contains("3C273"));

        auto& src = j["3C273"];
        REQUIRE(src.contains("source_name"));
        REQUIRE(src["source_name"] == "3C273");
        REQUIRE(src.contains("ra"));
        REQUIRE(src.contains("dec"));
    }

    //  Test Case 5: Multiple defs in one block
    {
        std::vector<hops::MHO_VexLine> lines = {
            MakeLine(1, 0, "$FREQ"),
            MakeLine(2, 1, "def FREQ1"),
            MakeLine(3, 2, "chan_def = &X : 8080.40 MHz : U : 32.00 MHz : &CH01 : &BBC01 : &U_Cal"),
            MakeLine(4, 3, "sample_rate = 64.00 Ms/sec"),
            MakeLine(5, 4, "enddef"),
            MakeLine(6, 5, "def FREQ2"),
            MakeLine(7, 6, "chan_def = &Y : 9000.00 MHz : L : 16.00 MHz : &CH02 : &BBC02 : &U_Cal"),
            MakeLine(8, 7, "sample_rate = 32.00 Ms/sec"),
            MakeLine(9, 8, "enddef"),
        };

        hops::MHO_VexBlockParser parser;
        parser.SetFormatDirectory(VEX_FORMAT_DIR "/vex-1.5/");
        auto j = parser.ParseBlockLines("$FREQ", &lines);

        REQUIRE(j.contains("FREQ1"));
        REQUIRE(j.contains("FREQ2"));

        // FREQ1 sample_rate
        REQUIRE(json_double_close(j["FREQ1"]["sample_rate"]["value"], 64.0));
        // FREQ2 sample_rate
        REQUIRE(json_double_close(j["FREQ2"]["sample_rate"]["value"], 32.0));

        // FREQ2 sideband is L
        REQUIRE(j["FREQ2"]["chan_def"][0]["net_sideband"] == "L");
    }

    //  Test Case 6: Missing required field (truncated chan_def)
    {
        std::vector<hops::MHO_VexLine> lines = {
            MakeLine(1, 0, "$FREQ"),
            MakeLine(2, 1, "def TESTFREQ"),
            MakeLine(3, 2, "chan_def = &X"),  // truncated - only band_id
            MakeLine(4, 3, "sample_rate = 64.00 Ms/sec"),
            MakeLine(5, 4, "enddef"),
        };

        hops::MHO_VexBlockParser parser;
        parser.SetFormatDirectory(VEX_FORMAT_DIR "/vex-1.5/");
        auto j = parser.ParseBlockLines("$FREQ", &lines);

        // Parser produces a partial JSON object with the band_id it could parse.
        // The remaining fields are absent from the chan_def entry.
        REQUIRE(j.contains("TESTFREQ"));

        auto& def = j["TESTFREQ"];
        REQUIRE(def.contains("chan_def"));
        REQUIRE(def["chan_def"][0]["band_id"] == "&X");

        // sky_frequency and other fields should be missing
        REQUIRE(def["chan_def"][0].contains("sky_frequency") == false);

        // sample_rate should still be parsed
        REQUIRE(json_double_close(def["sample_rate"]["value"], 64.0));
    }

    //  Test Case 7: Unknown block name
    {
        std::vector<hops::MHO_VexLine> lines = {
            MakeLine(1, 0, "$NOT_A_BLOCK"),
            MakeLine(2, 1, "def SOMETHING"),
            MakeLine(3, 2, "enddef"),
        };

        hops::MHO_VexBlockParser parser;
        parser.SetFormatDirectory(VEX_FORMAT_DIR "/vex-1.5/");
        auto j = parser.ParseBlockLines("$NOT_A_BLOCK", &lines);

        // No format file exists for $NOT_A_BLOCK, so parser returns empty JSON
        REQUIRE(j.empty());
    }

    //  Test Case 8: Literal line passthrough
    {
        std::vector<hops::MHO_VexLine> lines = {
            MakeLine(1, 0, "$FREQ"),
            MakeLine(2, 1, "def TESTFREQ"),
            MakeLine(3, 2, "This is a literal line", true),  // fIsLiteral = true
            MakeLine(4, 3, "sample_rate = 64.00 Ms/sec"),
            MakeLine(5, 4, "enddef"),
        };

        hops::MHO_VexBlockParser parser;
        parser.SetFormatDirectory(VEX_FORMAT_DIR "/vex-1.5/");
        auto j = parser.ParseBlockLines("$FREQ", &lines);

        // Literal lines are skipped during parsing (not stored in JSON).
        // The non-literal data should still be parsed correctly.
        REQUIRE(j.contains("TESTFREQ"));
        REQUIRE(json_double_close(j["TESTFREQ"]["sample_rate"]["value"], 64.0));

        // The literal line must NOT appear in the JSON output
        REQUIRE(j["TESTFREQ"].contains("This is a literal line") == false);
    }

    return 0;
}
