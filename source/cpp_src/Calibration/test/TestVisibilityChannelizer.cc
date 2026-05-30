#include <cmath>
#include <iostream>
#include <string>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

static void build_fixture(uch_visibility_store_type& uch,
                          std::size_t npp, std::size_t nap,
                          std::size_t nchan, std::size_t nspec,
                          bool ascending_freq)
{
    std::size_t total_freq = nchan * nspec;
    uch.Resize(npp, nap, total_freq);

    // fill data: real = global freq index, imag = time index
    for (std::size_t t = 0; t < nap; t++)
    {
        for (std::size_t f = 0; f < total_freq; f++)
        {
            uch(0, t, f) = std::complex<float>(static_cast<float>(f),
                                                static_cast<float>(t));
        }
    }

    // set polprod axis label
    std::get<UCH_POLPROD_AXIS>(uch).at(0) = "XX";

    // set monotonically increasing frequency coordinates
    const double base_freq = 8000.0;
    const double spacing   = 8.0;
    for (std::size_t f = 0; f < total_freq; f++)
    {
        std::get<UCH_FREQ_AXIS>(uch).at(f) = base_freq + f * spacing;
    }

    // attach interval labels for each channel
    for (std::size_t ch = 0; ch < nchan; ch++)
    {
        std::size_t lower = ch * nspec;
        std::size_t upper = lower + nspec;

        // Determine sky_freq assignment based on ascending_freq flag.
        // When ascending_freq == true:  ch 0 -> 8000, ch 1 -> 9000, ...
        // When ascending_freq == false: ch 0 -> 9000, ch 1 -> 8000, ...
        double sky_freq;
        if (ascending_freq)
        {
            sky_freq = 8000.0 + 1000.0 * (double)ch;
        }
        else
        {
            // descending: first input channel gets the highest freq
            sky_freq = 8000.0 + 1000.0 * (double)(nchan - 1 - ch);
        }

        // The "channel" value is the sorted-by-frequency position,
        // which is the key the channelizer uses for lookup.
        int channel_key;
        if (ascending_freq)
        {
            channel_key = (int)ch;
        }
        else
        {
            channel_key = (int)(nchan - 1 - ch);
        }

        mho_json label;
        label["sky_freq"]       = sky_freq;
        label["bandwidth"]      = 32.0;
        label["net_sideband"]   = "U";
        label["channel"]        = channel_key;
        label["lower_index"]    = (int)lower;
        label["upper_index"]    = (int)upper;
        label["chan_id"]        = "ch" + std::to_string(channel_key);
        label["frequency_band"] = "X";

        std::get<UCH_FREQ_AXIS>(uch).SetIntervalLabelObject(label, lower, upper);
    }
}

// Case 1: Output dimensions / channelization shape

static int test_dimensions()
{
    uch_visibility_store_type uch;
    visibility_store_type vis;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    build_fixture(uch, npp, nap, nchan, nspec, true);

    MHO_VisibilityChannelizer chan;
    chan.SetArgs(&uch, &vis);
    REQUIRE(chan.Initialize());
    REQUIRE(chan.Execute());

    std::size_t dim[visibility_store_type::rank::value];
    vis.GetDimensions(dim);
    REQUIRE(dim[POLPROD_AXIS] == npp);
    REQUIRE(dim[CHANNEL_AXIS] == nchan);
    REQUIRE(dim[TIME_AXIS]    == nap);
    REQUIRE(dim[FREQ_AXIS]    == nspec);

    return 0;
}

// Case 2: Data placement correctness

static int test_data_placement()
{
    uch_visibility_store_type uch;
    visibility_store_type vis;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    build_fixture(uch, npp, nap, nchan, nspec, true);

    MHO_VisibilityChannelizer chan;
    chan.SetArgs(&uch, &vis);
    REQUIRE(chan.Initialize());
    REQUIRE(chan.Execute());

    // verify: vis(0,ch,t,fo) == uch(0,t, ch*nspec+fo)
    // ascending_freq: ch 0 has sky_freq 8000 (indices 0..3),
    //                 ch 1 has sky_freq 9000 (indices 4..7)
    for (std::size_t ch = 0; ch < nchan; ch++)
    {
        for (std::size_t t = 0; t < nap; t++)
        {
            for (std::size_t fo = 0; fo < nspec; fo++)
            {
                std::size_t global_f = ch * nspec + fo;
                visibility_element_store_type expected = uch(0, t, global_f);
                visibility_element_store_type actual   = vis(0, ch, t, fo);
                REQUIRE(actual == expected);
            }
        }
    }

    return 0;
}

// Case 3: Channel-axis labels propagated

