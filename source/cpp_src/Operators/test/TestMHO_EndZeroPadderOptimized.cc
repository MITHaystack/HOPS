#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>

#include "MHO_EndZeroPadder.hh"
#include "MHO_EndZeroPadderOptimized.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

//Type aliases

using array1d_double = MHO_NDArrayWrapper< double, 1 >;
using array2d_double = MHO_NDArrayWrapper< double, 2 >;
using array1d_cplx = MHO_NDArrayWrapper< std::complex< double >, 1 >;
using array2d_cplx = MHO_NDArrayWrapper< std::complex< double >, 2 >;
using array3d_cplx = MHO_NDArrayWrapper< std::complex< double >, 3 >;

using table2d_double = MHO_TableContainer< double, MHO_AxisPack< MHO_Axis< double >, MHO_Axis< double > > >;

//Fill helpers

//double ramp into 1-D NDArrayWrapper:  a[i] = i+1
static void fill_ramp_double_1d(array1d_double& arr)
{
    for(std::size_t i = 0; i < arr.GetSize(); i++)
        arr[i] = double(i + 1);
}

//complex ramp into 1-D NDArrayWrapper: a[i] = (i+1) - (i+1)*j
static void fill_ramp_cplx_1d(array1d_cplx& arr)
{
    for(std::size_t i = 0; i < arr.GetSize(); i++)
    {
        double val = double(i + 1);
        arr[i] = std::complex< double >(val, -val);
    }
}

//2-D complex ramp into 2-D NDArrayWrapper: a(i,j) = (flat+1)-(flat+1)*j
static void fill_ramp_cplx_2d(array2d_cplx& arr)
{
    std::size_t dims[2];
    arr.GetDimensions(dims);
    for(std::size_t i = 0; i < dims[0]; i++)
    {
        for(std::size_t j = 0; j < dims[1]; j++)
        {
            double val = double(i * dims[1] + j + 1);
            arr(i, j) = std::complex< double >(val, -val);
        }
    }
}

//3-D complex ramp into 3-D NDArrayWrapper
static void fill_ramp_cplx_3d(array3d_cplx& arr)
{
    std::size_t dims[3];
    arr.GetDimensions(dims);
    for(std::size_t i = 0; i < dims[0]; i++)
    {
        for(std::size_t j = 0; j < dims[1]; j++)
        {
            for(std::size_t k = 0; k < dims[2]; k++)
            {
                double val = double(i * dims[1] * dims[2] + j * dims[2] + k + 1);
                arr(i, j, k) = std::complex< double >(val, -val);
            }
        }
    }
}

//double ramp into 2-D TableContainer
static void fill_ramp_double_2d(table2d_double& tbl)
{
    std::size_t dims[2];
    tbl.GetDimensions(dims);
    for(std::size_t i = 0; i < dims[0]; i++)
    {
        for(std::size_t j = 0; j < dims[1]; j++)
        {
            tbl(i, j) = double(i * dims[1] + j + 1);
        }
    }
}

//Dimension comparison helper

static int require_dims_equal(const array1d_double& a, const array1d_double& b)
{
    std::size_t da[1], db[1];
    a.GetDimensions(da);
    b.GetDimensions(db);
    REQUIRE(da[0] == db[0]);
    return 0;
}

static int require_dims_equal(const array2d_double& a, const array2d_double& b)
{
    std::size_t da[2], db[2];
    a.GetDimensions(da);
    b.GetDimensions(db);
    REQUIRE(da[0] == db[0]);
    REQUIRE(da[1] == db[1]);
    return 0;
}

static int require_dims_equal(const array1d_cplx& a, const array1d_cplx& b)
{
    std::size_t da[1], db[1];
    a.GetDimensions(da);
    b.GetDimensions(db);
    REQUIRE(da[0] == db[0]);
    return 0;
}

static int require_dims_equal(const array2d_cplx& a, const array2d_cplx& b)
{
    std::size_t da[2], db[2];
    a.GetDimensions(da);
    b.GetDimensions(db);
    REQUIRE(da[0] == db[0]);
    REQUIRE(da[1] == db[1]);
    return 0;
}

static int require_dims_equal(const array3d_cplx& a, const array3d_cplx& b)
{
    std::size_t da[3], db[3];
    a.GetDimensions(da);
    b.GetDimensions(db);
    REQUIRE(da[0] == db[0]);
    REQUIRE(da[1] == db[1]);
    REQUIRE(da[2] == db[2]);
    return 0;
}

