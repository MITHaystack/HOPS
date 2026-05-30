#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_DoubleSidebandChannelLabeler.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

static void set_chan(visibility_type& vis, std::size_t i,
                     double freq, double bw, const std::string& nsb)
{
    auto ax = &(std::get<CHANNEL_AXIS>(vis));
    ax->at(i) = freq;
    ax->InsertIndexLabelKeyValue(i, "bandwidth", bw);
    ax->InsertIndexLabelKeyValue(i, "net_sideband", nsb);
}

static visibility_type make_vis(std::size_t nchan)
{
    visibility_type vis;
    vis.Resize(4, nchan, 8, 8);
    return vis;
}

// Count "double_sideband" interval labels.
static std::size_t count_dsb(const channel_axis_type& ax)
{
    return ax.GetMatchingIntervalLabels("double_sideband").size();
}

// Try to retrieve dsb_partner on channel i; return true if found.
static bool get_dsb_partner(const channel_axis_type& ax, std::size_t i, int& out)
{
    return ax.RetrieveIndexLabelKeyValue(i, "dsb_partner", out);
}

// Case 1: Single LSB/USB DSB pair (happy path)

static int test_single_dsb_pair()
{
    visibility_type vis = make_vis(2);
    set_chan(vis, 0, 8000.0, 16.0, "L");
    set_chan(vis, 1, 8000.0, 16.0, "U");

    MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
    labeler.SetArgs(&vis);
    REQUIRE(labeler.Initialize());
    REQUIRE(labeler.Execute());

    auto& ax = std::get<CHANNEL_AXIS>(vis);
    REQUIRE(count_dsb(ax) == 1);

    int partner = 0;
    REQUIRE(get_dsb_partner(ax, 0, partner));
    REQUIRE(partner == 1);
    REQUIRE(get_dsb_partner(ax, 1, partner));
    REQUIRE(partner == -1);

    return 0;
}

// Case 2: Pure USB-only set (no DSB)

static int test_usb_only()
{
    visibility_type vis = make_vis(4);
    set_chan(vis, 0, 8000.0, 16.0, "U");
    set_chan(vis, 1, 8016.0, 16.0, "U");
    set_chan(vis, 2, 8032.0, 16.0, "U");
    set_chan(vis, 3, 8048.0, 16.0, "U");

    MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
    labeler.SetArgs(&vis);
    REQUIRE(labeler.Initialize());
    REQUIRE(labeler.Execute());

    auto& ax = std::get<CHANNEL_AXIS>(vis);
    REQUIRE(count_dsb(ax) == 0);

    int partner = 0;
    for (std::size_t i = 0; i < 4; i++)
    {
        REQUIRE(get_dsb_partner(ax, i, partner) == false);
    }

    return 0;
}

// Case 3: Pure LSB-only set (no DSB)

static int test_lsb_only()
{
    visibility_type vis = make_vis(4);
    set_chan(vis, 0, 8000.0, 16.0, "L");
    set_chan(vis, 1, 8016.0, 16.0, "L");
    set_chan(vis, 2, 8032.0, 16.0, "L");
    set_chan(vis, 3, 8048.0, 16.0, "L");

    MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
    labeler.SetArgs(&vis);
    REQUIRE(labeler.Initialize());
    REQUIRE(labeler.Execute());

    auto& ax = std::get<CHANNEL_AXIS>(vis);
    REQUIRE(count_dsb(ax) == 0);

    int partner = 0;
    for (std::size_t i = 0; i < 4; i++)
    {
        REQUIRE(get_dsb_partner(ax, i, partner) == false);
    }

    return 0;
}

// Case 4: Same-freq pair but WRONG sideband order (U then L)

static int test_wrong_order_ul()
{
    visibility_type vis = make_vis(2);
    set_chan(vis, 0, 8000.0, 16.0, "U");
    set_chan(vis, 1, 8000.0, 16.0, "L");

    MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
    labeler.SetArgs(&vis);
    REQUIRE(labeler.Initialize());
    REQUIRE(labeler.Execute());

    auto& ax = std::get<CHANNEL_AXIS>(vis);
    REQUIRE(count_dsb(ax) == 0);

    int partner = 0;
    REQUIRE(get_dsb_partner(ax, 0, partner) == false);
    REQUIRE(get_dsb_partner(ax, 1, partner) == false);

    return 0;
}

// Case 5: Same sideband order but mismatched bandwidth

static int test_mismatched_bandwidth()
{
    visibility_type vis = make_vis(2);
    set_chan(vis, 0, 8000.0, 16.0, "L");
    set_chan(vis, 1, 8000.0, 32.0, "U");

    MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
    labeler.SetArgs(&vis);
    REQUIRE(labeler.Initialize());
    REQUIRE(labeler.Execute());

    auto& ax = std::get<CHANNEL_AXIS>(vis);
    REQUIRE(count_dsb(ax) == 0);

    return 0;
}

