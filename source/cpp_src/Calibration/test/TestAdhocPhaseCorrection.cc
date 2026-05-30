#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_AdhocPhaseCorrection.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Constants.hh"
#include "MHO_Clock.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

static std::string make_temp_file(const std::string& suffix)
{
    static int counter = 0;
    char name[256];
    std::snprintf(name, sizeof(name), "/tmp/hops_adhoc_phase_%d_%s.tmp", counter++, suffix.c_str());
    return std::string(name);
}

// Write a PHYLE adhoc-phase file: each row is "<fpday>  <phase_deg_0>  <phase_deg_1> ..."
static void write_adhoc_file(const std::string& path, const std::vector<std::vector<double> >& rows)
{
    std::ofstream ofs(path.c_str());
    if (!ofs.is_open()) {
        std::cerr << "FAIL: cannot open adhoc file " << path
                  << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
        std::exit(1);
    }
    for (std::size_t r = 0; r < rows.size(); r++) {
        for (std::size_t c = 0; c < rows[r].size(); c++) {
            if (c > 0) ofs << "  ";
            ofs << rows[r][c];
        }
        ofs << std::endl;
    }
    ofs.close();
}

/* Build a visibility fixture: (npol=1, nchan=2, nap=4, nspec=2)
   ch0 label = "a", ch1 label = "b".  All elements = (1.0, 0.0).
   "start" tag = "2024y001d00h00m00s" (fpday ~ 1.0 for Jan 1 00:00). */
static void build_fixture(visibility_type& vis)
{
    vis.Resize(1, 2, 4, 2);

    // TIME axis: 0, 1, 2, 3  (AP = 1.0 s, center = t + 0.5)
    auto& time_ax = std::get< TIME_AXIS >(vis);
    for (std::size_t t = 0; t < time_ax.GetSize(); t++)
        time_ax.at(t) = static_cast<double>(t) * 1.0;

    // CHANNEL axis: freq-code labels
    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));

    // POLPROD axis
    auto& pp_ax = std::get< POLPROD_AXIS >(vis);
    pp_ax[0] = "XX";

    // Start tag
    vis.Insert(std::string("start"), std::string("2024y001d00h00m00s"));

    // Fill all elements with (1.0, 0.0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
                for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++)
                    vis(pp, ch, t, f) = std::complex<double>(1.0, 0.0);
}

// Test cases

// CASE 1: NONE mode is a pure no-op
static int test_case1_none_noop()
{
    visibility_type vis;
    build_fixture(vis);
    visibility_type pristine;
    pristine.Copy(vis);

    MHO_AdhocPhaseCorrection op;
    op.SetMode(AdhocPhaseMode::NONE);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    // Array must be unchanged
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
                for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++)
                    CHECK_CLOSE(std::abs(vis(pp, ch, t, f) - pristine(pp, ch, t, f)), 0.0, 1e-12);

    return 0;
}

// CASE 2: POLYNOMIAL constant term c0 only
static int test_case2_polynomial_c0()
{
    visibility_type vis;
    build_fixture(vis);

    double c0 = 0.5; // radians
    MHO_AdhocPhaseCorrection op;
    op.SetMode(AdhocPhaseMode::POLYNOMIAL);
    std::vector<double> coeffs;
    coeffs.push_back(c0);
    op.SetPolynomialCoeffs(coeffs);
    op.SetTRef(0.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    double expected_re = std::cos(c0);
    double expected_im = -std::sin(c0);

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
                for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++) {
                    CHECK_CLOSE(vis(pp, ch, t, f).real(), expected_re, 1e-9);
                    CHECK_CLOSE(vis(pp, ch, t, f).imag(), expected_im, 1e-9);
                }

    return 0;
}

// CASE 3: POLYNOMIAL time dependence (c1) -- check per-AP phase increment
static int test_case3_polynomial_c1()
{
    visibility_type vis;
    build_fixture(vis);

    double c1 = 0.01; // rad/s
    MHO_AdhocPhaseCorrection op;
    op.SetMode(AdhocPhaseMode::POLYNOMIAL);
    std::vector<double> coeffs;
    coeffs.push_back(0.0);
    coeffs.push_back(c1);
    op.SetPolynomialCoeffs(coeffs);
    op.SetTRef(0.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    // Phase increment between consecutive APs should be -c1*AP = -0.01 rad
    // Check at pol=0, spec=0 (applies to all pol-products and spectral points)
    double expected_delta_arg = -c1 * 1.0; // AP = 1.0 s
    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++) {
        for (std::size_t t = 0; t + 1 < vis.GetDimension(TIME_AXIS); t++) {
            double arg_t  = std::arg(vis(0, ch, t, 0));
            double arg_t1 = std::arg(vis(0, ch, t + 1, 0));
            CHECK_CLOSE(arg_t1 - arg_t, expected_delta_arg, 1e-9);
        }
    }

    return 0;
}

