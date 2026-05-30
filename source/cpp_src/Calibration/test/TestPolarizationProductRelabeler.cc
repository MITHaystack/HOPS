#include <iostream>
#include <string>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_PolarizationProductRelabeler.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

static visibility_type make_vis(const std::vector<std::string>& pprods,
                                const std::string& ref_mk4, const std::string& ref_code,
                                const std::string& rem_mk4, const std::string& rem_code)
{
    visibility_type vis;
    vis.Resize(pprods.size(), 4, 4, 4);
    auto pax = &(std::get<POLPROD_AXIS>(vis));
    for (std::size_t i = 0; i < pprods.size(); ++i)
        pax->at(i) = pprods[i];
    vis.Insert("reference_station_mk4id", ref_mk4);
    vis.Insert("reference_station",       ref_code);
    vis.Insert("remote_station_mk4id",    rem_mk4);
    vis.Insert("remote_station",          rem_code);
    return vis;
}

static void run_relabeler(MHO_PolarizationProductRelabeler<visibility_type>& rel,
                          visibility_type* vis)
{
    rel.SetArgs(vis);
    rel.Initialize();
    // ExecuteInPlace always returns false even on success - assert on contents, not return value
    rel.Execute();
}

// Case 1 - Reference-station swap only mutates char[0]

static int test_ref_station_swap()
{
    visibility_type vis = make_vis({"XX","XY","YX","YY"}, "G","Gs","E","Es");

    MHO_PolarizationProductRelabeler<visibility_type> rel;
    rel.SetPolarizationSwapPair("X", "Y");
    rel.SetStationIdentifier("G");  // matches ref only
    run_relabeler(rel, &vis);

    auto pax = &(std::get<POLPROD_AXIS>(vis));
    REQUIRE(pax->at(0) == "YX");  // XX -> YX (char[0] X->Y)
    REQUIRE(pax->at(1) == "YY");  // XY -> YY (char[0] X->Y)
    REQUIRE(pax->at(2) == "XX");  // YX -> XX (char[0] Y->X)
    REQUIRE(pax->at(3) == "XY");  // YY -> XY (char[0] Y->X)

    return 0;
}

// Case 2 - Remote-station swap only mutates char[1]

static int test_rem_station_swap()
{
    visibility_type vis = make_vis({"XX","XY","YX","YY"}, "G","Gs","E","Es");

    MHO_PolarizationProductRelabeler<visibility_type> rel;
    rel.SetPolarizationSwapPair("X", "Y");
    rel.SetStationIdentifier("E");  // matches rem only
    run_relabeler(rel, &vis);

    auto pax = &(std::get<POLPROD_AXIS>(vis));
    REQUIRE(pax->at(0) == "XY");  // XX -> XY (char[1] X->Y)
    REQUIRE(pax->at(1) == "XX");  // XY -> XX (char[1] Y->X)
    REQUIRE(pax->at(2) == "YY");  // YX -> YY (char[1] X->Y)
    REQUIRE(pax->at(3) == "YX");  // YY -> YX (char[1] Y->X)

    return 0;
}

// Case 3 - Both stations match -> both characters swapped

static int test_both_stations_swap()
{
    visibility_type vis = make_vis({"XX","XY","YX","YY"}, "G","Gs","G","Gs");

    MHO_PolarizationProductRelabeler<visibility_type> rel;
    rel.SetPolarizationSwapPair("X", "Y");
    rel.SetStationIdentifier("G");  // matches both ref and rem (same mk4id)
    run_relabeler(rel, &vis);

    auto pax = &(std::get<POLPROD_AXIS>(vis));
    REQUIRE(pax->at(0) == "YY");  // XX -> YY (both chars X->Y)
    REQUIRE(pax->at(1) == "YX");  // XY -> YX (char[0] X->Y, char[1] Y->X)
    REQUIRE(pax->at(2) == "XY");  // YX -> XY (char[0] Y->X, char[1] X->Y)
    REQUIRE(pax->at(3) == "XX");  // YY -> XX (both chars Y->X)

    return 0;
}

// Case 4 - No station matches -> unchanged

static int test_no_match()
{
    visibility_type vis = make_vis({"XX","XY","YX","YY"}, "G","Gs","E","Es");

    MHO_PolarizationProductRelabeler<visibility_type> rel;
    rel.SetPolarizationSwapPair("X", "Y");
    rel.SetStationIdentifier("Q");  // matches neither station
    run_relabeler(rel, &vis);

    auto pax = &(std::get<POLPROD_AXIS>(vis));
    REQUIRE(pax->at(0) == "XX");
    REQUIRE(pax->at(1) == "XY");
    REQUIRE(pax->at(2) == "YX");
    REQUIRE(pax->at(3) == "YY");

    return 0;
}

// Case 5 - Wildcards

