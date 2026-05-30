#include <iostream>
#include <string>
#include <vector>

#include "MHO_PolarizationRelabeler.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static multitone_pcal_type make_pcal(const std::vector<std::string>& pols,
                                     const std::string& mk4id,
                                     const std::string& code)
{
    multitone_pcal_type pc;
    pc.Resize(pols.size(), 2, 4);  // pol, time, freq
    auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
    for (std::size_t i = 0; i < pols.size(); ++i)
    {
        pax->at(i) = pols[i];
    }
    pc.Insert("station_mk4id", mk4id);
    pc.Insert("station_code", code);
    return pc;
}

// Case 1: Swap applies for matching mk4id (1-char)

static int test_swap_matching_mk4id()
{
    multitone_pcal_type pc = make_pcal(std::vector<std::string>{"X", "Y"}, "G", "Gs");

    MHO_PolarizationRelabeler<multitone_pcal_type> relabeler;
    relabeler.SetPolarizationSwapPair("X", "Y");
    relabeler.SetStationIdentifier("G");

    relabeler.SetArgs(&pc);
    relabeler.Initialize();
    relabeler.Execute();  // ExecuteInPlace always returns false -- don't check return

    auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
    REQUIRE(pax->at(0) == "Y");
    REQUIRE(pax->at(1) == "X");

    return 0;
}

// Case 2: Swap applies for matching 2-char code

static int test_swap_matching_2char_code()
{
    multitone_pcal_type pc = make_pcal(std::vector<std::string>{"X", "Y"}, "G", "Gs");

    MHO_PolarizationRelabeler<multitone_pcal_type> relabeler;
    relabeler.SetPolarizationSwapPair("X", "Y");
    relabeler.SetStationIdentifier("Gs");

    relabeler.SetArgs(&pc);
    relabeler.Initialize();
    relabeler.Execute();

    auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
    REQUIRE(pax->at(0) == "Y");
    REQUIRE(pax->at(1) == "X");

    return 0;
}

// Case 3: Non-matching station -> no change

static int test_nonmatching_station()
{
    multitone_pcal_type pc = make_pcal(std::vector<std::string>{"X", "Y"}, "G", "Gs");

    MHO_PolarizationRelabeler<multitone_pcal_type> relabeler;
    relabeler.SetPolarizationSwapPair("X", "Y");
    relabeler.SetStationIdentifier("E");  // different mk4id

    relabeler.SetArgs(&pc);
    relabeler.Initialize();
    relabeler.Execute();

    auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
    REQUIRE(pax->at(0) == "X");
    REQUIRE(pax->at(1) == "Y");

    return 0;
}

// Case 4: Wildcard identifiers

static int test_wildcard_identifiers()
{
    // Sub-case 4a: "?" matches ANY mk4id
    {
        multitone_pcal_type pc = make_pcal(std::vector<std::string>{"R", "L"}, "A", "Ab");

        MHO_PolarizationRelabeler<multitone_pcal_type> relabeler;
        relabeler.SetPolarizationSwapPair("R", "L");
        relabeler.SetStationIdentifier("?");

        relabeler.SetArgs(&pc);
        relabeler.Initialize();
        relabeler.Execute();

        auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
        REQUIRE(pax->at(0) == "L");
        REQUIRE(pax->at(1) == "R");
    }

    // Sub-case 4b: "??" matches ANY code
    {
        multitone_pcal_type pc = make_pcal(std::vector<std::string>{"R", "L"}, "B", "Bc");

        MHO_PolarizationRelabeler<multitone_pcal_type> relabeler;
        relabeler.SetPolarizationSwapPair("R", "L");
        relabeler.SetStationIdentifier("??");

        relabeler.SetArgs(&pc);
        relabeler.Initialize();
        relabeler.Execute();

        auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
        REQUIRE(pax->at(0) == "L");
        REQUIRE(pax->at(1) == "R");
    }

    return 0;
}

// Case 5: Labels outside the swap pair untouched

static int test_labels_outside_swap_pair()
{
    multitone_pcal_type pc = make_pcal(std::vector<std::string>{"X", "Y", "R"}, "G", "Gs");

    MHO_PolarizationRelabeler<multitone_pcal_type> relabeler;
    relabeler.SetPolarizationSwapPair("X", "Y");
    relabeler.SetStationIdentifier("G");

    relabeler.SetArgs(&pc);
    relabeler.Initialize();
    relabeler.Execute();

    auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
    REQUIRE(pax->at(0) == "Y");
    REQUIRE(pax->at(1) == "X");
    REQUIRE(pax->at(2) == "R");

    return 0;
}

// Case 6: Multi-character swap pair is rejected

static int test_multichar_swap_rejected()
{
    multitone_pcal_type pc = make_pcal(std::vector<std::string>{"X", "Y"}, "G", "Gs");

    MHO_PolarizationRelabeler<multitone_pcal_type> relabeler;
    relabeler.SetPolarizationSwapPair("XX", "YY");  // rejected -- sets fValid=false
    relabeler.SetStationIdentifier("G");

    relabeler.SetArgs(&pc);
    relabeler.Initialize();
    relabeler.Execute();

    auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
    REQUIRE(pax->at(0) == "X");  // unchanged because fPol1/fPol2 are empty
    REQUIRE(pax->at(1) == "Y");

    return 0;
}

// Case 7: SetStationIdentifiers (multiple) - match any in list

static int test_multiple_station_identifiers()
{
    multitone_pcal_type pc = make_pcal(std::vector<std::string>{"X", "Y"}, "G", "Gs");

    MHO_PolarizationRelabeler<multitone_pcal_type> relabeler;
    relabeler.SetPolarizationSwapPair("X", "Y");
    relabeler.SetStationIdentifiers(std::vector<std::string>{"E", "G"});

    relabeler.SetArgs(&pc);
    relabeler.Initialize();
    relabeler.Execute();

    auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
    REQUIRE(pax->at(0) == "Y");
    REQUIRE(pax->at(1) == "X");

    return 0;
}

// Case 8: Mixed-pol / linear-pol set

static int test_linear_pol_swap()
{
    multitone_pcal_type pc = make_pcal(std::vector<std::string>{"H", "V"}, "G", "Gs");

    MHO_PolarizationRelabeler<multitone_pcal_type> relabeler;
    relabeler.SetPolarizationSwapPair("H", "V");
    relabeler.SetStationIdentifier("G");

    relabeler.SetArgs(&pc);
    relabeler.Initialize();
    relabeler.Execute();

    auto pax = &(std::get<MTPCAL_POL_AXIS>(pc));
    REQUIRE(pax->at(0) == "V");
    REQUIRE(pax->at(1) == "H");

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_swap_matching_mk4id())           return 1;
    if (test_swap_matching_2char_code())      return 1;
    if (test_nonmatching_station())           return 1;
    if (test_wildcard_identifiers())          return 1;
    if (test_labels_outside_swap_pair())      return 1;
    if (test_multichar_swap_rejected())       return 1;
    if (test_multiple_station_identifiers())  return 1;
    if (test_linear_pol_swap())               return 1;

    return 0;
}