// CASE 4: SINEWAVE -- verify shape by computing expected zeta directly
static int test_case4_sinewave()
{
    visibility_type vis;
    build_fixture(vis);

    double amp = 0.2;
    double period = 4.0;
    MHO_AdhocPhaseCorrection op;
    op.SetMode(AdhocPhaseMode::SINEWAVE);
    op.SetAmplitude(amp);
    op.SetPeriod(period);
    op.SetTRef(0.0);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    /* Compute expected zeta using the same clock conversion the operator uses.
       thyme = ap_center_sec + scanStartFpDay * 86400 - tref.
       We replicate the computation to get expected values. */
    hops_clock::time_point tp = hops_clock::from_vex_format("2024y001d00h00m00s");
    int yr;
    double fpday;
    hops_clock::to_year_fpday(tp, yr, fpday);

    auto& time_ax = std::get< TIME_AXIS >(vis);
    double ap = time_ax.at(1) - time_ax.at(0); // AP = 1.0 s

    for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++) {
        double ap_center_sec = time_ax.at(t) + 0.5 * ap;
        double thyme = ap_center_sec + fpday * 86400.0;
        double phase_arg = 2.0 * M_PI * thyme / period;
        double expected_zeta = amp * std::sin(phase_arg);
        // phasor = exp(-i * expected_zeta) applied to (1,0)
        double expected_re = std::cos(expected_zeta);
        double expected_im = -std::sin(expected_zeta);

        // Check first pol-product and first spectral point
        CHECK_CLOSE(vis(0, 0, t, 0).real(), expected_re, 1e-6);
        CHECK_CLOSE(vis(0, 0, t, 0).imag(), expected_im, 1e-6);
    }

    /* Also verify the zeta values trace a sine: check that
       half-period-apart APs give opposite signs */
    double zeta[4];
    for (std::size_t t = 0; t < 4; t++)
        zeta[t] = -std::arg(vis(0, 0, t, 0));
    // zeta[0] and zeta[2] should be approximately negatives (pi phase shift)
    if (std::fabs(zeta[0]) > 1e-10) {
        CHECK_CLOSE(zeta[2], -zeta[0], 1e-6);
    }

    return 0;
}

// CASE 5: PHYLE differential phase (constant files)
static int test_case5_phyle_differential()
{
    visibility_type vis;
    build_fixture(vis);

    /* Create temp adhoc files with constant phases
       ref: 30 deg for 'a', 60 deg for 'b'
       rem: 10 deg for 'a', 20 deg for 'b'
       Both files have 2 rows at fpday 1.0 and 2.0 so interpolation is trivial. */
    std::string ref_file = make_temp_file("case5_ref");
    std::string rem_file = make_temp_file("case5_rem");

    std::vector<std::vector<double> > ref_rows;
    ref_rows.push_back(std::vector<double>{1.0, 30.0, 60.0});
    ref_rows.push_back(std::vector<double>{2.0, 30.0, 60.0});
    write_adhoc_file(ref_file, ref_rows);

    std::vector<std::vector<double> > rem_rows;
    rem_rows.push_back(std::vector<double>{1.0, 10.0, 20.0});
    rem_rows.push_back(std::vector<double>{2.0, 10.0, 20.0});
    write_adhoc_file(rem_file, rem_rows);

    MHO_AdhocPhaseCorrection op;
    op.SetMode(AdhocPhaseMode::PHYLE);
    op.SetRefAdhocFile(ref_file, "ab");
    op.SetRemAdhocFile(rem_file, "ab");
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    /* Channel 'a': zeta = (30-10) deg = 20 deg
       Channel 'b': zeta = (60-20) deg = 40 deg */
    double zeta_a = 20.0 * MHO_Constants::deg_to_rad;
    double zeta_b = 40.0 * MHO_Constants::deg_to_rad;

    for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
        for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++) {
            // channel 0 ('a')
            CHECK_CLOSE(vis(0, 0, t, f).real(), std::cos(zeta_a), 1e-7);
            CHECK_CLOSE(vis(0, 0, t, f).imag(), -std::sin(zeta_a), 1e-7);
            // channel 1 ('b')
            CHECK_CLOSE(vis(0, 1, t, f).real(), std::cos(zeta_b), 1e-7);
            CHECK_CLOSE(vis(0, 1, t, f).imag(), -std::sin(zeta_b), 1e-7);
        }

    std::remove(ref_file.c_str());
    std::remove(rem_file.c_str());
    return 0;
}