static int test_wildcards()
{
    // 5a: "?" matches both stations' mk4id (1-char wildcard)
    {
        visibility_type vis = make_vis({"XX","XY","YX","YY"}, "G","Gs","E","Es");

        MHO_PolarizationProductRelabeler<visibility_type> rel;
        rel.SetPolarizationSwapPair("X", "Y");
        rel.SetStationIdentifier("?");
        run_relabeler(rel, &vis);

        auto pax = &(std::get<POLPROD_AXIS>(vis));
        REQUIRE(pax->at(0) == "YY");
        REQUIRE(pax->at(1) == "YX");
        REQUIRE(pax->at(2) == "XY");
        REQUIRE(pax->at(3) == "XX");
    }

    // 5b: "??" matches both stations via station code (2-char wildcard)
    {
        visibility_type vis = make_vis({"XX","XY","YX","YY"}, "G","Gs","E","Es");

        MHO_PolarizationProductRelabeler<visibility_type> rel;
        rel.SetPolarizationSwapPair("X", "Y");
        rel.SetStationIdentifier("??");
        run_relabeler(rel, &vis);

        auto pax = &(std::get<POLPROD_AXIS>(vis));
        REQUIRE(pax->at(0) == "YY");
        REQUIRE(pax->at(1) == "YX");
        REQUIRE(pax->at(2) == "XY");
        REQUIRE(pax->at(3) == "XX");
    }

    return 0;
}

// Case 6 - Characters outside the swap pair untouched

static int test_chars_outside_swap()
{
    // Sub-case: swap X<->Y on circular-pol products - nothing matches
    {
        visibility_type vis = make_vis({"RL","LR","RR"}, "G","Gs","E","Es");

        MHO_PolarizationProductRelabeler<visibility_type> rel;
        rel.SetPolarizationSwapPair("X", "Y");
        rel.SetStationIdentifier("G");  // ref match
        run_relabeler(rel, &vis);

        auto pax = &(std::get<POLPROD_AXIS>(vis));
        REQUIRE(pax->at(0) == "RL");
        REQUIRE(pax->at(1) == "LR");
        REQUIRE(pax->at(2) == "RR");
    }

    // Variant: swap R<->L on REF only -> char[0] swapped
    {
        visibility_type vis = make_vis({"RL","LR","RR"}, "G","Gs","E","Es");

        MHO_PolarizationProductRelabeler<visibility_type> rel;
        rel.SetPolarizationSwapPair("R", "L");
        rel.SetStationIdentifier("G");  // ref match only
        run_relabeler(rel, &vis);

        auto pax = &(std::get<POLPROD_AXIS>(vis));
        REQUIRE(pax->at(0) == "LL");  // RL -> LL (char[0] R->L)
        REQUIRE(pax->at(1) == "RR");  // LR -> RR (char[0] L->R)
        REQUIRE(pax->at(2) == "LR");  // RR -> LR (char[0] R->L)
    }

    return 0;
}

// Case 7 - Multi-character swap pair rejected

static int test_multi_char_swap_rejected()
{
    visibility_type vis = make_vis({"XX","XY"}, "G","Gs","E","Es");

    MHO_PolarizationProductRelabeler<visibility_type> rel;
    rel.SetPolarizationSwapPair("XX", "YY");  // invalid - multi-char
    rel.SetStationIdentifier("G");
    run_relabeler(rel, &vis);

    // fPol1/fPol2 remain empty; operator[](0) returns '\0' which won't match
    // any real pol character, so strings are unchanged.
    auto pax = &(std::get<POLPROD_AXIS>(vis));
    REQUIRE(pax->at(0) == "XX");
    REQUIRE(pax->at(1) == "XY");

    return 0;
}

// Case 8 - Mixed-pol partial set

static int test_mixed_pol_partial()
{
    visibility_type vis = make_vis({"XY","YX"}, "G","Gs","E","Es");

    MHO_PolarizationProductRelabeler<visibility_type> rel;
    rel.SetPolarizationSwapPair("X", "Y");
    rel.SetStationIdentifier("G");  // ref match -> char[0] only
    run_relabeler(rel, &vis);

    auto pax = &(std::get<POLPROD_AXIS>(vis));
    REQUIRE(pax->at(0) == "YY");  // XY -> YY (char[0] X->Y)
    REQUIRE(pax->at(1) == "XX");  // YX -> XX (char[0] Y->X)

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_ref_station_swap())       return 1;
    if (test_rem_station_swap())       return 1;
    if (test_both_stations_swap())     return 1;
    if (test_no_match())               return 1;
    if (test_wildcards())              return 1;
    if (test_chars_outside_swap())     return 1;
    if (test_multi_char_swap_rejected()) return 1;
    if (test_mixed_pol_partial())      return 1;

    return 0;
}
