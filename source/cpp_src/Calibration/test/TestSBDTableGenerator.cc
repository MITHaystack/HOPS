#include <cmath>
#include <complex>
#include <iostream>
#include <vector>

#include "MHO_SBDTableGenerator.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* PADDING_FACTOR is #define'd as 4 in MHO_SBDTableGenerator.cc (private, not exported).
   We hard-code 4 here; if that constant changes, Cases 1/3/8 expectations must change. */
static const std::size_t PADDING_FACTOR = 4;

// Helpers

/* Build a synthetic input visibility with tagged axes.
   dim layout: {POLPROD, CHANNEL, TIME, FREQ}. */
static void build_input_visibility(visibility_type& vis, std::size_t in_dim[VIS_NDIM])
{
    vis.Resize(in_dim);
    vis.ZeroArray();

    // Tag the POLPROD axis (string type)
    auto& pol_ax = std::get<POLPROD_AXIS>(vis);
    for (std::size_t i = 0; i < in_dim[POLPROD_AXIS]; ++i) {
        pol_ax(i) = "POL" + std::to_string(i);
    }

    // Tag the CHANNEL axis (double type - sky frequency)
    auto& chan_ax = std::get<CHANNEL_AXIS>(vis);
    for (std::size_t i = 0; i < in_dim[CHANNEL_AXIS]; ++i) {
        chan_ax(i) = 4000.0 + static_cast<double>(i) * 100.0;
    }

    // Tag the TIME axis (double type)
    auto& time_ax = std::get<TIME_AXIS>(vis);
    for (std::size_t i = 0; i < in_dim[TIME_AXIS]; ++i) {
        time_ax(i) = 100.0 + static_cast<double>(i);
    }

    // Tag the FREQ axis (double type - distinctive values)
    auto& freq_ax = std::get<FREQ_AXIS>(vis);
    for (std::size_t i = 0; i < in_dim[FREQ_AXIS]; ++i) {
        freq_ax(i) = 1000.0 + static_cast<double>(i);
    }
}

// Verify all four dimensions match expected.
static void check_dims(const sbd_type& out, std::size_t exp[VIS_NDIM], int& ret)
{
    std::size_t out_dim[VIS_NDIM];
    out.GetDimensions(out_dim);
    for (std::size_t i = 0; i < VIS_NDIM; ++i) {
        if (out_dim[i] != exp[i]) {
            std::cerr << "FAIL: out_dim[" << i << "] = " << out_dim[i]
                      << " != expected " << exp[i] << " @ " << __FILE__ << ":"
                      << __LINE__ << std::endl;
            ret = 1;
        }
    }
}

// Test cases

// CASE 1 - Resize from empty output (happy path)
static int test_resize_from_empty()
{
    std::size_t in_dim[VIS_NDIM] = {1, 3, 2, 8};
    visibility_type vis;
    build_input_visibility(vis, in_dim);

    sbd_type sbd; // unsized

    MHO_SBDTableGenerator gen;
    gen.SetArgs(&vis, &sbd);
    REQUIRE(gen.Initialize());
    REQUIRE(gen.Execute());

    std::size_t exp[VIS_NDIM] = {1, 3, 2, PADDING_FACTOR * 8};
    int ret = 0;
    check_dims(sbd, exp, ret);
    return ret;
}

// CASE 2 - POLPROD / CHANNEL / TIME axes are copied from input
static int test_axes_copied()
{
    std::size_t in_dim[VIS_NDIM] = {1, 3, 2, 8};
    visibility_type vis;
    build_input_visibility(vis, in_dim);

    sbd_type sbd;
    MHO_SBDTableGenerator gen;
    gen.SetArgs(&vis, &sbd);
    REQUIRE(gen.Initialize());
    REQUIRE(gen.Execute());

    // Check POLPROD axis (string)
    const auto& in_pol  = std::get<POLPROD_AXIS>(vis);
    const auto& out_pol = std::get<POLPROD_AXIS>(sbd);
    REQUIRE(in_pol.GetSize() == out_pol.GetSize());
    for (std::size_t i = 0; i < in_pol.GetSize(); ++i) {
        REQUIRE(in_pol(i) == out_pol(i));
    }

    // Check CHANNEL axis (double)
    const auto& in_chan  = std::get<CHANNEL_AXIS>(vis);
    const auto& out_chan = std::get<CHANNEL_AXIS>(sbd);
    REQUIRE(in_chan.GetSize() == out_chan.GetSize());
    for (std::size_t i = 0; i < in_chan.GetSize(); ++i) {
        CHECK_CLOSE(out_chan(i), in_chan(i), 1e-12);
    }

    // Check TIME axis (double)
    const auto& in_time  = std::get<TIME_AXIS>(vis);
    const auto& out_time = std::get<TIME_AXIS>(sbd);
    REQUIRE(in_time.GetSize() == out_time.GetSize());
    for (std::size_t i = 0; i < in_time.GetSize(); ++i) {
        CHECK_CLOSE(out_time(i), in_time(i), 1e-12);
    }

    return 0;
}

// CASE 3 - FREQ axis is NOT copied (left as freshly-sized/zeroed)
static int test_freq_not_copied()
{
    std::size_t in_dim[VIS_NDIM] = {1, 3, 2, 8};
    visibility_type vis;
    build_input_visibility(vis, in_dim);

    // Input FREQ axis has distinctive values 1000.0..1007.0
    const auto& in_freq = std::get<FREQ_AXIS>(vis);
    REQUIRE(in_freq(0) == 1000.0);

    sbd_type sbd;
    MHO_SBDTableGenerator gen;
    gen.SetArgs(&vis, &sbd);
    REQUIRE(gen.Initialize());
    REQUIRE(gen.Execute());

    const auto& out_freq = std::get<FREQ_AXIS>(sbd);
    REQUIRE(out_freq.GetSize() == PADDING_FACTOR * 8);

    // The FREQ axis values should be default-initialized (0.0), not the input's 1000.0
    CHECK_CLOSE(out_freq(0), 0.0, 1e-12);
    // Also verify the input freq values were NOT transplanted
    REQUIRE(out_freq(0) != in_freq(0));

    return 0;
}