// CASE 6: PHYLE unknown freq-code -> zeta = 0 (no correction)
static int test_case6_phyle_unknown_code()
{
    visibility_type vis;
    build_fixture(vis);

    // Change channel 1's label to "z" (not in channel string "ab")
    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("z"));

    std::string ref_file = make_temp_file("case6_ref");
    std::string rem_file = make_temp_file("case6_rem");

    std::vector<std::vector<double> > ref_rows;
    ref_rows.push_back(std::vector<double>{1.0, 30.0, 60.0});
    ref_rows.push_back(std::vector<double>{2.0, 30.0, 60.0});
    write_adhoc_file(ref_file, ref_rows);

    std::vector<std::vector<double> > rem_rows;
    rem_rows.push_back(std::vector<double>{1.0, 10.0, 20.0});
    rem_rows.push_back(std::vector<double>{2.0, 10.0, 20.0});
    write_adhoc_file(rem_file, rem_rows);

    MHO_AdhocPhaseCorrection op;
    op.SetMode(AdhocPhaseMode::PHYLE);
    op.SetRefAdhocFile(ref_file, "ab");
    op.SetRemAdhocFile(rem_file, "ab");
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    double zeta_a = 20.0 * MHO_Constants::deg_to_rad;

    // Channel 0 ('a') -- corrected
    for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
        for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++) {
            CHECK_CLOSE(vis(0, 0, t, f).real(), std::cos(zeta_a), 1e-7);
            CHECK_CLOSE(vis(0, 0, t, f).imag(), -std::sin(zeta_a), 1e-7);
        }

    // Channel 1 ('z') -- unchanged because 'z' not in "ab"
    for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
        for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++) {
            CHECK_CLOSE(vis(0, 1, t, f).real(), 1.0, 1e-12);
            CHECK_CLOSE(vis(0, 1, t, f).imag(), 0.0, 1e-12);
        }

    std::remove(ref_file.c_str());
    std::remove(rem_file.c_str());
    return 0;
}

// CASE 7: Initialize failure -- missing "start" tag
static int test_case7_missing_start()
{
    visibility_type vis;
    // Build fixture without "start" tag
    vis.Resize(1, 2, 4, 2);

    auto& time_ax = std::get< TIME_AXIS >(vis);
    for (std::size_t t = 0; t < time_ax.GetSize(); t++)
        time_ax.at(t) = static_cast<double>(t) * 1.0;

    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));

    auto& pp_ax = std::get< POLPROD_AXIS >(vis);
    pp_ax[0] = "XX";
    // NOTE: intentionally NOT inserting "start" tag

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
                for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++)
                    vis(pp, ch, t, f) = std::complex<double>(1.0, 0.0);

    MHO_AdhocPhaseCorrection op;
    op.SetMode(AdhocPhaseMode::POLYNOMIAL);
    std::vector<double> coeffs;
    coeffs.push_back(0.5);
    op.SetPolynomialCoeffs(coeffs);
    op.SetArgs(&vis);
    bool ok = op.Initialize();
    REQUIRE(ok == false);

    return 0;
}

// CASE 8: Initialize failure -- nap < 2 (single AP)
static int test_case8_too_few_aps()
{
    visibility_type vis;
    // Resize to single AP: (npol=1, nchan=2, nap=1, nspec=2)
    vis.Resize(1, 2, 1, 2);

    auto& time_ax = std::get< TIME_AXIS >(vis);
    time_ax.at(0) = 0.0;

    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));

    auto& pp_ax = std::get< POLPROD_AXIS >(vis);
    pp_ax[0] = "XX";

    vis.Insert(std::string("start"), std::string("2024y001d00h00m00s"));

    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < vis.GetDimension(TIME_AXIS); t++)
                for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++)
                    vis(pp, ch, t, f) = std::complex<double>(1.0, 0.0);

    MHO_AdhocPhaseCorrection op;
    op.SetMode(AdhocPhaseMode::SINEWAVE);
    op.SetAmplitude(0.1);
    op.SetPeriod(2.0);
    op.SetArgs(&vis);
    bool ok = op.Initialize();
    REQUIRE(ok == false);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_none_noop()) return 1;
    if (test_case2_polynomial_c0()) return 1;
    if (test_case3_polynomial_c1()) return 1;
    if (test_case4_sinewave()) return 1;
    if (test_case5_phyle_differential()) return 1;
    if (test_case6_phyle_unknown_code()) return 1;
    if (test_case7_missing_start()) return 1;
    if (test_case8_too_few_aps()) return 1;

    return 0;
}