static int require_dims_equal(const table2d_double& a, const table2d_double& b)
{
    std::size_t da[2], db[2];
    a.GetDimensions(da);
    b.GetDimensions(db);
    REQUIRE(da[0] == db[0]);
    REQUIRE(da[1] == db[1]);
    return 0;
}

//Test Case 1: End-pad, factor 2, 1-D double

static int test_case1_endpad_factor2_1d_double()
{
    array1d_double in;
    in.Resize(4);
    fill_ramp_double_1d(in);

    array1d_double out_ref, out_opt;

    {
        MHO_EndZeroPadder< array1d_double > ref;
        ref.SetPaddingFactor(2);
        ref.SetEndPadded();
        ref.SetArgs(&in, &out_ref);
        REQUIRE(ref.Initialize());
        REQUIRE(ref.Execute());
    }
    {
        MHO_EndZeroPadderOptimized< array1d_double > opt;
        opt.SetPaddingFactor(2);
        opt.SetEndPadded();
        opt.SetArgs(&in, &out_opt);
        REQUIRE(opt.Initialize());
        REQUIRE(opt.Execute());
    }

    //dims must match
    REQUIRE(out_ref.GetSize() == out_opt.GetSize());
    REQUIRE(out_ref.GetSize() == 8);

    //elementwise comparison
    for(std::size_t i = 0; i < 8; i++)
    {
        CHECK_CLOSE(out_opt[i], out_ref[i], 0.0);
    }

    //explicit expected values
    double expected[8] = {1, 2, 3, 4, 0, 0, 0, 0};
    for(std::size_t i = 0; i < 8; i++)
        CHECK_CLOSE(out_opt[i], expected[i], 0.0);

    return 0;
}

//Test Case 2: End-pad, factor 2, 2-D complex (3x5 -> 6x10)

static int test_case2_endpad_factor2_2d_complex()
{
    array2d_cplx in;
    in.Resize(3, 5);
    fill_ramp_cplx_2d(in);

    array2d_cplx out_ref, out_opt;

    {
        MHO_EndZeroPadder< array2d_cplx > ref;
        ref.SetPaddingFactor(2);
        ref.SetEndPadded();
        ref.SetArgs(&in, &out_ref);
        REQUIRE(ref.Initialize());
        REQUIRE(ref.Execute());
    }
    {
        MHO_EndZeroPadderOptimized< array2d_cplx > opt;
        opt.SetPaddingFactor(2);
        opt.SetEndPadded();
        opt.SetArgs(&in, &out_opt);
        REQUIRE(opt.Initialize());
        REQUIRE(opt.Execute());
    }

    {
        std::size_t dr[2], do_[2];
        out_ref.GetDimensions(dr);
        out_opt.GetDimensions(do_);
        REQUIRE(dr[0] == do_[0]);
        REQUIRE(dr[1] == do_[1]);
        REQUIRE(dr[0] == 6);
        REQUIRE(dr[1] == 10);
    }

    for(std::size_t i = 0; i < 6; i++)
    {
        for(std::size_t j = 0; j < 10; j++)
        {
            REQUIRE_CLOSE_CPLX(out_opt(i, j), out_ref(i, j), 0.0);
        }
    }

    return 0;
}

//Test Case 3: End-pad with single-axis selection, 2-D

static int test_case3_endpad_single_axis_2d()
{
    array2d_cplx in;
    in.Resize(3, 5);
    fill_ramp_cplx_2d(in);

    array2d_cplx out_ref, out_opt;

    {
        MHO_EndZeroPadder< array2d_cplx > ref;
        ref.DeselectAllAxes();
        ref.SelectAxis(1);
        ref.SetPaddingFactor(2);
        ref.SetEndPadded();
        ref.SetArgs(&in, &out_ref);
        REQUIRE(ref.Initialize());
        REQUIRE(ref.Execute());
    }
    {
        MHO_EndZeroPadderOptimized< array2d_cplx > opt;
        opt.DeselectAllAxes();
        opt.SelectAxis(1);
        opt.SetPaddingFactor(2);
        opt.SetEndPadded();
        opt.SetArgs(&in, &out_opt);
        REQUIRE(opt.Initialize());
        REQUIRE(opt.Execute());
    }

    {
        std::size_t dr[2], do_[2];
        out_ref.GetDimensions(dr);
        out_opt.GetDimensions(do_);
        REQUIRE(dr[0] == do_[0]);
        REQUIRE(dr[1] == do_[1]);
        REQUIRE(dr[0] == 3);
        REQUIRE(dr[1] == 10);
    }

    for(std::size_t i = 0; i < 3; i++)
    {
        for(std::size_t j = 0; j < 10; j++)
        {
            REQUIRE_CLOSE_CPLX(out_opt(i, j), out_ref(i, j), 0.0);
        }
    }

    return 0;
}

