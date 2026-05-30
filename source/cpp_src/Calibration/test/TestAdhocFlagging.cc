#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_AdhocFlagging.hh"
#include "MHO_Clock.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

static std::string make_temp_flag_file(const std::string& suffix)
{
    static int counter = 0;
    char name[256];
    std::snprintf(name, sizeof(name), "/tmp/hops_adhoc_flag_%d_%s.tmp", counter++, suffix.c_str());
    return std::string(name);
}

static void write_flag_file(const std::string& path, double fpday, const std::string& hex)
{
    std::ofstream ofs(path.c_str());
    if (!ofs.is_open()) {
        std::cerr << "FAIL: cannot open flag file " << path
                  << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
        std::exit(1);
    }
    ofs << fpday << "  " << hex << std::endl;
    ofs.close();
}

/* Write two rows (before and after all AP centers) so the flag table
   brackets the full AP time range. Both rows use the same hex token. */
static void write_flag_file_bracketed(const std::string& path, double fpday_before,
                                      double fpday_after, const std::string& hex)
{
    std::ofstream ofs(path.c_str());
    if (!ofs.is_open()) {
        std::cerr << "FAIL: cannot open flag file " << path
                  << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
        std::exit(1);
    }
    ofs << fpday_before << "  " << hex << std::endl;
    ofs << fpday_after << "  " << hex << std::endl;
    ofs.close();
}

static void write_empty_flag_file(const std::string& path)
{
    std::ofstream ofs(path.c_str());
    if (!ofs.is_open()) {
        std::cerr << "FAIL: cannot open empty flag file " << path
                  << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
        std::exit(1);
    }
    ofs.close();
}

/* Build a standard weight fixture: (npol=2, nchan=2, nap=4, nspec=1)
   ch0 = USB ("U"), ch1 = LSB ("L"), weights all 1.0.
   Returns bracket times for flag-file rows that span all AP centers. */
struct FixtureInfo
{
    double scan_fpday;      // scan start fpday
    double row_fpday_lo;    // flag-file row before first AP center
    double row_fpday_hi;    // flag-file row after last AP center
};

static FixtureInfo build_fixture(weight_type& w, bool include_start_tag)
{
    w.Resize(2, 2, 4, 1);

    // TIME axis: 0, 1, 2, 3 (AP = 1.0 s, center = t + 0.5)
    auto& time_ax = std::get< TIME_AXIS >(w);
    for (std::size_t t = 0; t < time_ax.GetSize(); t++)
        time_ax.at(t) = (double)t * 1.0;

    // CHANNEL axis: sideband labels
    auto& chan_ax = std::get< CHANNEL_AXIS >(w);
    chan_ax.InsertIndexLabelKeyValue(0, "net_sideband", std::string("U"));
    chan_ax.InsertIndexLabelKeyValue(1, "net_sideband", std::string("L"));

    // POLPROD axis
    auto& pp_ax = std::get< POLPROD_AXIS >(w);
    pp_ax[0] = "XX";
    pp_ax[1] = "YY";

    // Start tag (optional)
    if (include_start_tag)
        w.Insert(std::string("start"), std::string("2024y001d00h00m00s"));

    // Fill weights with 1.0
    for (std::size_t pp = 0; pp < w.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < w.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t t = 0; t < w.GetDimension(TIME_AXIS); t++)
                w(pp, ch, t, 0) = 1.0;

    /* Compute fpday of scan start for flag file row times.
       AP centers are at scan_fpday + (t + 0.5*AP) / 86400.
       With 4 APs and AP=1.0 s: first AP center ~= 0.5 s, last ~= 3.5 s.
       LookupFlagBytes requires query_fpday to be between the first and
       last row times. We write two rows: one before first AP (0.25 s)
       and one after last AP (4.5 s) to bracket the full range. */
    FixtureInfo info;
    hops_clock::time_point tp = hops_clock::from_vex_format("2024y001d00h00m00s");
    int yr;
    hops_clock::to_year_fpday(tp, yr, info.scan_fpday);
    info.row_fpday_lo = info.scan_fpday + 0.25 / 86400.0;  // before first AP center
    info.row_fpday_hi = info.scan_fpday + 4.5 / 86400.0;   // after last AP center

    return info;
}

