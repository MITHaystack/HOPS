#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

#include "MHO_Message.hh"
#include "MHO_StationCodeMap.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eError);

    //-- Create the temp fixture file for Cases 6 and 7--
    const char* tmpfile = "TestStationCodeMap_tmp.codes";
    {
        std::ofstream ofs(tmpfile);
        ofs << "E Wf\n";
        ofs << "G Gs\n";
        ofs << "X On\n";
        ofs << "baddata\n";
        ofs << "E Dup\n";
        ofs << "Q on\n";
    }

    // Case 1: Empty map- unknown mk4id returns empty string
    {
        MHO_StationCodeMap m(false);
        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("Z"), "");
    }

    // Case 2: Auto-assign on first unknown code, from free pool (alphabetical, A first)
    {
        MHO_StationCodeMap m(false);
        std::string id1 = m.GetMk4IdFromStationCode("Bd");
        std::string id2 = m.GetMk4IdFromStationCode("Cm");
        std::string id1b = m.GetMk4IdFromStationCode("Bd");
        REQUIRE_EQUAL(id1, "A");
        REQUIRE_EQUAL(id2, "B");
        REQUIRE_EQUAL(id1b, "A");
    }

    // Case 3: Auto-assigned mapping is reversible
    {
        MHO_StationCodeMap m(false);
        m.GetMk4IdFromStationCode("Bd");
        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("A"), "Bd");
    }

    // Case 4: Canonical-case normalization of 2-char codes
    {
        MHO_StationCodeMap m(false);
        std::string idLower = m.GetMk4IdFromStationCode("wf");
        std::string idUpper = m.GetMk4IdFromStationCode("WF");
        std::string idCanon = m.GetMk4IdFromStationCode("Wf");
        REQUIRE_EQUAL(idLower, "A");
        REQUIRE_EQUAL(idUpper, "A");
        REQUIRE_EQUAL(idCanon, "A");
        // Only one pool slot consumed; a 4th distinct code gets "B"
        std::string idXy = m.GetMk4IdFromStationCode("Xy");
        REQUIRE_EQUAL(idXy, "B");
    }

    // Case 5: Wrong-length code returns empty (no assignment)
    {
        MHO_StationCodeMap m(false);
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("A"), "");
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("ABC"), "");
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode(""), "");
        // No pool slot consumed; subsequent valid "Wf" still gets "A"
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("Wf"), "A");
    }

    // Case 6: File initialization- valid pairs loaded
    {
        MHO_StationCodeMap m(false);
        m.InitializeStationCodes(tmpfile);
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("Wf"), "E");
        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("G"), "Gs");
        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("X"), "On");
    }

    // Case 7: File initialization- malformed and duplicate lines are ignored
    {
        MHO_StationCodeMap m(false);
        m.InitializeStationCodes(tmpfile);

        // "On" stays mapped to "X" (the "Q on" duplicate did NOT remap)
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("On"), "X");

        // "Dup" should not be present in GetAllStationCodes
        std::vector<std::string> codes = m.GetAllStationCodes();
        bool foundDup = false;
        for(std::size_t i = 0; i < codes.size(); i++) {
            if(codes[i] == "Dup") { foundDup = true; break; }
        }
        REQUIRE(!foundDup);

        // Exactly 3 valid pairs loaded from file (Wf->E, Gs->G, On->X)
        REQUIRE(codes.size() == 3);
    }

    // Case 8: Round-trip over all loaded mk4ids
    {
        MHO_StationCodeMap m(false);
        m.InitializeStationCodes(tmpfile);
        std::vector<std::string> mk4ids = m.GetAllMk4Ids();
        for(std::size_t i = 0; i < mk4ids.size(); i++) {
            std::string code = m.GetStationCodeFromMk4Id(mk4ids[i]);
            REQUIRE(!code.empty());
            std::string id2 = m.GetMk4IdFromStationCode(code);
            REQUIRE_EQUAL(id2, mk4ids[i]);
        }
    }

    // Case 9: Legacy default table golden pairs
    {
        MHO_StationCodeMap m(true);
        m.InitializeStationCodes("");

        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("A"), "Ai");
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("Ai"), "A");

        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("B"), "Bd");
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("Bd"), "B");

        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("F"), "Eb");
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("Eb"), "F");

        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("H"), "Ho");
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("Ho"), "H");

        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("K"), "Kk");
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("Kk"), "K");

        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("X"), "On");
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("On"), "X");

        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("Y"), "Yb");
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("Yb"), "Y");
    }

    // Case 10: Free-pool exhaustion (52 distinct codes, then 53rd returns empty)
    {
        MHO_StationCodeMap m(false);
        std::vector<std::string> ids;
        for(int i = 0; i < 52; i++) {
            std::string code(2, ' ');
            code[0] = static_cast<char>('A' + (i / 26));
            code[1] = static_cast<char>('a' + (i % 26));
            std::string id = m.GetMk4IdFromStationCode(code);
            REQUIRE(!id.empty());
            ids.push_back(id);
        }
        // All 52 ids must be distinct
        for(int i = 0; i < 52; i++) {
            for(int j = i + 1; j < 52; j++) {
                REQUIRE(ids[i] != ids[j]);
            }
        }
        // 53rd code- pool exhausted
        REQUIRE_EQUAL(m.GetMk4IdFromStationCode("Zz"), "");
    }

    // Case 11: Re-initialization clears prior state
    {
        MHO_StationCodeMap m(false);
        m.GetMk4IdFromStationCode("Wf"); // assigns "A"
        m.InitializeStationCodes("");    // clears everything
        REQUIRE_EQUAL(m.GetStationCodeFromMk4Id("A"), "");
    }

    // Case 12: ToUpperCase -- alpha chars upcased, non-alpha preserved
    {
        MHO_StationCodeMap m(false);
        REQUIRE_EQUAL(m.ToUpperCase("wf"), "WF");
        REQUIRE_EQUAL(m.ToUpperCase("Wf"), "WF"); // idempotent on already-upper
        REQUIRE_EQUAL(m.ToUpperCase("WF"), "WF");
        REQUIRE_EQUAL(m.ToUpperCase("a1-b"), "A1-B"); // digits/punctuation untouched
        REQUIRE_EQUAL(m.ToUpperCase(""), "");
    }

    // Case 13: ToLowerCase -- alpha chars downcased, non-alpha preserved
    {
        MHO_StationCodeMap m(false);
        REQUIRE_EQUAL(m.ToLowerCase("WF"), "wf");
        REQUIRE_EQUAL(m.ToLowerCase("Wf"), "wf"); // idempotent on already-lower
        REQUIRE_EQUAL(m.ToLowerCase("wf"), "wf");
        REQUIRE_EQUAL(m.ToLowerCase("A1-B"), "a1-b"); // digits/punctuation untouched
        REQUIRE_EQUAL(m.ToLowerCase(""), "");
    }

    // Case 14: ToCanonicalCase -- first char upper, remainder lower, non-alpha preserved
    {
        MHO_StationCodeMap m(false);
        REQUIRE_EQUAL(m.ToCanonicalCase("wf"), "Wf");
        REQUIRE_EQUAL(m.ToCanonicalCase("WF"), "Wf");
        REQUIRE_EQUAL(m.ToCanonicalCase("wF"), "Wf");
        REQUIRE_EQUAL(m.ToCanonicalCase("Wf"), "Wf"); // idempotent on canonical input
        REQUIRE_EQUAL(m.ToCanonicalCase("g"), "G");   // single char
        REQUIRE_EQUAL(m.ToCanonicalCase("g2"), "G2");  // alpha then non-alpha
        REQUIRE_EQUAL(m.ToCanonicalCase("2g"), "2g");  // leading non-alpha preserved, rest lower
        REQUIRE_EQUAL(m.ToCanonicalCase(""), "");      // empty input handled by guard
    }

    // Cleanup
    std::remove(tmpfile);

    return 0;
}
