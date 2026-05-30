#include <cmath>
#include <complex>
#include <iostream>

#include "MHO_CheckForNaN.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_NaNMasker.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static bool close(double a, double b, double atol = 1e-12)
{
    return std::fabs(a - b) <= atol;
}

//Fixture helpers

using arr1d_double = MHO_NDArrayWrapper< double, 1 >;
using arr1d_complex = MHO_NDArrayWrapper< std::complex< double >, 1 >;

static void fill_real_fixture(arr1d_double& arr)
{
    const double NaN = std::numeric_limits< double >::quiet_NaN();
    double vals[6] = {1.0, NaN, 3.0, NaN, -2.0, 0.0};
    for(std::size_t i = 0; i < 6; ++i)
        arr(i) = vals[i];
}

static void fill_complex_fixture(arr1d_complex& arr)
{
    const double NaN = std::numeric_limits< double >::quiet_NaN();
    arr(0) = std::complex< double >(1, 2);
    arr(1) = std::complex< double >(NaN, 0);
    arr(2) = std::complex< double >(0, NaN);
    arr(3) = std::complex< double >(NaN, NaN);
    arr(4) = std::complex< double >(-4, 5);
}

static void fill_inf_fixture(arr1d_double& arr)
{
    const double Inf = std::numeric_limits< double >::infinity();
    arr(0) = Inf;
    arr(1) = -Inf;
    arr(2) = 7.0;
}

//Test cases

//CASE 1 - Real array in-place masking
static int test_case1()
{
    std::size_t dim[1] = {6};
    arr1d_double arr(dim);
    fill_real_fixture(arr);

    using Broadcaster = MHO_FunctorBroadcaster< arr1d_double, MHO_NaNMasker< arr1d_double > >;
    Broadcaster op;
    op.SetArgs(&arr);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    double expected[6] = {1.0, 0.0, 3.0, 0.0, -2.0, 0.0};
    for(std::size_t i = 0; i < 6; ++i)
    {
        REQUIRE(close(arr(i), expected[i]));
        REQUIRE(!std::isnan(arr(i)));
    }

    return 0;
}

//CASE 2 - Real array out-of-place masking
static int test_case2()
{
    std::size_t dim[1] = {6};
    arr1d_double input(dim);
    arr1d_double output(dim);
    fill_real_fixture(input);

    // Save original NaN positions for verification
    bool inputNaN[6];
    for(std::size_t i = 0; i < 6; ++i)
        inputNaN[i] = std::isnan(input(i));

    using Broadcaster = MHO_FunctorBroadcaster< arr1d_double, MHO_NaNMasker< arr1d_double > >;
    Broadcaster op;
    op.SetArgs(&input, &output);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    double expected[6] = {1.0, 0.0, 3.0, 0.0, -2.0, 0.0};
    for(std::size_t i = 0; i < 6; ++i)
    {
        REQUIRE(close(output(i), expected[i]));
        REQUIRE(!std::isnan(output(i)));
    }

    // Input must still contain original NaNs
    for(std::size_t i = 0; i < 6; ++i)
        REQUIRE(std::isnan(input(i)) == inputNaN[i]);

    return 0;
}

//CASE 3 - Complex masking (NaN in either part triggers zeroing)
static int test_case3()
{
    std::size_t dim[1] = {5};
    arr1d_complex arr(dim);
    fill_complex_fixture(arr);

    using Broadcaster = MHO_FunctorBroadcaster< arr1d_complex, MHO_NaNMasker< arr1d_complex > >;
    Broadcaster op;
    op.SetArgs(&arr);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Elements 1, 2, 3 should be complex(0,0); elements 0, 4 unchanged
    std::complex< double > expected[5] = {std::complex< double >(1, 2), std::complex< double >(0, 0),
                                          std::complex< double >(0, 0), std::complex< double >(0, 0),
                                          std::complex< double >(-4, 5)};
    for(std::size_t i = 0; i < 5; ++i)
    {
        REQUIRE(close(arr(i).real(), expected[i].real()));
        REQUIRE(close(arr(i).imag(), expected[i].imag()));
        REQUIRE(!std::isnan(arr(i).real()));
        REQUIRE(!std::isnan(arr(i).imag()));
    }

    return 0;
}

