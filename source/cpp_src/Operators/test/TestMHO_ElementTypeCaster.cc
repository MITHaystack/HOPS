#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <iostream>

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ElementTypeCaster.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"

#include "MHO_TestAssertions.hh"

using namespace hops;

static bool close(double a, double b, double atol = 1e-12)
{
    return std::fabs(a - b) <= atol;
}

//Test cases

//CASE 1 - float -> double widening (Fixture A)
static int test_case1()
{
    using InArr = MHO_NDArrayWrapper< float, 2 >;
    using OutArr = MHO_NDArrayWrapper< double, 2 >;

    std::size_t dim[2] = {3, 4};
    InArr input(dim);
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
            input(i, j) = static_cast< float >(static_cast< float >(i) + 0.5f * static_cast< float >(j));

    OutArr output(dim);

    MHO_ElementTypeCaster< InArr, OutArr > caster;
    caster.SetArgs(&input, &output);
    REQUIRE(caster.Initialize());
    REQUIRE(caster.Execute());

    // Check dims
    REQUIRE(output.GetDimensions()[0] == 3);
    REQUIRE(output.GetDimensions()[1] == 4);

    // Check every element within 1e-6
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
        {
            double expected = static_cast< double >(input(i, j));
            REQUIRE(close(output(i, j), expected, 1e-6));
        }

    return 0;
}

//CASE 2 - double -> int truncation (Fixture B)
static int test_case2()
{
    using InArr = MHO_NDArrayWrapper< double, 1 >;
    using OutArr = MHO_NDArrayWrapper< int, 1 >;

    std::size_t dim[1] = {5};
    InArr input(dim);
    input(0) = 0.0;
    input(1) = 1.9;
    input(2) = -1.9;
    input(3) = 2.5;
    input(4) = -2.5;

    OutArr output(dim);

    MHO_ElementTypeCaster< InArr, OutArr > caster;
    caster.SetArgs(&input, &output);
    REQUIRE(caster.Initialize());
    REQUIRE(caster.Execute());

    // static_cast<int> truncates toward zero
    int expected[5] = {0, 1, -1, 2, -2};
    for(std::size_t k = 0; k < 5; ++k)
        REQUIRE(output(k) == expected[k]);

    return 0;
}

//CASE 3 - complex<float> -> complex<double> (Fixture C)
static int test_case3()
{
    using InArr = MHO_NDArrayWrapper< std::complex< float >, 1 >;
    using OutArr = MHO_NDArrayWrapper< std::complex< double >, 1 >;

    std::size_t dim[1] = {3};
    InArr input(dim);
    input(0) = std::complex< float >(1.5f, -2.5f);
    input(1) = std::complex< float >(0.0f, 0.0f);
    input(2) = std::complex< float >(-3.25f, 4.75f);

    OutArr output(dim);

    MHO_ElementTypeCaster< InArr, OutArr > caster;
    caster.SetArgs(&input, &output);
    REQUIRE(caster.Initialize());
    REQUIRE(caster.Execute());

    REQUIRE(output.GetDimensions()[0] == 3);

    for(std::size_t k = 0; k < 3; ++k)
    {
        std::complex< double > expected(input(k));
        REQUIRE(close(output(k).real(), expected.real(), 1e-6));
        REQUIRE(close(output(k).imag(), expected.imag(), 1e-6));
    }

    return 0;
}

//CASE 4 - Output auto-resize from empty
static int test_case4()
{
    using InArr = MHO_NDArrayWrapper< float, 2 >;
    using OutArr = MHO_NDArrayWrapper< double, 2 >;

    std::size_t dim[2] = {3, 4};
    InArr input(dim);
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
            input(i, j) = static_cast< float >(i + j);

    // Default-constructed output (empty)
    OutArr output;

    MHO_ElementTypeCaster< InArr, OutArr > caster;
    caster.SetArgs(&input, &output);
    REQUIRE(caster.Initialize());
    REQUIRE(caster.Execute());

    // Output should be resized to match input
    REQUIRE(output.GetDimensions()[0] == 3);
    REQUIRE(output.GetDimensions()[1] == 4);

    return 0;
}

