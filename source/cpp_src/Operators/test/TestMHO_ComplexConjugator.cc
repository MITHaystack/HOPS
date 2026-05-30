#include <cmath>
#include <complex>
#include <iostream>

#include "MHO_ComplexConjugator.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static bool close(double a, double b, double atol = 1e-12)
{
    return std::fabs(a - b) <= atol;
}

static bool closef(float a, float b, float atol = 1e-6f)
{
    return std::fabs(a - b) <= atol;
}

//Fixture helpers

using arr2d_complex = MHO_NDArrayWrapper< std::complex< double >, 2 >;
using arr2d_cfloat = MHO_NDArrayWrapper< std::complex< float >, 2 >;
using arr1d_complex = MHO_NDArrayWrapper< std::complex< double >, 1 >;

static void fill_fixture(arr2d_complex& arr)
{
    // 3x4 array: a(i,j) = complex( double(i+1), double(j+1) - 2.0 )
    //   j=0: imag = -1   j=1: imag = 0   j=2: imag = +1   j=3: imag = +2
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
            arr(i, j) = std::complex< double >(double(i + 1), double(j + 1) - 2.0);
}

static void fill_fixture_float(arr2d_cfloat& arr)
{
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
            arr(i, j) = std::complex< float >(float(i + 1), float(j + 1) - 2.0f);
}

//Test cases

//CASE 1 - Out-of-place conjugation of a 3x4 complex array
static int test_case1()
{
    std::size_t dim[2] = {3, 4};
    arr2d_complex input(dim);
    arr2d_complex output(dim);
    fill_fixture(input);

    // Save original for unchanged check
    arr2d_complex original = input;

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_complex, MHO_ComplexConjugator< arr2d_complex > >;
    Broadcaster op;
    op.SetArgs(&input, &output);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Check every element: conj(complex(r, im)) = complex(r, -im)
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
        {
            std::complex< double > expected = std::complex< double >(double(i + 1), -(double(j + 1) - 2.0));
            REQUIRE(close(output(i, j).real(), expected.real()));
            REQUIRE(close(output(i, j).imag(), expected.imag()));
        }

    // Input must be unchanged
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
        {
            REQUIRE(close(input(i, j).real(), original(i, j).real()));
            REQUIRE(close(input(i, j).imag(), original(i, j).imag()));
        }

    return 0;
}

//CASE 2 - In-place conjugation
static int test_case2()
{
    std::size_t dim[2] = {3, 4};
    arr2d_complex input(dim);
    fill_fixture(input);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_complex, MHO_ComplexConjugator< arr2d_complex > >;
    Broadcaster op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
        {
            std::complex< double > expected = std::complex< double >(double(i + 1), -(double(j + 1) - 2.0));
            REQUIRE(close(input(i, j).real(), expected.real()));
            REQUIRE(close(input(i, j).imag(), expected.imag()));
        }

    return 0;
}

//CASE 3 - Involution (double conjugate == identity)
static int test_case3()
{
    std::size_t dim[2] = {3, 4};
    arr2d_complex input(dim);
    fill_fixture(input);

    arr2d_complex original = input;

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_complex, MHO_ComplexConjugator< arr2d_complex > >;
    Broadcaster op;
    op.SetArgs(&input);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Second conjugation
    op.Initialize();
    REQUIRE(op.Execute());

    // Should match original
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
        {
            REQUIRE(close(input(i, j).real(), original(i, j).real()));
            REQUIRE(close(input(i, j).imag(), original(i, j).imag()));
        }

    return 0;
}

//CASE 4 - Real-only element (zero imaginary part)
static int test_case4()
{
    std::size_t dim[1] = {1};
    arr1d_complex arr(dim);
    arr(0) = std::complex< double >(5.0, 0.0);

    MHO_ComplexConjugator< arr1d_complex > conj;
    auto it = arr.begin();
    conj(it);

    REQUIRE(close(arr(0).real(), 5.0));
    // Imag may be -0.0; compare magnitude
    REQUIRE(std::fabs(arr(0).imag()) <= 1e-12);

    return 0;
}

//CASE 5 - Direct functor operator() (no broadcaster)
static int test_case5()
{
    // In-place form: complex(2, -7) -> complex(2, 7)
    std::size_t dim[1] = {1};
    arr1d_complex arr(dim);
    arr(0) = std::complex< double >(2.0, -7.0);

    MHO_ComplexConjugator< arr1d_complex > conj;
    auto it = arr.begin();
    conj(it);
    REQUIRE(close(arr(0).real(), 2.0));
    REQUIRE(close(arr(0).imag(), 7.0));

    // Out-of-place form: operator()(citerator, iterator)
    std::size_t dim2[1] = {1};
    arr1d_complex input(dim2);
    arr1d_complex output(dim2);
    input(0) = std::complex< double >(2.0, -7.0);

    auto cit = input.cbegin();
    auto oit = output.begin();
    conj(cit, oit);
    REQUIRE(close(output(0).real(), 2.0));
    REQUIRE(close(output(0).imag(), 7.0));

    // Input preserved for cit/it form
    REQUIRE(close(input(0).real(), 2.0));
    REQUIRE(close(input(0).imag(), -7.0));

    return 0;
}

//CASE 6 - std::complex<float> instantiation
static int test_case6()
{
    std::size_t dim[2] = {3, 4};
    arr2d_cfloat input(dim);
    arr2d_cfloat output(dim);
    fill_fixture_float(input);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_cfloat, MHO_ComplexConjugator< arr2d_cfloat > >;
    Broadcaster op;
    op.SetArgs(&input, &output);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
        {
            std::complex< float > expected = std::complex< float >(float(i + 1), -(float(j + 1) - 2.0f));
            REQUIRE(closef(float(output(i, j).real()), float(expected.real()), 1e-6f));
            REQUIRE(closef(float(output(i, j).imag()), float(expected.imag()), 1e-6f));
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

    return 0;
}
