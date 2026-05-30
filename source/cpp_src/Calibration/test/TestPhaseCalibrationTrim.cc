#include <cmath>
#include <complex>
#include <iostream>
#include <string>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_PhaseCalibrationTrim.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* CASE 1: No-op when start + AP count + delta coincide.
   Pcal with N=4 APs, identical start tag, AP=1.0 (centroids at 0.5,1.5,2.5,3.5).
   start_tdiff = -pcal_tdelta/2 = -0.5; |-0.5| < 0.9*1.0 -> coincident. */
static int test_case1_noop()
{
    // Build visibility: 4 APs with edges at 0,1,2,3 (AP=1.0)
    visibility_type vis;
    vis.Resize(1, 1, 4, 1);
    auto& vtime = std::get<TIME_AXIS>(vis);
    for (std::size_t t = 0; t < 4; t++) vtime.at(t) = static_cast<double>(t) * 1.0;
    vis.Insert(std::string("start"), std::string("2024y001d00h00m00s"));

    // Build pcal: 1 pol, 4 APs (centroids), 2 freq tones
    multitone_pcal_type pc;
    pc.Resize(1, 4, 2);
    auto& ptime = std::get<MTPCAL_TIME_AXIS>(pc);
    for (std::size_t k = 0; k < 4; k++)
        ptime.at(k) = static_cast<double>(k) * 1.0 + 0.5;
    pc.Insert(std::string("start"), std::string("2024y001d00h00m00s"));
    pc.Insert(std::string("station_code"), std::string("Gs"));

    for (std::size_t k = 0; k < 4; k++)
        for (std::size_t f = 0; f < 2; f++)
            pc(0, k, f) = std::complex<double>(static_cast<double>(k), static_cast<double>(f));

    MHO_PhaseCalibrationTrim op;
    op.SetVisibilities(&vis);
    op.SetArgs(&pc);
    op.Initialize();
    bool ok = op.Execute();
    REQUIRE(ok);

    // Size unchanged
    REQUIRE(pc.GetDimension(MTPCAL_TIME_AXIS) == 4);

    // Data unchanged
    for (std::size_t k = 0; k < 4; k++)
        for (std::size_t f = 0; f < 2; f++)
        {
            CHECK_CLOSE(pc(0, k, f).real(), static_cast<double>(k), 1e-12);
            CHECK_CLOSE(pc(0, k, f).imag(), static_cast<double>(f), 1e-12);
        }

    return 0;
}

/* CASE 2: Trim trailing APs (more pcal than vis).
   Pcal with 6 APs (centroids 0.5..5.5), vis with 4 APs spanning [0,4).
   Centroids 0.5,1.5,2.5,3.5 in range; 4.5,5.5 out -> 4 APs retained. */
static int test_case2_trim_trailing()
{
    visibility_type vis;
    vis.Resize(1, 1, 4, 1);
    auto& vtime = std::get<TIME_AXIS>(vis);
    for (std::size_t t = 0; t < 4; t++) vtime.at(t) = static_cast<double>(t) * 1.0;
    vis.Insert(std::string("start"), std::string("2024y001d00h00m00s"));

    multitone_pcal_type pc;
    pc.Resize(1, 6, 2);
    auto& ptime = std::get<MTPCAL_TIME_AXIS>(pc);
    for (std::size_t k = 0; k < 6; k++)
        ptime.at(k) = static_cast<double>(k) * 1.0 + 0.5;
    pc.Insert(std::string("start"), std::string("2024y001d00h00m00s"));
    pc.Insert(std::string("station_code"), std::string("Gs"));

    for (std::size_t k = 0; k < 6; k++)
        for (std::size_t f = 0; f < 2; f++)
            pc(0, k, f) = std::complex<double>(static_cast<double>(k), static_cast<double>(f));

    MHO_PhaseCalibrationTrim op;
    op.SetVisibilities(&vis);
    op.SetArgs(&pc);
    op.Initialize();
    bool ok = op.Execute();
    REQUIRE(ok);

    REQUIRE(pc.GetDimension(MTPCAL_TIME_AXIS) == 4);

    // Surviving real parts == {0,1,2,3}
    for (std::size_t k = 0; k < 4; k++)
    {
        CHECK_CLOSE(pc(0, k, 0).real(), static_cast<double>(k), 1e-12);
        CHECK_CLOSE(pc(0, k, 1).real(), static_cast<double>(k), 1e-12);
    }

    return 0;
}

/* CASE 3: Trim leading APs (pcal starts earlier).
   Pcal centroids: -1.5,-0.5,0.5,1.5,2.5,3.5 (6 APs).
   Vis span [0,4). Centroids -1.5,-0.5 dropped; 0.5,1.5,2.5,3.5 kept. */
