#include <cmath>
#include <complex>
#include <iostream>

#include "MHO_FunctorBroadcaster.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_UnaryFunctor.hh"

using namespace hops;

static bool close(double a, double b, double atol = 1e-12)
{
    return std::fabs(a - b) <= atol;
}

//Functor definitions

using arr2d_double = MHO_NDArrayWrapper< double, 2 >;

struct ScaleFunctor: public MHO_UnaryFunctor< arr2d_double >
{
        double k;

        ScaleFunctor(): k(1.0) {}

        void operator()(iterator_type& it) override { *it = (*it) * k; }

        void operator()(citerator_type& in, iterator_type& out) override { *out = (*in) * k; }
};

using arr2d_complex = MHO_NDArrayWrapper< std::complex< double >, 2 >;

struct RotateFunctor: public MHO_UnaryFunctor< arr2d_complex >
{
        RotateFunctor() {}

        void operator()(iterator_type& it) override { *it = (*it) * std::complex< double >(0, 1); }

        void operator()(citerator_type& in, iterator_type& out) override { *out = (*in) * std::complex< double >(0, 1); }
};

struct ConjFunctor: public MHO_UnaryFunctor< arr2d_complex >
{
        ConjFunctor() {}

        void operator()(iterator_type& it) override { *it = std::conj(*it); }

        void operator()(citerator_type& in, iterator_type& out) override { *out = std::conj(*in); }
};

//Helpers

static void fill_double(arr2d_double& arr)
{
    std::size_t d[2];
    arr.GetDimensions(d);
    int idx = 0;
    for(std::size_t i = 0; i < d[0]; ++i)
        for(std::size_t j = 0; j < d[1]; ++j)
            arr(i, j) = static_cast< double >(idx++);
}

static void fill_complex(arr2d_complex& arr, const std::complex< double >* values, std::size_t n)
{
    std::size_t d[2];
    arr.GetDimensions(d);
    std::size_t idx = 0;
    for(std::size_t i = 0; i < d[0] && idx < n; ++i)
        for(std::size_t j = 0; j < d[1] && idx < n; ++j)
            arr(i, j) = values[idx++];
}

//Test cases

//CASE 1 - In-place scale, double
static int test_case1()
{
    std::size_t dim[2] = {2, 3};
    arr2d_double in(dim);
    fill_double(in);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_double, ScaleFunctor >;
    Broadcaster op;
    op.GetFunctor()->k = 2.0;
    op.SetArgs(&in);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    for(std::size_t i = 0; i < dim[0]; ++i)
        for(std::size_t j = 0; j < dim[1]; ++j)
            REQUIRE(close(in(i, j), 2.0 * static_cast< double >(i * 3 + j)));

    return 0;
}

//CASE 2 - Out-of-place scale, double, same-shape
static int test_case2()
{
    std::size_t dim[2] = {2, 3};
    arr2d_double in(dim);
    arr2d_double out(dim);
    fill_double(in);
    fill_double(out);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_double, ScaleFunctor >;
    Broadcaster op;
    op.GetFunctor()->k = 3.0;
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Verify output
    for(std::size_t i = 0; i < dim[0]; ++i)
        for(std::size_t j = 0; j < dim[1]; ++j)
            REQUIRE(close(out(i, j), 3.0 * static_cast< double >(i * 3 + j)));

    // Verify input unchanged
    for(std::size_t i = 0; i < dim[0]; ++i)
        for(std::size_t j = 0; j < dim[1]; ++j)
            REQUIRE(close(in(i, j), static_cast< double >(i * 3 + j)));

    return 0;
}

//CASE 3 - Out-of-place auto-resize of mismatched output
static int test_case3()
{
    std::size_t in_dim[2] = {2, 3};
    std::size_t out_dim[2] = {5, 1};
    arr2d_double in(in_dim);
    arr2d_double out(out_dim);
    fill_double(in);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_double, ScaleFunctor >;
    Broadcaster op;
    op.GetFunctor()->k = 1.0;
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Verify out was resized
    std::size_t d[2];
    out.GetDimensions(d);
    REQUIRE(d[0] == 2 && d[1] == 3);

    // Verify contents
    for(std::size_t i = 0; i < in_dim[0]; ++i)
        for(std::size_t j = 0; j < in_dim[1]; ++j)
            REQUIRE(close(out(i, j), in(i, j)));

    return 0;
}