//Test Case 4: Reverse end-pad + NormFX enabled (default), 1-D

static int test_case4_reverse_normfx_on_1d()
{
    array1d_double in;
    in.Resize(4);
    fill_ramp_double_1d(in);

    array1d_double out_ref, out_opt;

    {
        MHO_EndZeroPadder< array1d_double > ref;
        ref.SetPaddingFactor(2);
        ref.SetReverseEndPadded();
        ref.SetArgs(&in, &out_ref);
        REQUIRE(ref.Initialize());
        REQUIRE(ref.Execute());
    }
    {
        MHO_EndZeroPadderOptimized< array1d_double > opt;
        opt.SetPaddingFactor(2);
        opt.SetReverseEndPadded();
        opt.SetArgs(&in, &out_opt);
        REQUIRE(opt.Initialize());
        REQUIRE(opt.Execute());
    }

    REQUIRE(out_ref.GetSize() == out_opt.GetSize());
    REQUIRE(out_ref.GetSize() == 8);

    for(std::size_t i = 0; i < 8; i++)
    {
        CHECK_CLOSE(out_opt[i], out_ref[i], 0.0);
    }

    return 0;
}

//Test Case 5: Reverse end-pad + NormFX DISABLED, 1-D

static int test_case5_reverse_normfx_off_1d()
{
    array1d_double in;
    in.Resize(4);
    fill_ramp_double_1d(in);

    array1d_double out_ref, out_opt;

    {
        MHO_EndZeroPadder< array1d_double > ref;
        ref.SetPaddingFactor(2);
        ref.SetReverseEndPadded();
        ref.DisableNormFXMode();
        ref.SetArgs(&in, &out_ref);
        REQUIRE(ref.Initialize());
        REQUIRE(ref.Execute());
    }
    {
        MHO_EndZeroPadderOptimized< array1d_double > opt;
        opt.SetPaddingFactor(2);
        opt.SetReverseEndPadded();
        opt.DisableNormFXMode();
        opt.SetArgs(&in, &out_opt);
        REQUIRE(opt.Initialize());
        REQUIRE(opt.Execute());
    }

    REQUIRE(out_ref.GetSize() == out_opt.GetSize());
    REQUIRE(out_ref.GetSize() == 8);

    //Optimized == reference
    for(std::size_t i = 0; i < 8; i++)
    {
        CHECK_CLOSE(out_opt[i], out_ref[i], 0.0);
    }

    //Explicit expected: [0,0,0,0, 4,3,2,1]
    double expected[8] = {0, 0, 0, 0, 4, 3, 2, 1};
    for(std::size_t i = 0; i < 8; i++)
        CHECK_CLOSE(out_opt[i], expected[i], 0.0);

    return 0;
}

//Test Case 6: SetPaddedSize absolute size, 1-D, end-pad

static int test_case6_paddedsize_1d()
{
    array1d_double in;
    in.Resize(4);
    fill_ramp_double_1d(in);

    array1d_double out_ref, out_opt;

    {
        MHO_EndZeroPadder< array1d_double > ref;
        ref.SetPaddedSize(7);
        ref.SetEndPadded();
        ref.SetArgs(&in, &out_ref);
        REQUIRE(ref.Initialize());
        REQUIRE(ref.Execute());
    }
    {
        MHO_EndZeroPadderOptimized< array1d_double > opt;
        opt.SetPaddedSize(7);
        opt.SetEndPadded();
        opt.SetArgs(&in, &out_opt);
        REQUIRE(opt.Initialize());
        REQUIRE(opt.Execute());
    }

    REQUIRE(out_ref.GetSize() == out_opt.GetSize());
    REQUIRE(out_ref.GetSize() == 7);

    for(std::size_t i = 0; i < 7; i++)
    {
        CHECK_CLOSE(out_opt[i], out_ref[i], 0.0);
    }

    //Explicit expected: [1,2,3,4, 0,0,0]
    double expected[7] = {1, 2, 3, 4, 0, 0, 0};
    for(std::size_t i = 0; i < 7; i++)
        CHECK_CLOSE(out_opt[i], expected[i], 0.0);

    return 0;
}

