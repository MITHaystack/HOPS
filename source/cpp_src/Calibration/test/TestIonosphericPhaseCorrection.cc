#include <cmath>
#include <iostream>
#include <string>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Constants.hh"
#include "MHO_IonosphericPhaseCorrection.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Fixtures

/* Build visibility: npol=2, nchan=2, nap=2, nspec=2
   Channel 0: sky 8000 MHz, bw 32, USB -> center 8016 MHz
   Channel 1: sky 8400 MHz, bw 32, USB -> center 8416 MHz
   All elements = (1.0, 0.0). */
static void build_fixture(visibility_type& vis, bool lsb_channel1 = false)
{
    vis.Resize(2, 2, 2, 2);

    auto chan_ax = &(std::get<CHANNEL_AXIS>(vis));
    (*chan_ax)(0) = 8000.0;
    (*chan_ax)(1) = 8400.0;
    chan_ax->InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax->InsertIndexLabelKeyValue(1, "net_sideband", std::string(lsb_channel1 ? "L" : "U"));
    chan_ax->InsertIndexLabelKeyValue(0, "bandwidth", 32.0);
    chan_ax->InsertIndexLabelKeyValue(1, "bandwidth", 32.0);

    // Fill all elements with (1.0, 0.0)
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex<double>(1.0, 0.0);
}

// Test cases

// CASE 1: Nonzero dTEC applies the expected per-channel phasor
static int test_case1_nonzero_dtec_phasor()
{
    visibility_type vis;
    build_fixture(vis);

    double dTEC = 10.0;
    MHO_IonosphericPhaseCorrection op;
    op.SetDifferentialTEC(dTEC);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    double ion_k = MHO_Constants::ion_k;
    // Channel centers: USB sky+bw/2
    double fc[2] = { 8000.0 + 32.0 / 2.0,  8400.0 + 32.0 / 2.0 };  // 8016, 8416

    for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++) {
        double theta = ion_k * dTEC / (1e6 * fc[ch]);
        std::complex<double> expected(std::cos(theta), std::sin(theta));

        for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++) {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), expected.real(), 1e-9);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), expected.imag(), 1e-9);
                }
    }

    return 0;
}

// CASE 2: 1/f scaling between channels
static int test_case2_1f_scaling()
{
    visibility_type vis;
    build_fixture(vis);

    double dTEC = 10.0;
    MHO_IonosphericPhaseCorrection op;
    op.SetDifferentialTEC(dTEC);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    double ion_k = MHO_Constants::ion_k;
    double fc0 = 8016.0, fc1 = 8416.0;

    // theta = ion_k * dTEC / (1e6 * fc), so theta0/theta1 = fc1/fc0
    double theta0 = ion_k * dTEC / (1e6 * fc0);
    double theta1 = ion_k * dTEC / (1e6 * fc1);
    double ratio = theta0 / theta1;
    double expected_ratio = fc1 / fc0;
    CHECK_CLOSE(ratio, expected_ratio, 1e-12);

    // Also verify operator output matches the per-channel expected phasor
    std::complex<double> expected0(std::cos(theta0), std::sin(theta0));
    std::complex<double> expected1(std::cos(theta1), std::sin(theta1));
    CHECK_CLOSE(vis(0, 0, 0, 0).real(), expected0.real(), 1e-9);
    CHECK_CLOSE(vis(0, 0, 0, 0).imag(), expected0.imag(), 1e-9);
    CHECK_CLOSE(vis(0, 1, 0, 0).real(), expected1.real(), 1e-9);
    CHECK_CLOSE(vis(0, 1, 0, 0).imag(), expected1.imag(), 1e-9);

    return 0;
}

