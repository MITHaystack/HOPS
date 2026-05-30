#include <cmath>
#include <iostream>
#include <string>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Fixture builder

static void build_fixture(uch_weight_store_type& uw,
                          std::size_t npp, std::size_t nap,
                          std::size_t nchan, std::size_t nspec)
{
    std::size_t total_freq = nchan * nspec;
    uw.Resize(npp, nap, total_freq);

    // Fill data per the spec:
    //   For channel ch, lower index L=ch*nspec.
    //   (*uw)(0, t, f) = 10.0*ch + double(t) + 0.5*(f - L);
    //   so the value at lower_index (f==L) is 10*ch + t.
    for (std::size_t t = 0; t < nap; t++)
    {
        for (std::size_t f = 0; f < total_freq; f++)
        {
            // determine which channel this frequency belongs to
            std::size_t ch = f / nspec;
            std::size_t L  = ch * nspec;
            uw(0, t, f) = 10.0f * (float)ch + (float)t + 0.5f * (float)(f - L);
        }
    }

    // Set polprod axis label
    std::get<UCH_POLPROD_AXIS>(uw).at(0) = "XX";

    // Set monotonically increasing frequency coordinates
    const double base_freq = 8000.0;
    const double spacing   = 8.0;
    for (std::size_t f = 0; f < total_freq; f++)
    {
        std::get<UCH_FREQ_AXIS>(uw).at(f) = base_freq + f * spacing;
    }

    // Attach interval labels for each channel (all USB)
    for (std::size_t ch = 0; ch < nchan; ch++)
    {
        std::size_t lower = ch * nspec;
        std::size_t upper = lower + nspec;

        double sky_freq = 8000.0 + 1000.0 * (double)ch;

        mho_json label;
        label["sky_freq"]       = sky_freq;
        label["bandwidth"]      = 32.0;
        label["net_sideband"]   = "U";
        label["channel"]        = (int)ch;
        label["lower_index"]    = (int)lower;
        label["upper_index"]    = (int)upper;
        label["chan_id"]        = "ch" + std::to_string(ch);
        label["frequency_band"] = "X";

        std::get<UCH_FREQ_AXIS>(uw).SetIntervalLabelObject(label, lower, upper);
    }
}

// Case 1: Output shape -- frequency axis collapses to 1

static int test_output_shape()
{
    uch_weight_store_type uw;
    weight_store_type w;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    build_fixture(uw, npp, nap, nchan, nspec);

    MHO_WeightChannelizer wc;
    wc.SetArgs(&uw, &w);
    REQUIRE(wc.Initialize());
    REQUIRE(wc.Execute());

    std::size_t dim[weight_store_type::rank::value];
    w.GetDimensions(dim);
    REQUIRE(dim[POLPROD_AXIS] == npp);
    REQUIRE(dim[CHANNEL_AXIS] == nchan);
    REQUIRE(dim[TIME_AXIS]    == nap);
    REQUIRE(dim[FREQ_AXIS]    == 1);  // must collapse to 1

    return 0;
}

// Case 2: Retained weight value == input at lower_index

static int test_retained_value()
{
    uch_weight_store_type uw;
    weight_store_type w;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    build_fixture(uw, npp, nap, nchan, nspec);

    MHO_WeightChannelizer wc;
    wc.SetArgs(&uw, &w);
    REQUIRE(wc.Initialize());
    REQUIRE(wc.Execute());

    // w(0,ch,t,0) == uw(0,t,ch*nspec) == 10*ch + t
    for (std::size_t ch = 0; ch < nchan; ch++)
    {
        for (std::size_t t = 0; t < nap; t++)
        {
            std::size_t L = ch * nspec;
            weight_element_store_type expected = uw(0, t, L);
            weight_element_store_type actual   = w(0, ch, t, 0);
            REQUIRE(std::abs(actual - expected) < 1e-6);
        }
    }

    return 0;
}

// Case 3: Channel count and channel-axis labels

static int test_channel_labels()
{
    uch_weight_store_type uw;
    weight_store_type w;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    build_fixture(uw, npp, nap, nchan, nspec);

    MHO_WeightChannelizer wc;
    wc.SetArgs(&uw, &w);
    REQUIRE(wc.Initialize());
    REQUIRE(wc.Execute());

    auto* chax = &(std::get<CHANNEL_AXIS>(w));

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

// Case 4: Empty input rejected

static int test_empty_input()
{
    uch_weight_store_type uw;  // default constructed, size 0
    weight_store_type w;

    MHO_WeightChannelizer wc;
    wc.SetArgs(&uw, &w);
    bool ok = wc.Initialize();
    REQUIRE(ok == false);

    return 0;
}

// Case 5: Re-use / idempotency

static int test_idempotency()
{
    uch_weight_store_type uw;
    weight_store_type w;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    build_fixture(uw, npp, nap, nchan, nspec);

    MHO_WeightChannelizer wc;
    wc.SetArgs(&uw, &w);
    REQUIRE(wc.Initialize());
    REQUIRE(wc.Execute());

    // sample elements after first run
    weight_element_store_type v000 = w(0, 0, 0, 0);
    weight_element_store_type v112 = w(0, 1, 2, 0);
    double ch0_freq = std::get<CHANNEL_AXIS>(w).at(0);

    // second run
    REQUIRE(wc.Initialize());
    REQUIRE(wc.Execute());

    // outputs must be identical
    REQUIRE(w(0, 0, 0, 0) == v000);
    REQUIRE(w(0, 1, 2, 0) == v112);
    REQUIRE(std::abs(std::get<CHANNEL_AXIS>(w).at(0) - ch0_freq) < 1e-9);

    return 0;
}

// Case 6: Sum-of-retained-weights conservation

static int test_sum_conservation()
{
    uch_weight_store_type uw;
    weight_store_type w;

    const std::size_t npp   = 1;
    const std::size_t nap   = 3;
    const std::size_t nchan = 2;
    const std::size_t nspec = 4;

    build_fixture(uw, npp, nap, nchan, nspec);

    MHO_WeightChannelizer wc;
    wc.SetArgs(&uw, &w);
    REQUIRE(wc.Initialize());
    REQUIRE(wc.Execute());

    // Sum of output weights should equal sum of input weights at lower_index
    float sum_output = 0.0f;
    float sum_input  = 0.0f;

    for (std::size_t ch = 0; ch < nchan; ch++)
    {
        for (std::size_t t = 0; t < nap; t++)
        {
            std::size_t L = ch * nspec;
            sum_output += w(0, ch, t, 0);
            sum_input  += uw(0, t, L);
        }
    }

    REQUIRE(std::abs(sum_output - sum_input) < 1e-6);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_output_shape())      return 1;
    if (test_retained_value())    return 1;
    if (test_channel_labels())    return 1;
    if (test_empty_input())       return 1;
    if (test_idempotency())       return 1;
    if (test_sum_conservation())  return 1;

    return 0;
}
