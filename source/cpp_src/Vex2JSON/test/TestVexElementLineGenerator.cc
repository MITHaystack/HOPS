#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_VexElementLineGenerator.hh"
#include "MHO_VexTokenProcessor.hh"
#include "MHO_TestAssertions.hh"


static std::string Trim(std::string s)
{
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
    std::size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    return s.substr(i);
}


int main()
{
    hops::MHO_Message::GetInstance().AcceptAllKeys();
    hops::MHO_Message::GetInstance().SetMessageLevel(hops::eFatal);


    // Case 1: GenerateInt
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json obj = 42;
        REQUIRE(Trim(gen.GenerateInt("exper_num", obj)) == "42");
    }


    // Case 2: GenerateListInt
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json obj;
        obj.push_back(1);
        obj.push_back(2);
        obj.push_back(3);
        std::string s = gen.GenerateListInt("freq_state", obj);
        REQUIRE(s.find("1") != std::string::npos);
        REQUIRE(s.find("2") != std::string::npos);
        REQUIRE(s.find("3") != std::string::npos);
    }


    // Case 3: GenerateReal with units
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json obj = mho_json::object();
        obj["value"] = 8080.40;
        obj["units"] = "MHz";
        std::string s = gen.GenerateReal("sky_frequency", obj);
        REQUIRE(s.find("8080.4") != std::string::npos);
        REQUIRE(s.find("MHz") != std::string::npos);
    }


    // Case 4: GenerateReal without units
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json obj = mho_json::object();
        obj["value"] = 3.14;
        std::string s = gen.GenerateReal("x", obj);
        REQUIRE(s.find("3.14") != std::string::npos);
        // no spurious units
        REQUIRE(s.find("MHz") == std::string::npos);
        REQUIRE(s.find("sec") == std::string::npos);
    }


    // Case 5: GenerateListReal
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json values;
        values.push_back(1.0);
        values.push_back(2.0);
        values.push_back(3.0);
        mho_json obj = mho_json::object();
        obj["values"] = values;
        obj["units"] = "s";
        std::string s = gen.GenerateListReal("state_periods", obj);
        REQUIRE(s.find("1") != std::string::npos);
        REQUIRE(s.find("s") != std::string::npos);
    }


    // Case 6: GenerateString
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json obj = "U";
        REQUIRE(Trim(gen.GenerateString("net_sideband", obj)) == "U");
    }


    // Case 7: GenerateListString
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json obj;
        obj.push_back("Aa");
        obj.push_back("Bb");
        obj.push_back("Cc");
        std::string s = gen.GenerateListString("stations", obj);
        REQUIRE(s.find("Aa") != std::string::npos);
        REQUIRE(s.find("Bb") != std::string::npos);
        REQUIRE(s.find("Cc") != std::string::npos);
    }


    // Case 8: GenerateKeyword
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json obj = "TESTFREQ";
        REQUIRE(gen.GenerateKeyword("keyword", obj).find("TESTFREQ") != std::string::npos);
    }


    // Case 9: GenerateLink
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json obj = "&CH01";
        REQUIRE(gen.GenerateLink("channel_id", obj).find("&CH01") != std::string::npos);
    }


    // Case 10: ConstructElementLine (compound chan_def)
    {
        hops::MHO_VexElementLineGenerator gen;
        // Format: list_compound with fields
        mho_json fmt = mho_json::object();
        fmt["type"] = "list_compound";
        fmt["fields"] = mho_json::array();
        fmt["fields"].push_back("band_id");
        fmt["fields"].push_back("sky_frequency");
        fmt["fields"].push_back("net_sideband");
        fmt["fields"].push_back("bandwidth");
        fmt["fields"].push_back("channel_id");
        fmt["fields"].push_back("bbc_id");
        fmt["fields"].push_back("phase_cal_id");

        // Each field needs a type in parameters
        mho_json params = mho_json::object();
        mho_json p_band_id = mho_json::object();
        p_band_id["type"] = "link";
        params["band_id"] = p_band_id;

        mho_json p_sky_freq = mho_json::object();
        p_sky_freq["type"] = "real";
        p_sky_freq["dimension"] = "frequency";
        params["sky_frequency"] = p_sky_freq;

        mho_json p_sideband = mho_json::object();
        p_sideband["type"] = "string";
        params["net_sideband"] = p_sideband;

        mho_json p_bw = mho_json::object();
        p_bw["type"] = "real";
        p_bw["dimension"] = "frequency";
        params["bandwidth"] = p_bw;

        mho_json p_chan = mho_json::object();
        p_chan["type"] = "link";
        params["channel_id"] = p_chan;

        mho_json p_bbc = mho_json::object();
        p_bbc["type"] = "link";
        params["bbc_id"] = p_bbc;

        mho_json p_cal = mho_json::object();
        p_cal["type"] = "link";
        params["phase_cal_id"] = p_cal;

        fmt["parameters"] = params;

        // Element
        mho_json elem = mho_json::object();
        elem["band_id"] = "&X";
        mho_json sky_freq = mho_json::object();
        sky_freq["value"] = 8080.40;
        sky_freq["units"] = "MHz";
        elem["sky_frequency"] = sky_freq;
        elem["net_sideband"] = "U";
        mho_json bw = mho_json::object();
        bw["value"] = 32.0;
        bw["units"] = "MHz";
        elem["bandwidth"] = bw;
        elem["channel_id"] = "&CH01";
        elem["bbc_id"] = "&BBC01";
        elem["phase_cal_id"] = "&U_Cal";

        std::string line = gen.ConstructElementLine("chan_def", elem, fmt);
        // Verify it contains all the key tokens
        REQUIRE(line.find("&X") != std::string::npos);
        REQUIRE(line.find("8080.4") != std::string::npos);
        REQUIRE(line.find("MHz") != std::string::npos);
        REQUIRE(line.find("&CH01") != std::string::npos);
        REQUIRE(line.find("&BBC01") != std::string::npos);
        REQUIRE(line.find("&U_Cal") != std::string::npos);
    }


    // Case 11: Round-trip GenerateReal -> ProcessReal
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json obj = mho_json::object();
        obj["value"] = 8080.40;
        obj["units"] = "MHz";
        std::string line = gen.GenerateReal("sky_frequency", obj);
        // Tokenize the output using SplitString
        std::vector<std::string> tokens = hops::SplitString(line, " ");
        // Construct a single token from the non-empty tokens
        std::string combined;
        for (std::size_t i = 0; i < tokens.size(); ++i) {
            if (!tokens[i].empty()) {
                if (!combined.empty()) combined += " ";
                combined += tokens[i];
            }
        }
        std::vector<std::string> proc_tokens = {combined};
        hops::MHO_VexTokenProcessor tp;
        mho_json fmt = mho_json::object();
        auto result = tp.ProcessReal("sky_frequency", fmt, proc_tokens);
        double recovered = (double)result["value"];
        REQUIRE(std::fabs(recovered - 8080.40) < 1e-9);
    }


    // Case 12: Trailing optional field
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json fmt = mho_json::object();
        fmt["type"] = "list_compound";
        fmt["fields"] = mho_json::array();
        fmt["fields"].push_back("band_id");
        fmt["fields"].push_back("sky_frequency");
        fmt["fields"].push_back("net_sideband");
        fmt["fields"].push_back("!freq_state");

        mho_json params = mho_json::object();
        mho_json p_band = mho_json::object();
        p_band["type"] = "link";
        params["band_id"] = p_band;

        mho_json p_freq = mho_json::object();
        p_freq["type"] = "real";
        p_freq["dimension"] = "frequency";
        params["sky_frequency"] = p_freq;

        mho_json p_sb = mho_json::object();
        p_sb["type"] = "string";
        params["net_sideband"] = p_sb;

        mho_json p_fs = mho_json::object();
        p_fs["type"] = "list_int";
        params["freq_state"] = p_fs;

        fmt["parameters"] = params;

        // Element missing freq_state
        mho_json elem = mho_json::object();
        elem["band_id"] = "&X";
        mho_json sf = mho_json::object();
        sf["value"] = 8080.40;
        sf["units"] = "MHz";
        elem["sky_frequency"] = sf;
        elem["net_sideband"] = "U";

        std::string line = gen.ConstructElementLine("chan_def", elem, fmt);
        // Trim and verify no trailing colons
        std::string trimmed = Trim(line);
        // Should not end with ":"
        REQUIRE(trimmed.back() != ':');
        // Should contain the fields that are present
        REQUIRE(line.find("&X") != std::string::npos);
    }


    // Case 13: Interior optional gap must keep placeholders (regression)
    // clock_early with only clock_early_offset (#2) and fmout2gps (#7) populated.
    // The empty interior optional fields must still emit positional placeholders so
    // fmout2gps is not shifted into an earlier slot (clock_rate) on re-parse.
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json fmt = mho_json::object();
        fmt["type"] = "list_compound";
        fmt["fields"] = mho_json::array();
        fmt["fields"].push_back("!start_validity_epoch");
        fmt["fields"].push_back("clock_early_offset");
        fmt["fields"].push_back("!origin_epoch");
        fmt["fields"].push_back("!clock_rate");
        fmt["fields"].push_back("!second_order_coeff");
        fmt["fields"].push_back("!third_order_coeff");
        fmt["fields"].push_back("!fmout2gps");

        mho_json params = mho_json::object();
        mho_json p_epoch = mho_json::object();
        p_epoch["type"] = "epoch";
        params["start_validity_epoch"] = p_epoch;
        params["origin_epoch"] = p_epoch;

        mho_json p_real = mho_json::object();
        p_real["type"] = "real";
        p_real["dimension"] = "time";
        params["clock_early_offset"] = p_real;
        params["clock_rate"] = p_real;
        params["second_order_coeff"] = p_real;
        params["third_order_coeff"] = p_real;
        params["fmout2gps"] = p_real;
        fmt["parameters"] = params;

        mho_json elem = mho_json::object();
        mho_json offset = mho_json::object();
        offset["value"] = 2.5;
        offset["units"] = "usec";
        elem["clock_early_offset"] = offset;
        mho_json fmout = mho_json::object();
        fmout["value"] = 12.0;
        fmout["units"] = "sec";
        elem["fmout2gps"] = fmout;

        std::string line = gen.ConstructElementLine("clock_early", elem, fmt);
        // 7 positional fields -> 6 element delimiters (':') joining them.
        // Before the fix the interior placeholders were dropped, yielding only 2.
        REQUIRE(std::count(line.begin(), line.end(), ':') == 6);
        // Both populated values must still be present.
        REQUIRE(line.find("2.5") != std::string::npos);
        REQUIRE(line.find("12") != std::string::npos);
        // The offset must come before fmout2gps in the line.
        REQUIRE(line.find("2.5") < line.find("12"));
    }


    // Case 14: Interior gap in the middle of populated fields ($SCHED station)
    // pass (#5) is missing but media_position (#4), wrap_id (#6) and record_flag (#7)
    // are present, so the missing pass must still produce a placeholder.
    {
        hops::MHO_VexElementLineGenerator gen;
        mho_json fmt = mho_json::object();
        fmt["type"] = "list_compound";
        fmt["fields"] = mho_json::array();
        fmt["fields"].push_back("station");
        fmt["fields"].push_back("data_good");
        fmt["fields"].push_back("data_stop");
        fmt["fields"].push_back("!media_position");
        fmt["fields"].push_back("!pass");
        fmt["fields"].push_back("!wrap_id");
        fmt["fields"].push_back("!record_flag");

        mho_json params = mho_json::object();
        mho_json p_link = mho_json::object();
        p_link["type"] = "link";
        params["station"] = p_link;
        params["pass"] = p_link;
        params["wrap_id"] = p_link;

        mho_json p_real = mho_json::object();
        p_real["type"] = "real";
        p_real["dimension"] = "time";
        params["data_good"] = p_real;
        params["data_stop"] = p_real;
        params["media_position"] = p_real;

        mho_json p_int = mho_json::object();
        p_int["type"] = "int";
        params["record_flag"] = p_int;
        fmt["parameters"] = params;

        mho_json elem = mho_json::object();
        elem["station"] = "&EF";
        mho_json dg = mho_json::object();
        dg["value"] = 0.0;
        dg["units"] = "sec";
        elem["data_good"] = dg;
        mho_json ds = mho_json::object();
        ds["value"] = 180.0;
        ds["units"] = "sec";
        elem["data_stop"] = ds;
        mho_json mp = mho_json::object();
        mp["value"] = 0.0;
        mp["units"] = "ft";
        elem["media_position"] = mp;
        elem["wrap_id"] = "&n";
        elem["record_flag"] = 1;

        std::string line = gen.ConstructElementLine("station", elem, fmt);
        // 7 positional fields -> 6 delimiters; the missing 'pass' keeps its placeholder
        // so wrap_id (&n) and record_flag (1) stay in their correct slots.
        REQUIRE(std::count(line.begin(), line.end(), ':') == 6);
        REQUIRE(line.find("&n") != std::string::npos);
    }

    return 0;
}