static int test_case3_trim_leading()
{
    visibility_type vis;
    vis.Resize(1, 1, 4, 1);
    auto& vtime = std::get<TIME_AXIS>(vis);
    for (std::size_t t = 0; t < 4; t++) vtime.at(t) = static_cast<double>(t) * 1.0;
    vis.Insert(std::string("start"), std::string("2024y001d00h00m00s"));

    multitone_pcal_type pc;
    pc.Resize(1, 6, 2);
    auto& ptime = std::get<MTPCAL_TIME_AXIS>(pc);
    for (std::size_t k = 0; k < 6; k++)
        ptime.at(k) = (static_cast<double>(k) - 2.0) * 1.0 + 0.5;
    pc.Insert(std::string("start"), std::string("2024y001d00h00m00s"));
    pc.Insert(std::string("station_code"), std::string("Gs"));

    for (std::size_t k = 0; k < 6; k++)
        for (std::size_t f = 0; f < 2; f++)
            pc(0, k, f) = std::complex<double>(static_cast<double>(k), static_cast<double>(f));

    MHO_PhaseCalibrationTrim op;
    op.SetVisibilities(&vis);
    op.SetArgs(&pc);
    op.Initialize();
    bool ok = op.Execute();
    REQUIRE(ok);

    REQUIRE(pc.GetDimension(MTPCAL_TIME_AXIS) == 4);

    // First surviving AP should be original k=2 (real part == 2.0)
    CHECK_CLOSE(pc(0, 0, 0).real(), 2.0, 1e-12);
    CHECK_CLOSE(pc(0, 1, 0).real(), 3.0, 1e-12);
    CHECK_CLOSE(pc(0, 2, 0).real(), 4.0, 1e-12);
    CHECK_CLOSE(pc(0, 3, 0).real(), 5.0, 1e-12);

    return 0;
}

/* CASE 4: AP-delta mismatch -> false.
   Pcal AP delta 2.0 vs vis AP delta 1.0. |2.0-1.0| = 1.0 > fAPEps(0.01). */
static int test_case4_ap_delta_mismatch()
{
    visibility_type vis;
    vis.Resize(1, 1, 4, 1);
    auto& vtime = std::get<TIME_AXIS>(vis);
    for (std::size_t t = 0; t < 4; t++) vtime.at(t) = static_cast<double>(t) * 1.0;
    vis.Insert(std::string("start"), std::string("2024y001d00h00m00s"));

    multitone_pcal_type pc;
    pc.Resize(1, 4, 2);
    auto& ptime = std::get<MTPCAL_TIME_AXIS>(pc);
    // AP delta = 2.0 (centroids at 0.5, 2.5, 4.5, 6.5)
    for (std::size_t k = 0; k < 4; k++)
        ptime.at(k) = static_cast<double>(k) * 2.0 + 0.5;
    pc.Insert(std::string("start"), std::string("2024y001d00h00m00s"));
    pc.Insert(std::string("station_code"), std::string("Gs"));

    for (std::size_t k = 0; k < 4; k++)
        for (std::size_t f = 0; f < 2; f++)
            pc(0, k, f) = std::complex<double>(static_cast<double>(k), static_cast<double>(f));

    MHO_PhaseCalibrationTrim op;
    op.SetVisibilities(&vis);
    op.SetArgs(&pc);
    op.Initialize();
    bool ok = op.Execute();
    REQUIRE(ok == false);

    // Pcal should be unchanged
    REQUIRE(pc.GetDimension(MTPCAL_TIME_AXIS) == 4);

    return 0;
}

// CASE 5: Too few pcal points (ntime < 2) -> false.
static int test_case5_too_few_pcal_points()
{
    visibility_type vis;
    vis.Resize(1, 1, 4, 1);
    auto& vtime = std::get<TIME_AXIS>(vis);
    for (std::size_t t = 0; t < 4; t++) vtime.at(t) = static_cast<double>(t) * 1.0;
    vis.Insert(std::string("start"), std::string("2024y001d00h00m00s"));

    multitone_pcal_type pc;
    pc.Resize(1, 1, 2);
    auto& ptime = std::get<MTPCAL_TIME_AXIS>(pc);
    ptime.at(0) = 0.5;
    pc.Insert(std::string("start"), std::string("2024y001d00h00m00s"));
    pc.Insert(std::string("station_code"), std::string("Gs"));

    for (std::size_t f = 0; f < 2; f++)
        pc(0, 0, f) = std::complex<double>(0.0, static_cast<double>(f));

    MHO_PhaseCalibrationTrim op;
    op.SetVisibilities(&vis);
    op.SetArgs(&pc);
    op.Initialize();
    bool ok = op.Execute();
    REQUIRE(ok == false);

    return 0;
}