// CASE 4 - Output data buffer is zeroed
static int test_data_zeroed()
{
    std::size_t in_dim[VIS_NDIM] = {1, 3, 2, 8};
    visibility_type vis;
    build_input_visibility(vis, in_dim);

    sbd_type sbd;
    MHO_SBDTableGenerator gen;
    gen.SetArgs(&vis, &sbd);
    REQUIRE(gen.Initialize());
    REQUIRE(gen.Execute());

    // Scan all elements - should be (0,0)
    for (auto it = sbd.begin(); it != sbd.end(); ++it) {
        CHECK_CLOSE(it->real(), 0.0, 1e-12);
        CHECK_CLOSE(it->imag(), 0.0, 1e-12);
    }

    return 0;
}

// CASE 5 - Already-correctly-sized output is left in place (no spurious resize)
static int test_already_sized()
{
    std::size_t in_dim[VIS_NDIM] = {1, 3, 2, 8};
    visibility_type vis;
    build_input_visibility(vis, in_dim);

    // Pre-size output to exactly the correct dimensions
    std::size_t out_dim[VIS_NDIM] = {1, 3, 2, PADDING_FACTOR * 8};
    sbd_type sbd;
    sbd.Resize(out_dim);
    sbd.ZeroArray();

    // Copy axes so they match input (so no resize is needed)
    std::get<POLPROD_AXIS>(sbd).Copy(std::get<POLPROD_AXIS>(vis));
    std::get<CHANNEL_AXIS>(sbd).Copy(std::get<CHANNEL_AXIS>(vis));
    std::get<TIME_AXIS>(sbd).Copy(std::get<TIME_AXIS>(vis));

    // Set a SENTINEL value in the data buffer
    sbd(0, 0, 0, 0) = std::complex<double>(7.0, 7.0);

    MHO_SBDTableGenerator gen;
    gen.SetArgs(&vis, &sbd);
    REQUIRE(gen.Initialize());
    REQUIRE(gen.Execute());

    // Sentinel must remain untouched (no resize/zero happened)
    CHECK_CLOSE(sbd(0, 0, 0, 0).real(), 7.0, 1e-12);
    CHECK_CLOSE(sbd(0, 0, 0, 0).imag(), 7.0, 1e-12);

    // Dims should still be correct
    int ret = 0;
    check_dims(sbd, out_dim, ret);
    return ret;
}

// CASE 6 - Re-initialization / idempotency
static int test_reinit()
{
    std::size_t in_dim[VIS_NDIM] = {1, 3, 2, 8};
    visibility_type vis;
    build_input_visibility(vis, in_dim);

    sbd_type sbd;
    MHO_SBDTableGenerator gen;
    gen.SetArgs(&vis, &sbd);
    REQUIRE(gen.Initialize());
    REQUIRE(gen.Execute());

    // Capture dims after first init
    std::size_t first_dim[VIS_NDIM];
    sbd.GetDimensions(first_dim);

    // Re-initialize with same args
    REQUIRE(gen.Initialize());
    REQUIRE(gen.Execute());

    // Dims should be unchanged
    std::size_t second_dim[VIS_NDIM];
    sbd.GetDimensions(second_dim);
    for (std::size_t i = 0; i < VIS_NDIM; ++i) {
        REQUIRE(first_dim[i] == second_dim[i]);
    }

    return 0;
}

// CASE 7 - Execute before Initialize fails
static int test_execute_before_init()
{
    std::size_t in_dim[VIS_NDIM] = {1, 3, 2, 8};
    visibility_type vis;
    build_input_visibility(vis, in_dim);

    sbd_type sbd;
    MHO_SBDTableGenerator gen;
    gen.SetArgs(&vis, &sbd);
    // Do NOT call Initialize()

    REQUIRE(gen.Execute() == false);

    return 0;
}

// CASE 8 - Different FREQ size re-pads correctly
static int test_different_freq_resize()
{
    // Input with freq dim 5
    std::size_t in_dim[VIS_NDIM] = {1, 3, 2, 5};
    visibility_type vis;
    build_input_visibility(vis, in_dim);

    // Pre-size output with the OLD freq (32 = 4*8 from previous test)
    std::size_t old_out_dim[VIS_NDIM] = {1, 3, 2, 32};
    sbd_type sbd;
    sbd.Resize(old_out_dim);
    sbd.ZeroArray();

    MHO_SBDTableGenerator gen;
    gen.SetArgs(&vis, &sbd);
    REQUIRE(gen.Initialize());

    // sbd FREQ dim should now be 4*5 = 20
    std::size_t out_dim[VIS_NDIM];
    sbd.GetDimensions(out_dim);
    REQUIRE(out_dim[POLPROD_AXIS] == 1);
    REQUIRE(out_dim[CHANNEL_AXIS] == 3);
    REQUIRE(out_dim[TIME_AXIS] == 2);
    REQUIRE(out_dim[FREQ_AXIS] == PADDING_FACTOR * 5);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_resize_from_empty())  return 1;
    if (test_axes_copied())        return 1;
    if (test_freq_not_copied())    return 1;
    if (test_data_zeroed())        return 1;
    if (test_already_sized())      return 1;
    if (test_reinit())             return 1;
    if (test_execute_before_init()) return 1;
    if (test_different_freq_resize()) return 1;

    return 0;
}