static int test_channel_labels()
{
    uch_visibility_store_type uch;
    visibility_store_type vis;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    build_fixture(uch, npp, nap, nchan, nspec, true);

    MHO_VisibilityChannelizer chan;
    chan.SetArgs(&uch, &vis);
    REQUIRE(chan.Initialize());
    REQUIRE(chan.Execute());

    auto* chax = &(std::get<CHANNEL_AXIS>(vis));

    for (std::size_t ch = 0; ch < nchan; ch++)
    {
        double expected_sky_freq = 8000.0 + 1000.0 * (double)ch;
        mho_json lbl = chax->GetLabelObject(ch);

        REQUIRE(std::abs(lbl["sky_freq"].get<double>() - expected_sky_freq) < 1e-9);
        REQUIRE(lbl["net_sideband"].get<std::string>() == "U");
        REQUIRE(lbl["chan_id"].get<std::string>() == "ch" + std::to_string(ch));
        REQUIRE(lbl["index"].get<int>() == (int)ch);

        // channel-axis coordinate value
        REQUIRE(std::abs(chax->at(ch) - expected_sky_freq) < 1e-9);
    }

    return 0;
}

// Case 4: Channel sort order (descending sky_freq on input)

static int test_channel_sort_order()
{
    uch_visibility_store_type uch;
    visibility_store_type vis;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    // descending_freq: input ch 0 -> sky_freq 9000, input ch 1 -> sky_freq 8000
    build_fixture(uch, npp, nap, nchan, nspec, false);

    MHO_VisibilityChannelizer chan;
    chan.SetArgs(&uch, &vis);
    REQUIRE(chan.Initialize());
    REQUIRE(chan.Execute());

    auto* chax = &(std::get<CHANNEL_AXIS>(vis));

    // Output channel 0 should be sky_freq 8000, channel 1 should be 9000
    REQUIRE(std::abs(chax->at(0) - 8000.0) < 1e-9);
    REQUIRE(std::abs(chax->at(1) - 9000.0) < 1e-9);

    // Verify monotonically non-decreasing
    for (std::size_t ch = 1; ch < nchan; ch++)
    {
        REQUIRE(chax->at(ch) >= chax->at(ch - 1) - 1e-9);
    }

    // Data for output channel 0 should come from the input whose "channel"==0,
    // which is the 8000 channel (input indices 4..7, lower_index=4).
    for (std::size_t t = 0; t < nap; t++)
    {
        for (std::size_t fo = 0; fo < nspec; fo++)
        {
            // The 8000 channel has "channel"=0 and occupies freq indices 4..7
            std::size_t global_f = 4 + fo;
            visibility_element_store_type expected = uch(0, t, global_f);
            visibility_element_store_type actual   = vis(0, 0, t, fo);
            REQUIRE(actual == expected);
        }
    }

    return 0;
}

// Case 5: Empty input rejected

static int test_empty_input()
{
    uch_visibility_store_type uch;  // default constructed, size 0
    visibility_store_type vis;

    MHO_VisibilityChannelizer chan;
    chan.SetArgs(&uch, &vis);
    bool ok = chan.Initialize();
    REQUIRE(ok == false);

    // Execute should also fail
    bool exec_ok = chan.Execute();
    REQUIRE(exec_ok == false);

    return 0;
}

// Case 6: Re-use / idempotency

static int test_idempotency()
{
    uch_visibility_store_type uch;
    visibility_store_type vis;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    build_fixture(uch, npp, nap, nchan, nspec, true);

    MHO_VisibilityChannelizer chan;
    chan.SetArgs(&uch, &vis);
    REQUIRE(chan.Initialize());
    REQUIRE(chan.Execute());

    // sample a few elements after first run
    visibility_element_store_type v0000 = vis(0, 0, 0, 0);
    visibility_element_store_type v1123 = vis(0, 1, 2, 3);
    double ch0_freq = std::get<CHANNEL_AXIS>(vis).at(0);

    // second run on the same operator+inputs
    REQUIRE(chan.Initialize());
    REQUIRE(chan.Execute());

    // outputs must be identical
    REQUIRE(vis(0, 0, 0, 0) == v0000);
    REQUIRE(vis(0, 1, 2, 3) == v1123);
    REQUIRE(std::abs(std::get<CHANNEL_AXIS>(vis).at(0) - ch0_freq) < 1e-9);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eWarning);

    if (test_dimensions())         return 1;
    if (test_data_placement())     return 1;
    if (test_channel_labels())     return 1;
    if (test_channel_sort_order()) return 1;
    if (test_empty_input())        return 1;
    if (test_idempotency())        return 1;

    return 0;
}
