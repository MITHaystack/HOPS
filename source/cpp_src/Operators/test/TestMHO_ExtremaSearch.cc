#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>
#include <limits>

#include "MHO_ExtremaSearch.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static bool close(double a, double b, double atol = 1e-12)
{
    return std::fabs(a - b) <= atol;
}

using arr1d_double = MHO_NDArrayWrapper< double, 1 >;
using arr2d_double = MHO_NDArrayWrapper< double, 2 >;
using arr2d_complex = MHO_NDArrayWrapper< std::complex< double >, 2 >;

//CASE 1 - 1-D real max/min value and location
static int test_case1()
{
    std::size_t dim[1] = {6};
    arr1d_double input(dim);
    double vals[6] = {3, 1, 4, 1, 5, 9};
    for(std::size_t i = 0; i < 6; ++i)
        input(i) = vals[i];

    MHO_ExtremaSearch< arr1d_double > op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(close(op.GetMax(), 9.0));
    REQUIRE(op.GetMaxLocation() == 5);
    REQUIRE(close(op.GetMin(), 1.0));
    REQUIRE(op.GetMinLocation() == 1); // first occurrence

    return 0;
}

//CASE 2 - 2-D real extrema and flat offsets
static int test_case2()
{
    std::size_t dim[2] = {3, 4};
    arr2d_double input(dim);
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
            input(i, j) = 2.0;
    input(1, 2) = 10.0; // max  flat offset = 1*4+2 = 6
    input(2, 0) = -3.0; // min  flat offset = 2*4+0 = 8

    MHO_ExtremaSearch< arr2d_double > op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(close(op.GetMax(), 10.0));
    REQUIRE(op.GetMaxLocation() == 6);
    REQUIRE(close(op.GetMin(), -3.0));
    REQUIRE(op.GetMinLocation() == 8);

    return 0;
}

//CASE 3 - Complex magnitude extrema
static int test_case3()
{
    std::size_t dim[2] = {3, 4};
    arr2d_complex input(dim);
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
            input(i, j) = std::complex< double >(1, 0); // mag 1
    input(0, 3) = std::complex< double >(0, 6);         // mag 6  flat = 0*4+3 = 3
    input(2, 2) = std::complex< double >(0.1, 0);       // mag 0.1  flat = 2*4+2 = 10

    MHO_ExtremaSearch< arr2d_complex > op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(close(op.GetMax(), 6.0));
    REQUIRE(op.GetMaxLocation() == 3);
    REQUIRE(close(op.GetMin(), 0.1));
    REQUIRE(op.GetMinLocation() == 10);

    return 0;
}

//CASE 4 - Single-element array
static int test_case4()
{
    std::size_t dim[2] = {1, 1};
    arr2d_double input(dim);
    input(0, 0) = 7.0;

    MHO_ExtremaSearch< arr2d_double > op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(close(op.GetMax(), 7.0));
    REQUIRE(close(op.GetMin(), 7.0));
    REQUIRE(op.GetMaxLocation() == 0);
    REQUIRE(op.GetMinLocation() == 0);

    return 0;
}

//CASE 5 - All-equal array
static int test_case5()
{
    std::size_t dim[1] = {5};
    arr1d_double input(dim);
    for(std::size_t i = 0; i < 5; ++i)
        input(i) = 2.0;

    MHO_ExtremaSearch< arr1d_double > op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // 2.0 > numeric_limits::lowest() so fMax updates on first element.
    // 2.0 < numeric_limits::max() so fMin updates on first element.
    // Subsequent equal values do not update (strict comparison).
    REQUIRE(close(op.GetMax(), 2.0));
    REQUIRE(op.GetMaxLocation() == 0);
    REQUIRE(close(op.GetMin(), 2.0));
    REQUIRE(op.GetMinLocation() == 0);

    return 0;
}

//CASE 6 - all-negative real array
//
//  Regression test for the fMax-seed fix: fMax is now seeded with
//  std::numeric_limits<double>::lowest() (most negative) instead of min()
//  (smallest positive normal).  The maximum of an all-negative array is
//  therefore found correctly.

static int test_case6()
{
    std::size_t dim[1] = {4};
    arr1d_double input(dim);
    double vals[4] = {-5, -2, -8, -3};
    for(std::size_t i = 0; i < 4; ++i)
        input(i) = vals[i];

    MHO_ExtremaSearch< arr1d_double > op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // max of an all-negative array is the least-negative element
    REQUIRE(close(op.GetMax(), -2.0));
    REQUIRE(op.GetMaxLocation() == 1);
    REQUIRE(close(op.GetMin(), -8.0));
    REQUIRE(op.GetMinLocation() == 2);

    return 0;
}

//CASE 7 - Idempotency / re-use
static int test_case7()
{
    // First run: 1-D array from case 1
    std::size_t dim1[1] = {6};
    arr1d_double input1(dim1);
    double vals1[6] = {3, 1, 4, 1, 5, 9};
    for(std::size_t i = 0; i < 6; ++i)
        input1(i) = vals1[i];

    MHO_ExtremaSearch< arr1d_double > op1;
    op1.SetArgs(&input1);
    REQUIRE(op1.Initialize());
    REQUIRE(op1.Execute());

    // (Results from first run not checked here -- we care about the second run)

    // Second run: 2-D array from case 2 on a fresh object of the same type
    // Reuse pattern: create a second object for the different array type.
    std::size_t dim2[2] = {3, 4};
    arr2d_double input2(dim2);
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
            input2(i, j) = 2.0;
    input2(1, 2) = 10.0;
    input2(2, 0) = -3.0;

    MHO_ExtremaSearch< arr2d_double > op2;
    op2.SetArgs(&input2);
    REQUIRE(op2.Initialize());
    REQUIRE(op2.Execute());

    REQUIRE(close(op2.GetMax(), 10.0));
    REQUIRE(op2.GetMaxLocation() == 6);
    REQUIRE(close(op2.GetMin(), -3.0));
    REQUIRE(op2.GetMinLocation() == 8);

    return 0;
}

//CASE 8 (edge) - input unchanged after search
static int test_case8()
{
    std::size_t dim[1] = {6};
    arr1d_double input(dim);
    double vals[6] = {3, 1, 4, 1, 5, 9};
    for(std::size_t i = 0; i < 6; ++i)
        input(i) = vals[i];

    // Snapshot
    double snapshot[6];
    for(std::size_t i = 0; i < 6; ++i)
        snapshot[i] = input(i);

    MHO_ExtremaSearch< arr1d_double > op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    for(std::size_t i = 0; i < 6; ++i)
        REQUIRE(close(input(i), snapshot[i]));

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
    if(test_case8())
        return 1;

    return 0;
}