//CASE 4 - Execute-before-Initialize guard
static int test_case4()
{
    std::size_t dim[2] = {2, 3};
    arr2d_double in(dim);
    fill_double(in);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_double, ScaleFunctor >;
    Broadcaster op;
    op.GetFunctor()->k = 5.0;
    op.SetArgs(&in);
    // Do NOT call Initialize()

    bool ok = op.Execute();
    REQUIRE(ok == false);

    // Verify array unchanged
    for(std::size_t i = 0; i < dim[0]; ++i)
        for(std::size_t j = 0; j < dim[1]; ++j)
            REQUIRE(close(in(i, j), static_cast< double >(i * 3 + j)));

    return 0;
}

//CASE 5 - Re-Initialize + re-Execute (not idempotent)
static int test_case5()
{
    std::size_t dim[2] = {2, 3};
    arr2d_double in(dim);
    fill_double(in);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_double, ScaleFunctor >;
    Broadcaster op;
    op.GetFunctor()->k = 2.0;
    op.SetArgs(&in);
    op.Initialize();
    op.Execute();

    // Second pass - doubles again (4x original)
    op.Initialize();
    op.Execute();

    for(std::size_t i = 0; i < dim[0]; ++i)
        for(std::size_t j = 0; j < dim[1]; ++j)
            REQUIRE(close(in(i, j), 4.0 * static_cast< double >(i * 3 + j)));

    return 0;
}

//CASE 6 - Complex out-of-place 90-degree rotation
static int test_case6()
{
    std::size_t dim[2] = {2, 2};
    std::complex< double > vals[4] = {std::complex< double >(1, 0), std::complex< double >(0, 1), std::complex< double >(2, 0),
                                      std::complex< double >(0, -1)};
    std::complex< double > expected[4] = {
        std::complex< double >(0, 1),  // 1*i = i
        std::complex< double >(-1, 0), // i*i = -1
        std::complex< double >(0, 2),  // 2*i = 2i
        std::complex< double >(1, 0)   // (-i)*i = 1
    };

    arr2d_complex in(dim);
    arr2d_complex out(dim);
    fill_complex(in, vals, 4);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_complex, RotateFunctor >;
    Broadcaster op;
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    std::size_t idx = 0;
    for(std::size_t i = 0; i < dim[0]; ++i)
        for(std::size_t j = 0; j < dim[1]; ++j)
        {
            REQUIRE(close(out(i, j).real(), expected[idx].real()));
            REQUIRE(close(out(i, j).imag(), expected[idx].imag()));
            ++idx;
        }

    return 0;
}

//CASE 7 - Complex in-place conjugation
static int test_case7()
{
    std::size_t dim[2] = {2, 2};
    std::complex< double > vals[4] = {std::complex< double >(1, 2), std::complex< double >(3, -4), std::complex< double >(0, 1),
                                      std::complex< double >(-5, 0)};
    std::complex< double > expected[4] = {std::complex< double >(1, -2), std::complex< double >(3, 4),
                                          std::complex< double >(0, -1), std::complex< double >(-5, 0)};

    arr2d_complex in(dim);
    fill_complex(in, vals, 4);

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_complex, ConjFunctor >;
    Broadcaster op;
    op.SetArgs(&in);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    std::size_t idx = 0;
    for(std::size_t i = 0; i < dim[0]; ++i)
        for(std::size_t j = 0; j < dim[1]; ++j)
        {
            REQUIRE(close(in(i, j).real(), expected[idx].real()));
            REQUIRE(close(in(i, j).imag(), expected[idx].imag()));
            ++idx;
        }

    return 0;
}

//CASE 8 - Empty / zero-size array
static int test_case8()
{
    arr2d_double in; // default-constructed, empty
    // Construct with nullptr to get a valid but empty wrapper
    // The wrapper supports this via default constructor

    using Broadcaster = MHO_FunctorBroadcaster< arr2d_double, ScaleFunctor >;
    Broadcaster op;
    op.GetFunctor()->k = 2.0;
    op.SetArgs(&in);
    REQUIRE(op.Initialize() == true);
    REQUIRE(op.Execute() == true);
    // No crash, no out-of-bounds - that's the test

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