// CASE 3: dtec_phase_deg metadata written correctly
static int test_case3_metadata()
{
    visibility_type vis;
    build_fixture(vis);

    double dTEC = 10.0;
    MHO_IonosphericPhaseCorrection op;
    op.SetDifferentialTEC(dTEC);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    double ion_k = MHO_Constants::ion_k;
    double fc[2] = { 8016.0, 8416.0 };

    auto chan_ax = &(std::get<CHANNEL_AXIS>(vis));
    for (std::size_t ch = 0; ch < 2; ch++) {
        double theta = ion_k * dTEC / (1e6 * fc[ch]);
        double expected_deg = theta * MHO_Constants::rad_to_deg;

        double actual_deg;
        bool found = chan_ax->RetrieveIndexLabelKeyValue(ch, "dtec_phase_deg", actual_deg);
        REQUIRE(found);
        CHECK_CLOSE(actual_deg, expected_deg, 1e-6);
    }

    return 0;
}

// CASE 4: dTEC == 0 is a no-op (but metadata is still written as 0)
static int test_case4_zero_dtec()
{
    visibility_type vis;
    build_fixture(vis);
    visibility_type pristine;
    pristine.Copy(vis);

    MHO_IonosphericPhaseCorrection op2;
    op2.SetDifferentialTEC(0.0);
    op2.SetArgs(&vis);
    REQUIRE(op2.Initialize());
    op2.Execute();

    // Array must be unchanged
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    CHECK_CLOSE(std::abs(vis(pp, ch, ap, sp) - pristine(pp, ch, ap, sp)), 0.0, 1e-12);

    // dtec_phase_deg label must still be written with value 0
    auto chan_ax = &(std::get<CHANNEL_AXIS>(vis));
    for (std::size_t ch = 0; ch < 2; ch++) {
        double actual_deg;
        bool found = chan_ax->RetrieveIndexLabelKeyValue(ch, "dtec_phase_deg", actual_deg);
        REQUIRE(found);
        CHECK_CLOSE(actual_deg, 0.0, 1e-12);
    }

    return 0;
}

// CASE 5: LSB channel uses lower center frequency
static int test_case5_lsb_channel()
{
    visibility_type vis;
    build_fixture(vis, true);  // channel 1 is LSB

    double dTEC = 10.0;
    MHO_IonosphericPhaseCorrection op;
    op.SetDifferentialTEC(dTEC);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    op.Execute();

    double ion_k = MHO_Constants::ion_k;
    /* Channel 0: USB center 8016 (same as before)
       Channel 1: LSB center = 8400 - 32/2 = 8384 */
    double fc0 = 8016.0, fc1_lsb = 8384.0;

    // Verify channel 0 is unchanged from USB case
    {
        double theta = ion_k * dTEC / (1e6 * fc0);
        std::complex<double> expected(std::cos(theta), std::sin(theta));
        CHECK_CLOSE(vis(0, 0, 0, 0).real(), expected.real(), 1e-9);
        CHECK_CLOSE(vis(0, 0, 0, 0).imag(), expected.imag(), 1e-9);
    }

    // Verify channel 1 uses LSB center frequency
    {
        double theta = ion_k * dTEC / (1e6 * fc1_lsb);
        std::complex<double> expected(std::cos(theta), std::sin(theta));

        for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++) {
                    CHECK_CLOSE(vis(pp, 1, ap, sp).real(), expected.real(), 1e-9);
                    CHECK_CLOSE(vis(pp, 1, ap, sp).imag(), expected.imag(), 1e-9);
                }
    }

    // Verify LSB center differs from USB center (8416 vs 8384)
    double arg_usb_case = std::arg(std::exp(std::complex<double>(0, ion_k * dTEC / (1e6 * 8416.0))));
    double arg_lsb_case = std::arg(vis(0, 1, 0, 0));
    REQUIRE(std::fabs(arg_lsb_case - arg_usb_case) > 1e-12);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_nonzero_dtec_phasor()) return 1;
    if (test_case2_1f_scaling()) return 1;
    if (test_case3_metadata()) return 1;
    if (test_case4_zero_dtec()) return 1;
    if (test_case5_lsb_channel()) return 1;

    return 0;
}