//CASE 5 - Table cast preserves axes and tags (Fixture D)
static int test_case5()
{
    using AxisPack_t = MHO_AxisPack< MHO_Axis< int >, MHO_Axis< double > >;

    using InTable = MHO_TableContainer< double, AxisPack_t >;
    using OutTable = MHO_TableContainer< int, AxisPack_t >;

    std::size_t dim[2] = {2, 3};
    InTable input(dim);

    // Fill data: a(i,j) = i*10 + j + 0.3
    for(std::size_t i = 0; i < 2; ++i)
        for(std::size_t j = 0; j < 3; ++j)
            input(i, j) = static_cast< double >(i) * 10.0 + static_cast< double >(j) + 0.3;

    // Set axis values
    auto& axis0 = std::get< 0 >(*input.GetAxisPack());
    auto& axis1 = std::get< 1 >(*input.GetAxisPack());
    for(std::size_t i = 0; i < 2; ++i)
        axis0(i) = static_cast< int >(i);
    for(std::size_t j = 0; j < 3; ++j)
        axis1(j) = static_cast< double >(j) * 0.25;

    OutTable output;

    MHO_ElementTypeCaster< InTable, OutTable > caster;
    caster.SetArgs(&input, &output);
    REQUIRE(caster.Initialize());
    REQUIRE(caster.Execute());

    // Check dims
    REQUIRE(output.GetDimensions()[0] == 2);
    REQUIRE(output.GetDimensions()[1] == 3);

    // Check data: truncated to int (i*10 + j + 0.3 -> i*10 + j)
    for(std::size_t i = 0; i < 2; ++i)
        for(std::size_t j = 0; j < 3; ++j)
        {
            int expected = static_cast< int >(i) * 10 + static_cast< int >(j);
            REQUIRE(output(i, j) == expected);
        }

    // Check axis0 labels (int)
    const auto& outAxis0 = std::get< 0 >(*output.GetAxisPack());
    for(std::size_t i = 0; i < 2; ++i)
        REQUIRE(outAxis0(i) == axis0(i));

    // Check axis1 labels (double within 1e-12)
    const auto& outAxis1 = std::get< 1 >(*output.GetAxisPack());
    for(std::size_t j = 0; j < 3; ++j)
        REQUIRE(close(outAxis1(j), axis1(j), 1e-12));

    return 0;
}

//CASE 6 - Round-trip widening then narrowing
static int test_case6()
{
    using IntArr = MHO_NDArrayWrapper< int, 1 >;
    using DblArr = MHO_NDArrayWrapper< double, 1 >;

    std::size_t dim[1] = {4};
    IntArr original(dim);
    original(0) = 3;
    original(1) = -7;
    original(2) = 0;
    original(3) = 42;

    // int -> double
    DblArr dbl(dim);
    {
        MHO_ElementTypeCaster< IntArr, DblArr > caster1;
        caster1.SetArgs(&original, &dbl);
        REQUIRE(caster1.Initialize());
        REQUIRE(caster1.Execute());
    }

    // double -> int (round-trip)
    IntArr roundtrip(dim);
    {
        MHO_ElementTypeCaster< DblArr, IntArr > caster2;
        caster2.SetArgs(&dbl, &roundtrip);
        REQUIRE(caster2.Initialize());
        REQUIRE(caster2.Execute());
    }

    // Verify round-trip equals original
    for(std::size_t k = 0; k < 4; ++k)
        REQUIRE(roundtrip(k) == original(k));

    return 0;
}

//CASE 7 - Null output pointer (error path)
static int test_case7()
{
    using InArr = MHO_NDArrayWrapper< float, 1 >;
    using OutArr = MHO_NDArrayWrapper< double, 1 >;

    std::size_t dim[1] = {3};
    InArr input(dim);
    input(0) = 1.0f;
    input(1) = 2.0f;
    input(2) = 3.0f;

    MHO_ElementTypeCaster< InArr, OutArr > caster;

    // Pass nullptr as output -- ExecuteImpl should return false
    caster.SetArgs(&input, static_cast< OutArr* >(nullptr));
    REQUIRE(caster.Initialize());

    // Execute should return false (no crash)
    bool result = caster.Execute();
    REQUIRE(result == false);

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