//Test Case 7: 3-D end-pad, factor 2, complex, all axes

static int test_case7_endpad_3d_complex()
{
    array3d_cplx in;
    in.Resize(2, 3, 4);
    fill_ramp_cplx_3d(in);

    array3d_cplx out_ref, out_opt;

    {
        MHO_EndZeroPadder< array3d_cplx > ref;
        ref.SetPaddingFactor(2);
        ref.SetEndPadded();
        ref.SetArgs(&in, &out_ref);
        REQUIRE(ref.Initialize());
        REQUIRE(ref.Execute());
    }
    {
        MHO_EndZeroPadderOptimized< array3d_cplx > opt;
        opt.SetPaddingFactor(2);
        opt.SetEndPadded();
        opt.SetArgs(&in, &out_opt);
        REQUIRE(opt.Initialize());
        REQUIRE(opt.Execute());
    }

    {
        std::size_t dr[3], do_[3];
        out_ref.GetDimensions(dr);
        out_opt.GetDimensions(do_);
        REQUIRE(dr[0] == do_[0]);
        REQUIRE(dr[1] == do_[1]);
        REQUIRE(dr[2] == do_[2]);
        REQUIRE(dr[0] == 4);
        REQUIRE(dr[1] == 6);
        REQUIRE(dr[2] == 8);
    }

    for(std::size_t i = 0; i < 4; i++)
    {
        for(std::size_t j = 0; j < 6; j++)
        {
            for(std::size_t k = 0; k < 8; k++)
            {
                REQUIRE_CLOSE_CPLX(out_opt(i, j, k), out_ref(i, j, k), 0.0);
            }
        }
    }

    return 0;
}

//Test Case 8: Table container axis transform (end-pad), rank 2

static int test_case8_table_container_axis()
{
    table2d_double in;
    in.Resize(3, 5);
    fill_ramp_double_2d(in);

    //Set axis labels: axis0(i) = i*1.0, axis1(j) = j*0.5
    {
        auto& axis0 = std::get< 0 >(in);
        axis0.Resize(3);
        for(std::size_t i = 0; i < 3; i++)
            axis0(i) = double(i) * 1.0;

        auto& axis1 = std::get< 1 >(in);
        axis1.Resize(5);
        for(std::size_t j = 0; j < 5; j++)
            axis1(j) = double(j) * 0.5;
    }

    table2d_double out_ref, out_opt;

    {
        MHO_EndZeroPadder< table2d_double > ref;
        ref.SetPaddingFactor(2);
        ref.SetEndPadded();
        ref.EnableTagCopy();
        ref.SetArgs(&in, &out_ref);
        REQUIRE(ref.Initialize());
        REQUIRE(ref.Execute());
    }
    {
        MHO_EndZeroPadderOptimized< table2d_double > opt;
        opt.SetPaddingFactor(2);
        opt.SetEndPadded();
        opt.EnableTagCopy();
        opt.SetArgs(&in, &out_opt);
        REQUIRE(opt.Initialize());
        REQUIRE(opt.Execute());
    }

    //dims
    {
        std::size_t dr[2], do_[2];
        out_ref.GetDimensions(dr);
        out_opt.GetDimensions(do_);
        REQUIRE(dr[0] == do_[0]);
        REQUIRE(dr[1] == do_[1]);
        REQUIRE(dr[0] == 6);
        REQUIRE(dr[1] == 10);
    }

    //data comparison
    for(std::size_t i = 0; i < 6; i++)
    {
        for(std::size_t j = 0; j < 10; j++)
        {
            CHECK_CLOSE(out_opt(i, j), out_ref(i, j), 0.0);
        }
    }

    //axis label comparison
    {
        auto& ref_axis0 = std::get< 0 >(out_ref);
        auto& opt_axis0 = std::get< 0 >(out_opt);
        for(std::size_t i = 0; i < ref_axis0.GetSize(); i++)
            CHECK_CLOSE(opt_axis0(i), ref_axis0(i), 1e-12);
    }
    {
        auto& ref_axis1 = std::get< 1 >(out_ref);
        auto& opt_axis1 = std::get< 1 >(out_opt);
        for(std::size_t j = 0; j < ref_axis1.GetSize(); j++)
            CHECK_CLOSE(opt_axis1(j), ref_axis1(j), 1e-12);
    }

    return 0;
}