// Test cases

// CASE 1: DecodeHexToken "80" -> USB flagged, LSB retained
static int test_case1_decode_hex_token_80()
{
    weight_type w;
    FixtureInfo info = build_fixture(w, true);

    std::string ref_file = make_temp_flag_file("case1");
    write_flag_file_bracketed(ref_file, info.row_fpday_lo, info.row_fpday_hi, "80");

    MHO_AdhocFlagging op;
    op.SetRefFlagFile(ref_file);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // ch0 (USB): 0x80 & 0x55 = 0x00 -> zeroed
    for (std::size_t pp = 0; pp < 2; pp++)
        for (std::size_t t = 0; t < 4; t++)
            CHECK_CLOSE(w(pp, 0, t, 0), 0.0, 1e-15);

    // ch1 (LSB): 0x80 & 0xAA = 0x80 != 0 -> retained
    for (std::size_t pp = 0; pp < 2; pp++)
        for (std::size_t t = 0; t < 4; t++)
            CHECK_CLOSE(w(pp, 1, t, 0), 1.0, 1e-15);

    std::remove(ref_file.c_str());
    return 0;
}

// CASE 2: Byte 0x55 -> USB retained, LSB flagged
static int test_case2_byte_55()
{
    weight_type w;
    FixtureInfo info = build_fixture(w, true);

    std::string ref_file = make_temp_flag_file("case2");
    write_flag_file_bracketed(ref_file, info.row_fpday_lo, info.row_fpday_hi, "55");

    MHO_AdhocFlagging op;
    op.SetRefFlagFile(ref_file);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // ch0 (USB): 0x55 & 0x55 = 0x55 != 0 -> retained
    for (std::size_t pp = 0; pp < 2; pp++)
        for (std::size_t t = 0; t < 4; t++)
            CHECK_CLOSE(w(pp, 0, t, 0), 1.0, 1e-15);

    // ch1 (LSB): 0x55 & 0xAA = 0x00 -> zeroed
    for (std::size_t pp = 0; pp < 2; pp++)
        for (std::size_t t = 0; t < 4; t++)
            CHECK_CLOSE(w(pp, 1, t, 0), 0.0, 1e-15);

    std::remove(ref_file.c_str());
    return 0;
}

// CASE 3: ref AND rem combination -> FF & 00 = 00 -> all zeroed
static int test_case3_ref_and_rem()
{
    weight_type w;
    FixtureInfo info = build_fixture(w, true);

    std::string ref_file = make_temp_flag_file("case3r");
    std::string rem_file = make_temp_flag_file("case3m");
    write_flag_file_bracketed(ref_file, info.row_fpday_lo, info.row_fpday_hi, "FF");
    write_flag_file_bracketed(rem_file, info.row_fpday_lo, info.row_fpday_hi, "00");

    MHO_AdhocFlagging op;
    op.SetRefFlagFile(ref_file);
    op.SetRemFlagFile(rem_file);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // combined = 0xFF & 0x00 = 0x00 -> both USB and LSB zeroed
    for (std::size_t pp = 0; pp < 2; pp++)
        for (std::size_t ch = 0; ch < 2; ch++)
            for (std::size_t t = 0; t < 4; t++)
                CHECK_CLOSE(w(pp, ch, t, 0), 0.0, 1e-15);

    std::remove(ref_file.c_str());
    std::remove(rem_file.c_str());
    return 0;
}

// CASE 4: Empty station -> 0xFF -> no flagging
static int test_case4_empty_ref()
{
    weight_type w;
    FixtureInfo info = build_fixture(w, true);

    // Keep a pristine copy for comparison
    weight_type pristine;
    pristine.Copy(w);

    std::string rem_file = make_temp_flag_file("case4");
    write_flag_file_bracketed(rem_file, info.row_fpday_lo, info.row_fpday_hi, "FF");

    // ref file is NOT set (empty) -> treated as all 0xFF -> no flagging
    MHO_AdhocFlagging op;
    op.SetRemFlagFile(rem_file);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Array should be unchanged (all 1.0)
    for (std::size_t pp = 0; pp < 2; pp++)
        for (std::size_t ch = 0; ch < 2; ch++)
            for (std::size_t t = 0; t < 4; t++)
                CHECK_CLOSE(w(pp, ch, t, 0), pristine(pp, ch, t, 0), 1e-15);

    std::remove(rem_file.c_str());
    return 0;
}

