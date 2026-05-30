#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>

#include "MHO_Axis.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_SubSample.hh"
#include "MHO_TableContainer.hh"

using namespace hops;

#include "MHO_TestAssertions.hh"

// Type aliases

using array2d_complex = MHO_NDArrayWrapper< std::complex< double >, 2 >;
using array2d_double = MHO_NDArrayWrapper< double, 2 >;
using array3d_double = MHO_NDArrayWrapper< double, 3 >;

using table_axis_pack = MHO_AxisPack< MHO_Axis< int >, MHO_Axis< double > >;
using table2d_type = MHO_TableContainer< double, table_axis_pack >;

// Helpers

static const double LABEL_EPS = 1e-12;

static bool close_double(double a, double b)
{
    return std::fabs(a - b) < LABEL_EPS;
}

// Fixtures

// 8x8 complex: a(i,j) = complex(i, j)
static array2d_complex make_complex_8x8()
{
    std::size_t dims[2] = {8, 8};
    array2d_complex arr(dims);
    for(std::size_t i = 0; i < 8; ++i)
        for(std::size_t j = 0; j < 8; ++j)
            arr(i, j) = std::complex< double >(static_cast< double >(i), static_cast< double >(j));
    return arr;
}

// 8x8 double: a(i,j) = i*1000 + j
static array2d_double make_double_8x8()
{
    std::size_t dims[2] = {8, 8};
    array2d_double arr(dims);
    for(std::size_t i = 0; i < 8; ++i)
        for(std::size_t j = 0; j < 8; ++j)
            arr(i, j) = static_cast< double >(i * 1000 + j);
    return arr;
}

// 2x6x3 double: a(i,j,k) = i*100 + j*10 + k
static array3d_double make_double_2x6x3()
{
    std::size_t dims[3] = {2, 6, 3};
    array3d_double arr(dims);
    for(std::size_t i = 0; i < 2; ++i)
        for(std::size_t j = 0; j < 6; ++j)
            for(std::size_t k = 0; k < 3; ++k)
                arr(i, j, k) = static_cast< double >(i * 100 + j * 10 + k);
    return arr;
}

// 4x8 table: axis1(j) = j (0..7), axis0 default ints
static table2d_type make_table_4x8()
{
    std::size_t dims[2] = {4, 8};
    table2d_type tbl;
    tbl.Resize(dims);
    auto& axis1 = std::get< 1 >(tbl);
    for(std::size_t j = 0; j < 8; ++j)
        axis1[j] = static_cast< double >(j);
    return tbl;
}

// main()