//Test Case 9: In-place execution path

static int test_case9_inplace()
{
    array1d_double inA, inB;
    inA.Resize(4);
    fill_ramp_double_1d(inA);
    inB.Resize(4);
    fill_ramp_double_1d(inB);

    {
        MHO_EndZeroPadder< array1d_double > ref;
        ref.SetPaddingFactor(2);
        ref.SetEndPadded();
        ref.SetArgs(&inA);
        REQUIRE(ref.Initialize());
        REQUIRE(ref.Execute());
    }
    {
        MHO_EndZeroPadderOptimized< array1d_double > opt;
        opt.SetPaddingFactor(2);
        opt.SetEndPadded();
        opt.SetArgs(&inB);
        REQUIRE(opt.Initialize());
        REQUIRE(opt.Execute());
    }

    //dims match
    REQUIRE(inA.GetSize() == inB.GetSize());
    REQUIRE(inA.GetSize() == 8);

    //elementwise
    for(std::size_t i = 0; i < 8; i++)
    {
        CHECK_CLOSE(inA[i], inB[i], 0.0);
    }

    //ramp in [0..3], zeros in [4..7]
    double expected[8] = {1, 2, 3, 4, 0, 0, 0, 0};
    for(std::size_t i = 0; i < 8; i++)
        CHECK_CLOSE(inB[i], expected[i], 0.0);

    return 0;
}

//Test Case 10: factor 1 (degenerate / no-padding edge case)

static int test_case10_factor1_edge()
{
    array2d_cplx in;
    in.Resize(3, 5);
    fill_ramp_cplx_2d(in);

    array2d_cplx out_ref, out_opt;

    //Initialize both -- with factor=1 and fPaddedSize=0,
    //ConditionallyResizeOutput sets output dims to 0 (degenerate).
    //We check that both produce the same (possibly degenerate) dims.
    {
        MHO_EndZeroPadder< array2d_cplx > ref;
        ref.SetPaddingFactor(1);
        ref.SetEndPadded();
        ref.SetArgs(&in, &out_ref);
        REQUIRE(ref.Initialize());
    }
    {
        MHO_EndZeroPadderOptimized< array2d_cplx > opt;
        opt.SetPaddingFactor(1);
        opt.SetEndPadded();
        opt.SetArgs(&in, &out_opt);
        REQUIRE(opt.Initialize());
    }

    //Dims must agree -- even if both are degenerate
    {
        std::size_t dr[2], do_[2];
        out_ref.GetDimensions(dr);
        out_opt.GetDimensions(do_);
        REQUIRE(dr[0] == do_[0]);
        REQUIRE(dr[1] == do_[1]);
    }

    //Only run Execute if both outputs have non-zero total elements.
    // Degenerate config (0-sized axis) causes crash on Execute.
    if(out_ref.GetSize() > 0 && out_opt.GetSize() > 0)
    {
        {
            MHO_EndZeroPadder< array2d_cplx > ref;
            ref.SetPaddingFactor(1);
            ref.SetEndPadded();
            ref.SetArgs(&in, &out_ref);
            REQUIRE(ref.Initialize());
            REQUIRE(ref.Execute());
        }
        {
            MHO_EndZeroPadderOptimized< array2d_cplx > opt;
            opt.SetPaddingFactor(1);
            opt.SetEndPadded();
            opt.SetArgs(&in, &out_opt);
            REQUIRE(opt.Initialize());
            REQUIRE(opt.Execute());
        }

        std::size_t dims[2];
        out_opt.GetDimensions(dims);
        for(std::size_t i = 0; i < dims[0]; i++)
        {
            for(std::size_t j = 0; j < dims[1]; j++)
            {
                REQUIRE_CLOSE_CPLX(out_opt(i, j), out_ref(i, j), 0.0);
            }
        }
    }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if(test_case1_endpad_factor2_1d_double())
        return 1;
    if(test_case2_endpad_factor2_2d_complex())
        return 1;
    if(test_case3_endpad_single_axis_2d())
        return 1;
    if(test_case4_reverse_normfx_on_1d())
        return 1;
    if(test_case5_reverse_normfx_off_1d())
        return 1;
    if(test_case6_paddedsize_1d())
        return 1;
    if(test_case7_endpad_3d_complex())
        return 1;
    if(test_case8_table_container_axis())
        return 1;
    if(test_case9_inplace())
        return 1;
    if(test_case10_factor1_edge())
        return 1;

    return 0;
}