// CASE 5: Time out of file range -> 0xFF -> no flagging
static int test_case5_time_out_of_range()
{
    weight_type w;
    FixtureInfo info = build_fixture(w, true);

    weight_type pristine;
    pristine.Copy(w);

    // Flag file row at scan_fpday + 1.0 (~1 day after all AP centers)
    std::string ref_file = make_temp_flag_file("case5");
    write_flag_file(ref_file, info.scan_fpday + 1.0, "00");

    MHO_AdhocFlagging op;
    op.SetRefFlagFile(ref_file);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // All AP centers < table.front().time -> LookupFlagBytes returns ALL_GOOD
    for (std::size_t pp = 0; pp < 2; pp++)
        for (std::size_t ch = 0; ch < 2; ch++)
            for (std::size_t t = 0; t < 4; t++)
                CHECK_CLOSE(w(pp, ch, t, 0), pristine(pp, ch, t, 0), 1e-15);

    std::remove(ref_file.c_str());
    return 0;
}

// CASE 6: total_summed_weights + parameter store update
static int test_case6_parameter_store()
{
    weight_type w;
    FixtureInfo info = build_fixture(w, true);

    std::string ref_file = make_temp_flag_file("case6");
    write_flag_file_bracketed(ref_file, info.row_fpday_lo, info.row_fpday_hi, "80");

    MHO_ParameterStore pstore;
    MHO_AdhocFlagging op;
    op.SetRefFlagFile(ref_file);
    op.SetParameterStore(&pstore);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    /* ch0 zeroed, ch1 kept.
       Per AP: 2 pols * 2 chans * 1 spec = 4 elements
       After zeroing ch0 (2 pols): 2 retained per AP
       Total: 4 APs * 2 = 8.0 */
    {
        double total_sw;
        REQUIRE(pstore.Get("/fringe/total_summed_weights", total_sw));
        CHECK_CLOSE(total_sw, 8.0, 1e-12);
    }
    {
        int total_naps;
        REQUIRE(pstore.Get("/config/total_naps", total_naps));
        REQUIRE(total_naps == 4);
    }

    std::remove(ref_file.c_str());
    return 0;
}

// CASE 7: Both files empty -> no-op
static int test_case7_both_empty()
{
    weight_type w;
    build_fixture(w, true);

    weight_type pristine;
    pristine.Copy(w);

    // Neither SetRefFlagFile nor SetRemFlagFile called
    MHO_AdhocFlagging op;
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Array unchanged; n_zeroed == 0 so recompute block is skipped
    for (std::size_t pp = 0; pp < 2; pp++)
        for (std::size_t ch = 0; ch < 2; ch++)
            for (std::size_t t = 0; t < 4; t++)
                CHECK_CLOSE(w(pp, ch, t, 0), pristine(pp, ch, t, 0), 1e-15);

    return 0;
}

// CASE 8: Missing "start" tag -> Initialize fails
static int test_case8_missing_start_tag()
{
    weight_type w;
    // Build fixture WITHOUT "start" tag
    FixtureInfo info = build_fixture(w, false);

    std::string ref_file = make_temp_flag_file("case8");
    write_flag_file_bracketed(ref_file, info.row_fpday_lo, info.row_fpday_hi, "FF");

    MHO_AdhocFlagging op;
    op.SetRefFlagFile(ref_file);
    op.SetArgs(&w);
    bool ok = op.Initialize();
    REQUIRE(ok == false);

    std::remove(ref_file.c_str());
    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_case1_decode_hex_token_80()) return 1;
    if (test_case2_byte_55()) return 1;
    if (test_case3_ref_and_rem()) return 1;
    if (test_case4_empty_ref()) return 1;
    if (test_case5_time_out_of_range()) return 1;
    if (test_case6_parameter_store()) return 1;
    if (test_case7_both_empty()) return 1;
    if (test_case8_missing_start_tag()) return 1;

    return 0;
}