int main()
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // ---- Case 1: axis 1, stride 2, 2-D complex 8x8 ----
    {
        array2d_complex input = make_complex_8x8();
        array2d_complex output;

        MHO_SubSample< array2d_complex > op;
        op.SetDimensionAndStride(1, 2);
        op.SetArgs(&input, &output);
        REQUIRE(op.Initialize());
        REQUIRE(op.Execute());

        auto odims = output.GetDimensionArray();
        REQUIRE(odims[0] == 8);
        REQUIRE(odims[1] == 4);
        for(std::size_t i = 0; i < 8; ++i)
            for(std::size_t j = 0; j < 4; ++j)
            {
                std::complex< double > expected(static_cast< double >(i), static_cast< double >(2 * j));
                REQUIRE(output(i, j) == expected);
            }
    }

    // ---- Case 2: axis 0, stride 2, 2-D double 8x8 ----
    {
        array2d_double input = make_double_8x8();
        array2d_double output;

        MHO_SubSample< array2d_double > op;
        op.SetDimensionAndStride(0, 2);
        op.SetArgs(&input, &output);
        REQUIRE(op.Initialize());
        REQUIRE(op.Execute());

        auto odims = output.GetDimensionArray();
        REQUIRE(odims[0] == 4);
        REQUIRE(odims[1] == 8);
        for(std::size_t i = 0; i < 4; ++i)
            for(std::size_t j = 0; j < 8; ++j)
            {
                double expected = static_cast< double >((2 * i) * 1000 + j);
                REQUIRE(output(i, j) == expected);
            }
    }

    // ---- Case 3: stride 1 (identity) ----
    {
        array2d_double input = make_double_8x8();
        array2d_double output;

        MHO_SubSample< array2d_double > op;
        op.SetDimensionAndStride(1, 1);
        op.SetArgs(&input, &output);
        REQUIRE(op.Initialize());
        REQUIRE(op.Execute());

        auto odims = output.GetDimensionArray();
        REQUIRE(odims[0] == 8);
        REQUIRE(odims[1] == 8);
        for(std::size_t i = 0; i < 8; ++i)
            for(std::size_t j = 0; j < 8; ++j)
                REQUIRE(output(i, j) == input(i, j));
    }

    // ---- Case 4: stride == dimension (output size 1 on that axis) ----
    {
        array2d_double input = make_double_8x8();
        array2d_double output;

        MHO_SubSample< array2d_double > op;
        op.SetDimensionAndStride(1, 8);
        op.SetArgs(&input, &output);
        REQUIRE(op.Initialize());
        REQUIRE(op.Execute());

        auto odims = output.GetDimensionArray();
        REQUIRE(odims[0] == 8);
        REQUIRE(odims[1] == 1);
        for(std::size_t i = 0; i < 8; ++i)
            REQUIRE(output(i, 0) == input(i, 0));
    }

    // ---- Case 5: non-divisible stride must fail ----
    {
        array2d_double input = make_double_8x8();
        array2d_double output;

        MHO_SubSample< array2d_double > op;
        op.SetDimensionAndStride(1, 3);
        op.SetArgs(&input, &output);
        REQUIRE(op.Initialize() == false);
    }

    // ---- Case 6: table axis-label sub-sampling ----
    {
        table2d_type input = make_table_4x8();
        table2d_type output;

        MHO_SubSample< table2d_type > op;
        op.SetDimensionAndStride(1, 2);
        op.SetArgs(&input, &output);
        REQUIRE(op.Initialize());
        REQUIRE(op.Execute());

        auto odims = output.GetDimensionArray();
        REQUIRE(odims[0] == 4);
        REQUIRE(odims[1] == 4);

        auto& out_axis1 = std::get< 1 >(output);
        REQUIRE(out_axis1.GetSize() == 4);
        REQUIRE(close_double(out_axis1[0], 0.0));
        REQUIRE(close_double(out_axis1[1], 2.0));
        REQUIRE(close_double(out_axis1[2], 4.0));
        REQUIRE(close_double(out_axis1[3], 6.0));
    }

    // ---- Case 7: in-place execution ----
    {
        table2d_type table = make_table_4x8();

        MHO_SubSample< table2d_type > op;
        op.SetDimensionAndStride(1, 2);
        op.SetArgs(&table);
        REQUIRE(op.Initialize());
        REQUIRE(op.Execute());

        auto dims = table.GetDimensionArray();
        REQUIRE(dims[0] == 4);
        REQUIRE(dims[1] == 4);

        auto& axis1 = std::get< 1 >(table);
        REQUIRE(axis1.GetSize() == 4);
        REQUIRE(close_double(axis1[0], 0.0));
        REQUIRE(close_double(axis1[1], 2.0));
        REQUIRE(close_double(axis1[2], 4.0));
        REQUIRE(close_double(axis1[3], 6.0));

        // Data at every-other column preserved
        // Original table(i,j) = 0 (default double). After sub-sample col j
        // holds original col 2*j.
        for(std::size_t i = 0; i < 4; ++i)
            for(std::size_t j = 0; j < 4; ++j)
            {
                // default-initialized data is 0, so all preserved values are 0
                REQUIRE(table(i, j) == 0.0);
            }
    }

    // ---- Case 8: re-use object with new dimension/stride ----
    {
        // First run: axis 1 stride 2 on 8x8 complex
        array2d_complex A = make_complex_8x8();
        array2d_complex outA;
        MHO_SubSample< array2d_complex > op;
        op.SetDimensionAndStride(1, 2);
        op.SetArgs(&A, &outA);
        REQUIRE(op.Initialize());
        REQUIRE(op.Execute());
        {
            auto d = outA.GetDimensionArray();
            REQUIRE(d[0] == 8 && d[1] == 4);
        }

        // Second run: reconfigure to axis 0 stride 4 on fresh 8x8 double
        array2d_double B = make_double_8x8();
        array2d_double outB;
        MHO_SubSample< array2d_double > op2;
        op2.SetDimensionAndStride(1, 2);
        op2.SetArgs(&B, &outB);
        REQUIRE(op2.Initialize());
        REQUIRE(op2.Execute());

        // Now reconfigure op2 with different stride/dimension
        op2.SetDimensionAndStride(0, 4);
        array2d_double B2 = make_double_8x8();
        array2d_double outB2;
        op2.SetArgs(&B2, &outB2);
        REQUIRE(op2.Initialize());
        REQUIRE(op2.Execute());

        auto odims = outB2.GetDimensionArray();
        REQUIRE(odims[0] == 2);
        REQUIRE(odims[1] == 8);
        for(std::size_t i = 0; i < 2; ++i)
            for(std::size_t j = 0; j < 8; ++j)
            {
                double expected = static_cast< double >((4 * i) * 1000 + j);
                REQUIRE(outB2(i, j) == expected);
            }
    }

    // ---- Case 9 (edge): 3-D array, sub-sample middle axis ----
    {
        array3d_double input = make_double_2x6x3();
        array3d_double output;

        MHO_SubSample< array3d_double > op;
        op.SetDimensionAndStride(1, 3);
        op.SetArgs(&input, &output);
        REQUIRE(op.Initialize());
        REQUIRE(op.Execute());

        auto odims = output.GetDimensionArray();
        REQUIRE(odims[0] == 2);
        REQUIRE(odims[1] == 2);
        REQUIRE(odims[2] == 3);
        for(std::size_t i = 0; i < 2; ++i)
            for(std::size_t j = 0; j < 2; ++j)
                for(std::size_t k = 0; k < 3; ++k)
                {
                    double expected = static_cast< double >(i * 100 + (3 * j) * 10 + k);
                    REQUIRE(output(i, j, k) == expected);
                }
    }

    return 0;
}
