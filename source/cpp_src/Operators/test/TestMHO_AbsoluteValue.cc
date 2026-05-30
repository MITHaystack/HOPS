#include <cmath>
#include <complex>
#include <iostream>

#include "MHO_AbsoluteValue.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static bool close(double a, double b, double atol = 1e-12)
{
    return std::fabs(a - b) <= atol;
}

//Fixture helpers

using arr1d_double = MHO_NDArrayWrapper< double, 1 >;
using arr2d_complex = MHO_NDArrayWrapper< std::complex< double >, 2 >;

static void fill_real_fixture(arr1d_double& arr)
{
    double vals[5] = {-3.0, 0.0, 2.5, -7.25, 4.0};
    for(std::size_t i = 0; i < 5; ++i)
        arr(i) = vals[i];
}

static void fill_complex_fixture(arr2d_complex& arr)
{
    // 2x3 array with Pythagorean triples for exact magnitudes
    arr(0, 0) = std::complex< double >(3, 4);   // |.| = 5
    arr(0, 1) = std::complex< double >(-3, -4); // |.| = 5
    arr(0, 2) = std::complex< double >(0, 0);   // |.| = 0
    arr(1, 0) = std::complex< double >(5, 12);  // |.| = 13
    arr(1, 1) = std::complex< double >(-8, 15); // |.| = 17
    arr(1, 2) = std::complex< double >(1, 0);   // |.| = 1
}

//Test cases

//CASE 1 - Real array, out-of-place
static int test_case1()
{
    std::size_t dim[1] = {5};
    arr1d_double input(dim);
    arr1d_double output(dim);
    fill_real_fixture(input);

    double expected[5] = {3.0, 0.0, 2.5, 7.25, 4.0};
    // Save original for unchanged check
    double original[5];
    for(std::size_t i = 0; i < 5; ++i)
        original[i] = input(i);

    using Broadcaster = MHO_FunctorBroadcaster< arr1d_double, MHO_AbsoluteValue< arr1d_double > >;
    Broadcaster op;
    op.SetArgs(&input, &output);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    for(std::size_t i = 0; i < 5; ++i)
        REQUIRE(close(output(i), expected[i]));

    // Input must be unchanged
    for(std::size_t i = 0; i < 5; ++i)
        REQUIRE(close(input(i), original[i]));

    return 0;
}

//CASE 2 - Real array, in-place
static int test_case2()
{
    std::size_t dim[1] = {5};
    arr1d_double input(dim);
    fill_real_fixture(input);

    double expected[5] = {3.0, 0.0, 2.5, 7.25, 4.0};

    using Broadcaster = MHO_FunctorBroadcaster< arr1d_double, MHO_AbsoluteValue< arr1d_double > >;
    Broadcaster op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    for(std::size_t i = 0; i < 5; ++i)
        REQUIRE(close(input(i), expected[i]));

    return 0;
}

//CASE 3 - Complex magnitudes (Pythagorean exactness), out-of-place
static int test_case3()
{
    std::size_t dim[2] = {2, 3};
    arr2d_complex input(dim);
    arr2d_complex output(dim);
    fill_complex_fixture(input);

    double expected_mag[2][3] = {
        {5,  5,  0},
        {13, 17, 1}
    };

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_complex, MHO_AbsoluteValue< arr2d_complex > >;
    Broadcaster op;
    op.SetArgs(&input, &output);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Check real part matches magnitude; imaginary part is zeroed
    for(std::size_t i = 0; i < 2; ++i)
        for(std::size_t j = 0; j < 3; ++j)
        {
            REQUIRE(close(output(i, j).real(), expected_mag[i][j]));
            REQUIRE(std::fabs(output(i, j).imag()) <= 1e-12);
        }

    return 0;
}

//CASE 4 - Idempotency on real array
static int test_case4()
{
    std::size_t dim[1] = {5};
    arr1d_double input(dim);
    fill_real_fixture(input);

    using Broadcaster = MHO_FunctorBroadcaster< arr1d_double, MHO_AbsoluteValue< arr1d_double > >;
    Broadcaster op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    double after_first[5];
    for(std::size_t i = 0; i < 5; ++i)
        after_first[i] = input(i);

    // Second application
    op.Initialize();
    REQUIRE(op.Execute());

    // Should be identical to first application result
    for(std::size_t i = 0; i < 5; ++i)
        REQUIRE(close(input(i), after_first[i]));

    return 0;
}

//CASE 5 - Idempotency on complex array
static int test_case5()
{
    std::size_t dim[2] = {2, 3};
    arr2d_complex input(dim);
    fill_complex_fixture(input);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_complex, MHO_AbsoluteValue< arr2d_complex > >;
    Broadcaster op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Store after first pass
    std::complex< double > after_first[2][3];
    for(std::size_t i = 0; i < 2; ++i)
        for(std::size_t j = 0; j < 3; ++j)
            after_first[i][j] = input(i, j);

    // Second application
    op.Initialize();
    REQUIRE(op.Execute());

    // After first pass elements are complex(mag, 0); |complex(mag,0)| = mag again
    for(std::size_t i = 0; i < 2; ++i)
        for(std::size_t j = 0; j < 3; ++j)
        {
            REQUIRE(close(input(i, j).real(), after_first[i][j].real()));
            REQUIRE(std::fabs(input(i, j).imag()) <= 1e-12);
        }

    return 0;
}

//CASE 6 - Direct functor operator() (no broadcaster)
static int test_case6()
{
    // Real: single element -9.0 -> 9.0
    std::size_t dim[1] = {1};
    arr1d_double rarr(dim);
    rarr(0) = -9.0;

    MHO_AbsoluteValue< arr1d_double > rop;
    auto rit = rarr.begin();
    rop(rit);
    REQUIRE(close(rarr(0), 9.0));

    // Also exercise out-of-place: rop(cit, it)
    std::size_t dim2[1] = {1};
    arr1d_double rin(dim2);
    arr1d_double rout(dim2);
    rin(0) = -9.0;
    auto rcit = rin.cbegin();
    auto rit2 = rout.begin();
    rop(rcit, rit2);
    REQUIRE(close(rout(0), 9.0));

    // Complex: single element (6,8) -> (10,0)
    std::size_t cdim[2] = {1, 1};
    arr2d_complex carr(cdim);
    carr(0, 0) = std::complex< double >(6, 8);

    MHO_AbsoluteValue< arr2d_complex > cop;
    auto cit = carr.begin();
    cop(cit);
    REQUIRE(close(carr(0, 0).real(), 10.0));
    REQUIRE(std::fabs(carr(0, 0).imag()) <= 1e-12);

    return 0;
}

//CASE 7 (edge) - zero element
static int test_case7()
{
    // Real zero
    std::size_t rdim[1] = {1};
    arr1d_double rarr(rdim);
    rarr(0) = 0.0;

    MHO_AbsoluteValue< arr1d_double > rop;
    auto rit = rarr.begin();
    rop(rit);
    REQUIRE(close(rarr(0), 0.0));

    // Complex zero
    std::size_t cdim[2] = {1, 1};
    arr2d_complex carr(cdim);
    carr(0, 0) = std::complex< double >(0, 0);

    MHO_AbsoluteValue< arr2d_complex > cop;
    auto cit = carr.begin();
    cop(cit);
    REQUIRE(close(carr(0, 0).real(), 0.0));
    REQUIRE(std::fabs(carr(0, 0).imag()) <= 1e-12);

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