// Case 6: Same sideband order but different sky frequency

static int test_different_freq()
{
    visibility_type vis = make_vis(2);
    set_chan(vis, 0, 8000.0, 16.0, "L");
    set_chan(vis, 1, 8016.0, 16.0, "U");

    MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
    labeler.SetArgs(&vis);
    REQUIRE(labeler.Initialize());
    REQUIRE(labeler.Execute());

    auto& ax = std::get<CHANNEL_AXIS>(vis);
    REQUIRE(count_dsb(ax) == 0);

    return 0;
}

// Case 7: Multiple adjacent DSB pairs

static int test_multiple_dsb_pairs()
{
    visibility_type vis = make_vis(4);
    set_chan(vis, 0, 8000.0, 16.0, "L");
    set_chan(vis, 1, 8000.0, 16.0, "U");
    set_chan(vis, 2, 8100.0, 16.0, "L");
    set_chan(vis, 3, 8100.0, 16.0, "U");

    MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
    labeler.SetArgs(&vis);
    REQUIRE(labeler.Initialize());
    REQUIRE(labeler.Execute());

    auto& ax = std::get<CHANNEL_AXIS>(vis);
    REQUIRE(count_dsb(ax) == 2);

    int partner = 0;
    REQUIRE(get_dsb_partner(ax, 0, partner));
    REQUIRE(partner == 1);
    REQUIRE(get_dsb_partner(ax, 1, partner));
    REQUIRE(partner == -1);
    REQUIRE(get_dsb_partner(ax, 2, partner));
    REQUIRE(partner == 1);
    REQUIRE(get_dsb_partner(ax, 3, partner));
    REQUIRE(partner == -1);

    return 0;
}

// Case 8: Tolerance boundary (fEps)

static int test_tolerance_boundary()
{
    // Sub-case 8a: default fEps=1e-6, freq diff 5e-7 < 1e-6 -> DSB
    {
        visibility_type vis = make_vis(2);
        set_chan(vis, 0, 8000.0, 16.0, "L");
        set_chan(vis, 1, 8000.0000005, 16.0, "U");

        MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
        labeler.SetArgs(&vis);
        REQUIRE(labeler.Initialize());
        REQUIRE(labeler.Execute());

        auto& ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(count_dsb(ax) == 1);
    }

    // Sub-case 8b: freq diff 1e-3 > 1e-6 -> NOT DSB
    {
        visibility_type vis = make_vis(2);
        set_chan(vis, 0, 8000.0, 16.0, "L");
        set_chan(vis, 1, 8000.001, 16.0, "U");

        MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
        labeler.SetArgs(&vis);
        REQUIRE(labeler.Initialize());
        REQUIRE(labeler.Execute());

        auto& ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(count_dsb(ax) == 0);
    }

    // Sub-case 8c: SetTolerance(1.0) -> freq diff 1e-3 < 1.0 -> DSB
    {
        visibility_type vis = make_vis(2);
        set_chan(vis, 0, 8000.0, 16.0, "L");
        set_chan(vis, 1, 8000.001, 16.0, "U");

        MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
        labeler.SetTolerance(1.0);
        labeler.SetArgs(&vis);
        REQUIRE(labeler.Initialize());
        REQUIRE(labeler.Execute());

        auto& ax = std::get<CHANNEL_AXIS>(vis);
        REQUIRE(count_dsb(ax) == 1);
    }

    return 0;
}

// Case 9: Missing bandwidth / net_sideband labels

static int test_missing_labels()
{
    visibility_type vis = make_vis(2);
    auto ax = &(std::get<CHANNEL_AXIS>(vis));
    ax->at(0) = 8000.0;
    ax->at(1) = 8000.0;
    // No bandwidth or net_sideband labels set

    MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
    labeler.SetArgs(&vis);
    REQUIRE(labeler.Initialize());
    REQUIRE(labeler.Execute());

    REQUIRE(count_dsb(*ax) == 0);

    return 0;
}

// Case 10: Single channel (nchans == 1)

static int test_single_channel()
{
    visibility_type vis = make_vis(1);
    set_chan(vis, 0, 8000.0, 16.0, "L");

    MHO_DoubleSidebandChannelLabeler<visibility_type> labeler;
    labeler.SetArgs(&vis);
    REQUIRE(labeler.Initialize());
    REQUIRE(labeler.Execute());

    auto& ax = std::get<CHANNEL_AXIS>(vis);
    REQUIRE(count_dsb(ax) == 0);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_single_dsb_pair())        return 1;
    if (test_usb_only())               return 1;
    if (test_lsb_only())               return 1;
    if (test_wrong_order_ul())         return 1;
    if (test_mismatched_bandwidth())   return 1;
    if (test_different_freq())         return 1;
    if (test_multiple_dsb_pairs())     return 1;
    if (test_tolerance_boundary())     return 1;
    if (test_missing_labels())         return 1;
    if (test_single_channel())         return 1;

    return 0;
}
