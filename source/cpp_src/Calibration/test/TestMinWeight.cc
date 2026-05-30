#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

#include "MHO_MinWeight.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Helpers

/* Build the standard fixture: [POLPROD=1, CHANNEL=2, TIME=2, FREQ=2] = 8 elems.
   Fill order (flat): {0.0, 0.1, 0.25, 0.5, 0.75, 1.0, 0.2, 0.49}. */
static void build_fixture(weight_type& w, const std::vector<double>& values)
{
    std::size_t dim[VIS_NDIM] = {1, 2, 2, 2};
    w.Resize(dim);
    w.ZeroArray();

    std::size_t idx = 0;
    for (auto it = w.begin(); it != w.end(); ++it) {
        *it = values[idx++];
    }
}

static const std::vector<double> default_values(
    {0.0, 0.1, 0.25, 0.5, 0.75, 1.0, 0.2, 0.49});

// Test cases

// CASE 1 - Default threshold (0.0) zeroes nothing (strict <)
static int test_default_threshold()
{
    weight_type w;
    build_fixture(w, default_values);

    // Save originals
    std::vector<double> original;
    for (auto it = w.begin(); it != w.end(); ++it)
        original.push_back(*it);

    MHO_MinWeight op;
    // No SetMinWeight call - default fMinWeight == 0.0
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // All elements must be unchanged (none are strictly < 0.0)
    std::size_t idx = 0;
    for (auto it = w.begin(); it != w.end(); ++it) {
        CHECK_CLOSE(*it, original[idx], 1e-15);
        ++idx;
    }

    return 0;
}

// CASE 2 - Threshold 0.5 zeroes elements strictly below 0.5
static int test_threshold_half()
{
    weight_type w;
    build_fixture(w, default_values);

    // Expected: {0.0, 0.0, 0.0, 0.5, 0.75, 1.0, 0.0, 0.0}
    std::vector<double> expected = {0.0, 0.0, 0.0, 0.5, 0.75, 1.0, 0.0, 0.0};

    MHO_MinWeight op;
    op.SetMinWeight(0.5);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    std::size_t idx = 0;
    for (auto it = w.begin(); it != w.end(); ++it) {
        CHECK_CLOSE(*it, expected[idx], 1e-12);
        ++idx;
    }

    return 0;
}

// CASE 3 - Boundary exactly at threshold is preserved
static int test_boundary_at_threshold()
{
    weight_type w;
    std::vector<double> vals(8, 0.7);  // all elements 0.7
    build_fixture(w, vals);

    MHO_MinWeight op;
    op.SetMinWeight(0.7);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // All elements remain 0.7 (0.7 < 0.7 is false)
    for (auto it = w.begin(); it != w.end(); ++it) {
        CHECK_CLOSE(*it, 0.7, 1e-12);
    }

    return 0;
}

// CASE 4 - Threshold above all values zeroes everything
static int test_threshold_above_all()
{
    weight_type w;
    build_fixture(w, default_values);

    MHO_MinWeight op;
    op.SetMinWeight(2.0);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    for (auto it = w.begin(); it != w.end(); ++it) {
        CHECK_CLOSE(*it, 0.0, 1e-12);
    }

    return 0;
}

// CASE 5 - Negative threshold keeps everything (including zeros)
static int test_negative_threshold()
{
    weight_type w;
    build_fixture(w, default_values);

    // Save originals
    std::vector<double> original;
    for (auto it = w.begin(); it != w.end(); ++it)
        original.push_back(*it);

    MHO_MinWeight op;
    op.SetMinWeight(-1.0);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Nothing zeroed; 0.0 < -1.0 is false, so 0.0 stays
    std::size_t idx = 0;
    for (auto it = w.begin(); it != w.end(); ++it) {
        CHECK_CLOSE(*it, original[idx], 1e-12);
        ++idx;
    }

    return 0;
}

// CASE 6 - Idempotency (re-Execute is stable)
static int test_idempotency()
{
    weight_type w;
    build_fixture(w, default_values);

    MHO_MinWeight op;
    op.SetMinWeight(0.5);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Capture state after first execute
    weight_type after_first;
    after_first.Copy(w);

    // Second execute
    REQUIRE(op.Execute());

    // Every element must be identical
    auto it1 = w.begin();
    auto it2 = after_first.begin();
    for (; it1 != w.end(); ++it1, ++it2) {
        CHECK_CLOSE(*it1, *it2, 1e-12);
    }

    return 0;
}

// CASE 7 - NaN handling: NaN < threshold is FALSE, so NaN is preserved
static int test_nan_handling()
{
    weight_type w;
    build_fixture(w, default_values);

    // Set the last element to NaN
    auto it_end = w.end();
    --it_end;
    *it_end = std::numeric_limits<double>::quiet_NaN();

    MHO_MinWeight op;
    op.SetMinWeight(0.5);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // The NaN element should still be NaN (not zeroed)
    auto it_nan = w.end();
    --it_nan;
    REQUIRE(std::isnan(*it_nan));

    return 0;
}

// CASE 8 - Empty / zero-size array is a benign no-op
static int test_empty_array()
{
    weight_type w;
    std::size_t dim[VIS_NDIM] = {0, 2, 2, 2};  // zero dimension
    w.Resize(dim);
    w.ZeroArray();

    MHO_MinWeight op;
    op.SetMinWeight(0.5);
    op.SetArgs(&w);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // begin() == end(), no crash
    REQUIRE(w.begin() == w.end());

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_default_threshold())     return 1;
    if (test_threshold_half())        return 1;
    if (test_boundary_at_threshold()) return 1;
    if (test_threshold_above_all())   return 1;
    if (test_negative_threshold())    return 1;
    if (test_idempotency())           return 1;
    if (test_nan_handling())          return 1;
    if (test_empty_array())           return 1;

    return 0;
}