// CASE 6: Missing vis "start" tag -> false.
static int test_case6_missing_vis_start()
{
    visibility_type vis;
    vis.Resize(1, 1, 4, 1);
    auto& vtime = std::get<TIME_AXIS>(vis);
    for (std::size_t t = 0; t < 4; t++) vtime.at(t) = static_cast<double>(t) * 1.0;
    // No "start" tag inserted

    multitone_pcal_type pc;
    pc.Resize(1, 4, 2);
    auto& ptime = std::get<MTPCAL_TIME_AXIS>(pc);
    for (std::size_t k = 0; k < 4; k++)
        ptime.at(k) = static_cast<double>(k) * 1.0 + 0.5;
    pc.Insert(std::string("start"), std::string("2024y001d00h00m00s"));
    pc.Insert(std::string("station_code"), std::string("Gs"));

    for (std::size_t k = 0; k < 4; k++)
        for (std::size_t f = 0; f < 2; f++)
            pc(0, k, f) = std::complex<double>(static_cast<double>(k), static_cast<double>(f));

    MHO_PhaseCalibrationTrim op;
    op.SetVisibilities(&vis);
    op.SetArgs(&pc);
    op.Initialize();
    bool ok = op.Execute();
    REQUIRE(ok == false);

    return 0;
}

/* CASE 7: Missing pcal "start" tag -> assume equal to vis (warn), then proceed.
   With identical timing, the no-op path should fire -> returns true, size=4. */
static int test_case7_missing_pcal_start()
{
    visibility_type vis;
    vis.Resize(1, 1, 4, 1);
    auto& vtime = std::get<TIME_AXIS>(vis);
    for (std::size_t t = 0; t < 4; t++) vtime.at(t) = static_cast<double>(t) * 1.0;
    vis.Insert(std::string("start"), std::string("2024y001d00h00m00s"));

    multitone_pcal_type pc;
    pc.Resize(1, 4, 2);
    auto& ptime = std::get<MTPCAL_TIME_AXIS>(pc);
    for (std::size_t k = 0; k < 4; k++)
        ptime.at(k) = static_cast<double>(k) * 1.0 + 0.5;
    // No "start" tag on pcal
    pc.Insert(std::string("station_code"), std::string("Gs"));

    for (std::size_t k = 0; k < 4; k++)
        for (std::size_t f = 0; f < 2; f++)
            pc(0, k, f) = std::complex<double>(static_cast<double>(k), static_cast<double>(f));

    MHO_PhaseCalibrationTrim op;
    op.SetVisibilities(&vis);
    op.SetArgs(&pc);
    op.Initialize();
    bool ok = op.Execute();
    REQUIRE(ok == true);

    REQUIRE(pc.GetDimension(MTPCAL_TIME_AXIS) == 4);

    // Data unchanged
    for (std::size_t k = 0; k < 4; k++)
        for (std::size_t f = 0; f < 2; f++)
        {
            CHECK_CLOSE(pc(0, k, f).real(), static_cast<double>(k), 1e-12);
            CHECK_CLOSE(pc(0, k, f).imag(), static_cast<double>(f), 1e-12);
        }

    return 0;
}

// CASE 8: Null visibility pointer -> false.
static int test_case8_null_visibility()
{
    multitone_pcal_type pc;
    pc.Resize(1, 4, 2);
    auto& ptime = std::get<MTPCAL_TIME_AXIS>(pc);
    for (std::size_t k = 0; k < 4; k++)
        ptime.at(k) = static_cast<double>(k) * 1.0 + 0.5;
    pc.Insert(std::string("start"), std::string("2024y001d00h00m00s"));
    pc.Insert(std::string("station_code"), std::string("Gs"));

    for (std::size_t k = 0; k < 4; k++)
        for (std::size_t f = 0; f < 2; f++)
            pc(0, k, f) = std::complex<double>(static_cast<double>(k), static_cast<double>(f));

    MHO_PhaseCalibrationTrim op;
    op.SetVisibilities(nullptr);
    op.SetArgs(&pc);
    op.Initialize();
    bool ok = op.Execute();
    REQUIRE(ok == false);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eWarning);

    if (test_case1_noop()) return 1;
    if (test_case2_trim_trailing()) return 2;
    if (test_case3_trim_leading()) return 3;
    if (test_case4_ap_delta_mismatch()) return 4;
    if (test_case5_too_few_pcal_points()) return 5;
    if (test_case6_missing_vis_start()) return 6;
    if (test_case7_missing_pcal_start()) return 7;
    if (test_case8_null_visibility()) return 8;

    return 0;
}