//CASE 4 - No-NaN array is a no-op
static int test_case4()
{
    std::size_t dim[1] = {4};
    arr1d_double arr(dim);
    arr(0) = 1.0;
    arr(1) = 2.0;
    arr(2) = 3.0;
    arr(3) = 4.0;

    using Broadcaster = MHO_FunctorBroadcaster< arr1d_double, MHO_NaNMasker< arr1d_double > >;
    Broadcaster op;
    op.SetArgs(&arr);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(close(arr(0), 1.0));
    REQUIRE(close(arr(1), 2.0));
    REQUIRE(close(arr(2), 3.0));
    REQUIRE(close(arr(3), 4.0));

    return 0;
}

//CASE 5 - Inf passes through (NOT masked)
static int test_case5()
{
    std::size_t dim[1] = {3};
    arr1d_double arr(dim);
    fill_inf_fixture(arr);

    using Broadcaster = MHO_FunctorBroadcaster< arr1d_double, MHO_NaNMasker< arr1d_double > >;
    Broadcaster op;
    op.SetArgs(&arr);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(std::isinf(arr(0)) && arr(0) > 0);
    REQUIRE(std::isinf(arr(1)) && arr(1) < 0);
    REQUIRE(close(arr(2), 7.0));

    return 0;
}

//CASE 6 - Idempotency
static int test_case6()
{
    std::size_t dim[1] = {6};
    arr1d_double arr(dim);
    fill_real_fixture(arr);

    using Broadcaster = MHO_FunctorBroadcaster< arr1d_double, MHO_NaNMasker< arr1d_double > >;
    Broadcaster op;
    op.SetArgs(&arr);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Second application
    op.Initialize();
    REQUIRE(op.Execute());

    double expected[6] = {1.0, 0.0, 3.0, 0.0, -2.0, 0.0};
    for(std::size_t i = 0; i < 6; ++i)
        REQUIRE(close(arr(i), expected[i]));

    return 0;
}

//CASE 7 - Direct functor operator() (no broadcaster)
static int test_case7()
{
    const double NaN = std::numeric_limits< double >::quiet_NaN();

    // In-place: NaN element -> 0.0
    std::size_t dim[1] = {1};
    arr1d_double arr(dim);
    arr(0) = NaN;

    MHO_NaNMasker< arr1d_double > masker;
    auto it = arr.begin();
    masker(it);
    REQUIRE(close(arr(0), 0.0));
    REQUIRE(!std::isnan(arr(0)));

    // In-place: 5.0 element unchanged
    arr(0) = 5.0;
    it = arr.begin();
    masker(it);
    REQUIRE(close(arr(0), 5.0));

    // Out-of-place: NaN -> 0.0
    {
        std::size_t dim2[1] = {1};
        arr1d_double in(dim2);
        arr1d_double out(dim2);
        in(0) = NaN;
        auto cit = in.cbegin();
        auto oit = out.begin();
        masker(cit, oit);
        REQUIRE(close(out(0), 0.0));
        REQUIRE(!std::isnan(out(0)));
    }

    // Out-of-place: 5.0 -> 5.0 pass-through
    {
        std::size_t dim2[1] = {1};
        arr1d_double in(dim2);
        arr1d_double out(dim2);
        in(0) = 5.0;
        auto cit = in.cbegin();
        auto oit = out.begin();
        masker(cit, oit);
        REQUIRE(close(out(0), 5.0));
    }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if(test_case1())
        return 1;
    if(test_case2())
        return 1;
    if(test_case3())
        return 1;
    if(test_case4())
        return 1;
    if(test_case5())
        return 1;
    if(test_case6())
        return 1;
    if(test_case7())
        return 1;

    return 0;
}
