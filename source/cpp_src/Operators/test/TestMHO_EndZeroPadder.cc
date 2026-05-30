#include <complex>
#include <iostream>

#include "MHO_EndZeroPadder.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* Type aliases                                                       */

using array1_type = MHO_NDArrayWrapper< std::complex< double >, 1 >;
using array2_type = MHO_NDArrayWrapper< std::complex< double >, 2 >;
using table1_type = MHO_TableContainer< double, MHO_AxisPack< MHO_Axis< double > > >;

/* Fixtures                                                           */

/* (a) 1-D complex<double> NDArrayWrapper size 4,
 *      values {(1,0),(2,0),(3,0),(4,0)}                         */
static array1_type make_fixture_1d()
{
    array1_type arr;
    arr.Resize(4);
    for(std::size_t i = 0; i < 4; i++)
        arr[i] = std::complex< double >((double)(i + 1), 0.0);
    return arr;
}

/* (b) 2-D complex<double> NDArrayWrapper size 3x4,
 *      v[i][j] = (i+1) + (j+1)*1j                              */
static array2_type make_fixture_2d()
{
    array2_type arr;
    arr.Resize(3, 4);
    for(std::size_t i = 0; i < 3; i++)
    {
        for(std::size_t j = 0; j < 4; j++)
        {
            arr(i, j) = std::complex< double >((double)(i + 1), (double)(j + 1));
        }
    }
    return arr;
}

/* (c) 1-D MHO_TableContainer<double, MHO_AxisPack<MHO_Axis<double>>>
 *      values = {1,2,3,4}, axis labels = {0.0,0.5,1.0,1.5}    */
static table1_type make_fixture_table1d()
{
    std::size_t dim = 4;
    table1_type tbl;
    tbl.Resize(&dim);
    for(std::size_t i = 0; i < 4; i++)
        tbl[i] = (double)(i + 1);
    auto& axis = std::get< 0 >(tbl);
    axis.Resize(4);
    for(std::size_t i = 0; i < 4; i++)
        axis[i] = (double)i * 0.5;
    return tbl;
}

/* Test cases                                                         */

/* Case 1: EndPadded factor=2 (basic)                                */
static int test_endpadded_factor2()
{
    array1_type in = make_fixture_1d();
    array1_type out;

    MHO_EndZeroPadder< array1_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetEndPadded();
    padder.SetArgs(&in, &out);
    REQUIRE(padder.Initialize());
    REQUIRE(padder.Execute());

    REQUIRE(out.GetSize() == 8);
    for(std::size_t i = 0; i < 4; i++)
    {
        REQUIRE_CLOSE_CPLX(out[i], in[i], 1e-14);
    }
    for(std::size_t i = 4; i < 8; i++)
    {
        REQUIRE_CLOSE_CPLX(out[i], std::complex< double >(0, 0), 1e-14);
    }

    return 0;
}

/* Case 2: EndPadded with SetPaddedSize                              */
static int test_endpadded_paddedsize()
{
    array1_type in = make_fixture_1d();
    array1_type out;

    MHO_EndZeroPadder< array1_type > padder;
    padder.SetPaddedSize(6);
    padder.SetEndPadded();
    padder.SetArgs(&in, &out);
    REQUIRE(padder.Initialize());
    REQUIRE(padder.Execute());

    REQUIRE(out.GetSize() == 6);
    for(std::size_t i = 0; i < 4; i++)
    {
        REQUIRE_CLOSE_CPLX(out[i], in[i], 1e-14);
    }
    for(std::size_t i = 4; i < 6; i++)
    {
        REQUIRE_CLOSE_CPLX(out[i], std::complex< double >(0, 0), 1e-14);
    }

    return 0;
}

/* Case 3: ReverseEndPadded NormFXMode default ON                    */
static int test_reverse_endpadded_normfx_on()
{
    array1_type in = make_fixture_1d();
    array1_type out;

    MHO_EndZeroPadder< array1_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetReverseEndPadded();
    padder.SetArgs(&in, &out);
    REQUIRE(padder.Initialize());
    REQUIRE(padder.Execute());

    REQUIRE(out.GetSize() == 8);

    /* NormFXMode-on mapping:
     *   in_index 0 -> out_index N_out/2 = 4
     *   in_index k>0 -> out_index N_out - k
     * Expected: {(0),(0),(0),(0),(1,0),(4,0),(3,0),(2,0)}          */
    std::complex< double > expected[8];
    for(std::size_t i = 0; i < 8; i++)
        expected[i] = std::complex< double >(0, 0);
    expected[4] = std::complex< double >(1, 0); // in[0]
    expected[7] = std::complex< double >(2, 0); // in[1]: 8-1=7
    expected[6] = std::complex< double >(3, 0); // in[2]: 8-2=6
    expected[5] = std::complex< double >(4, 0); // in[3]: 8-3=5

    for(std::size_t i = 0; i < 8; i++)
    {
        REQUIRE_CLOSE_CPLX(out[i], expected[i], 1e-14);
    }

    return 0;
}

/* Case 4: ReverseEndPadded NormFXMode DISABLED                      */
static int test_reverse_endpadded_normfx_off()
{
    array1_type in = make_fixture_1d();
    array1_type out;

    MHO_EndZeroPadder< array1_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetReverseEndPadded();
    padder.DisableNormFXMode();
    padder.SetArgs(&in, &out);
    REQUIRE(padder.Initialize());
    REQUIRE(padder.Execute());

    REQUIRE(out.GetSize() == 8);

    /* NormFXMode-off mapping: out_index = N_out - 1 - in_index
     * Expected: {(0),(0),(0),(0),(4,0),(3,0),(2,0),(1,0)}          */
    std::complex< double > expected[8];
    for(std::size_t i = 0; i < 8; i++)
        expected[i] = std::complex< double >(0, 0);
    expected[7] = std::complex< double >(1, 0); // in[0]: 7-0=7
    expected[6] = std::complex< double >(2, 0); // in[1]: 7-1=6
    expected[5] = std::complex< double >(3, 0); // in[2]: 7-2=5
    expected[4] = std::complex< double >(4, 0); // in[3]: 7-3=4

    for(std::size_t i = 0; i < 8; i++)
    {
        REQUIRE_CLOSE_CPLX(out[i], expected[i], 1e-14);
    }

    return 0;
}

/* Case 5: 2-D padding on both axes (EndPadded)                      */
static int test_2d_pad_both_axes()
{
    array2_type in = make_fixture_2d();
    array2_type out;

    MHO_EndZeroPadder< array2_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetEndPadded();
    padder.SetArgs(&in, &out);
    REQUIRE(padder.Initialize());
    REQUIRE(padder.Execute());

    /* output dims = 6x8 */
    std::size_t dims[2];
    out.GetDimensions(dims);
    REQUIRE(dims[0] == 6);
    REQUIRE(dims[1] == 8);

    /* Data region i<3 AND j<4 matches input */
    for(std::size_t i = 0; i < 3; i++)
    {
        for(std::size_t j = 0; j < 4; j++)
        {
            REQUIRE_CLOSE_CPLX(out(i, j), in(i, j), 1e-14);
        }
    }
    /* Padded region is zero */
    for(std::size_t i = 0; i < 6; i++)
    {
        for(std::size_t j = 0; j < 8; j++)
        {
            if(i >= 3 || j >= 4)
            {
                REQUIRE_CLOSE_CPLX(out(i, j), std::complex< double >(0, 0), 1e-14);
            }
        }
    }

    return 0;
}

/* Case 6: Per-axis selection: pad only axis 1                       */
static int test_per_axis_pad_axis1()
{
    array2_type in = make_fixture_2d();
    array2_type out;

    MHO_EndZeroPadder< array2_type > padder;
    padder.DeselectAllAxes();
    padder.SelectAxis(1);
    padder.SetPaddingFactor(2);
    padder.SetEndPadded();
    padder.SetArgs(&in, &out);
    REQUIRE(padder.Initialize());
    REQUIRE(padder.Execute());

    /* output dims = 3x8 */
    std::size_t dims[2];
    out.GetDimensions(dims);
    REQUIRE(dims[0] == 3);
    REQUIRE(dims[1] == 8);

    /* All data matches for j<4; zero for j>=4 */
    for(std::size_t i = 0; i < 3; i++)
    {
        for(std::size_t j = 0; j < 4; j++)
        {
            REQUIRE_CLOSE_CPLX(out(i, j), in(i, j), 1e-14);
        }
        for(std::size_t j = 4; j < 8; j++)
        {
            REQUIRE_CLOSE_CPLX(out(i, j), std::complex< double >(0, 0), 1e-14);
        }
    }

    return 0;
}

/* Case 7: In-place vs out-of-place equivalence                      */
static int test_inplace_vs_oop_equivalence()
{
    array2_type in_oop = make_fixture_2d();
    array2_type in_ip = make_fixture_2d();
    array2_type out_oop;

    /* Out-of-place */
    {
        MHO_EndZeroPadder< array2_type > padder;
        padder.SetPaddingFactor(2);
        padder.SetEndPadded();
        padder.SetArgs(&in_oop, &out_oop);
        REQUIRE(padder.Initialize());
        REQUIRE(padder.Execute());
    }

    /* In-place */
    {
        MHO_EndZeroPadder< array2_type > padder;
        padder.SetPaddingFactor(2);
        padder.SetEndPadded();
        padder.SetArgs(&in_ip);
        REQUIRE(padder.Initialize());
        REQUIRE(padder.Execute());
    }

    /* Results should be element-wise equal */
    std::size_t dims[2];
    in_ip.GetDimensions(dims);
    REQUIRE(dims[0] == 6);
    REQUIRE(dims[1] == 8);

    for(std::size_t i = 0; i < dims[0]; i++)
    {
        for(std::size_t j = 0; j < dims[1]; j++)
        {
            REQUIRE_CLOSE_CPLX(in_ip(i, j), out_oop(i, j), 1e-14);
        }
    }

    return 0;
}

/* Case 8: fPreserveWorkspace round-trip                             */
static int test_preserve_workspace()
{
    MHO_EndZeroPadder< array1_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetEndPadded();
    padder.PreserveWorkspace();

    /* First execution */
    {
        array1_type in = make_fixture_1d();
        padder.SetArgs(&in);
        REQUIRE(padder.Initialize());
        REQUIRE(padder.Execute());
        REQUIRE(in.GetSize() == 8);
        for(std::size_t i = 0; i < 4; i++)
        {
            REQUIRE_CLOSE_CPLX(in[i], std::complex< double >((double)(i + 1), 0), 1e-14);
        }
        for(std::size_t i = 4; i < 8; i++)
        {
            REQUIRE_CLOSE_CPLX(in[i], std::complex< double >(0, 0), 1e-14);
        }
    }

    /* Second execution with different input of same shape */
    {
        array1_type in;
        in.Resize(4);
        for(std::size_t i = 0; i < 4; i++)
            in[i] = std::complex< double >(10.0 * (double)(i + 1), 0.0);
        padder.SetArgs(&in);
        REQUIRE(padder.Initialize());
        REQUIRE(padder.Execute());
        REQUIRE(in.GetSize() == 8);
        for(std::size_t i = 0; i < 4; i++)
        {
            REQUIRE_CLOSE_CPLX(in[i], std::complex< double >(10.0 * (double)(i + 1), 0), 1e-14);
        }
        for(std::size_t i = 4; i < 8; i++)
        {
            REQUIRE_CLOSE_CPLX(in[i], std::complex< double >(0, 0), 1e-14);
        }
    }

    return 0;
}

/* Case 9: fPreserveWorkspace OFF (default)                          */
static int test_no_preserve_workspace()
{
    MHO_EndZeroPadder< array1_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetEndPadded();
    padder.DoNotPreserveWorkspace(); // explicitly set default

    /* First execution */
    {
        array1_type in = make_fixture_1d();
        padder.SetArgs(&in);
        REQUIRE(padder.Initialize());
        REQUIRE(padder.Execute());
        REQUIRE(in.GetSize() == 8);
        for(std::size_t i = 0; i < 4; i++)
        {
            REQUIRE_CLOSE_CPLX(in[i], std::complex< double >((double)(i + 1), 0), 1e-14);
        }
        for(std::size_t i = 4; i < 8; i++)
        {
            REQUIRE_CLOSE_CPLX(in[i], std::complex< double >(0, 0), 1e-14);
        }
    }

    /* Re-initialize and execute again -- workspace should be re-allocated */
    {
        array1_type in;
        in.Resize(4);
        for(std::size_t i = 0; i < 4; i++)
            in[i] = std::complex< double >(100.0 * (double)(i + 1), 0.0);
        padder.SetArgs(&in);
        REQUIRE(padder.Initialize());
        REQUIRE(padder.Execute());
        REQUIRE(in.GetSize() == 8);
        for(std::size_t i = 0; i < 4; i++)
        {
            REQUIRE_CLOSE_CPLX(in[i], std::complex< double >(100.0 * (double)(i + 1), 0), 1e-14);
        }
        for(std::size_t i = 4; i < 8; i++)
        {
            REQUIRE_CLOSE_CPLX(in[i], std::complex< double >(0, 0), 1e-14);
        }
    }

    return 0;
}

/* Case 10: TableContainer axis-label extension (EndPadded)           */
static int test_table_axis_endpadded()
{
    table1_type in = make_fixture_table1d();
    table1_type out;

    MHO_EndZeroPadder< table1_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetEndPadded();
    padder.EnableTagCopy();
    padder.SetArgs(&in, &out);
    REQUIRE(padder.Initialize());
    REQUIRE(padder.Execute());

    REQUIRE(out.GetSize() == 8);

    /* axis labels: {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5} */
    auto& axis = std::get< 0 >(out);
    double expected[8] = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5};
    for(std::size_t i = 0; i < 8; i++)
    {
        CHECK_CLOSE(axis[i], expected[i], 1e-14);
    }

    return 0;
}

/* Case 11: TableContainer axis-label extension (ReverseEndPadded)   */
static int test_table_axis_reverse_endpadded()
{
    table1_type in = make_fixture_table1d();
    table1_type out;

    MHO_EndZeroPadder< table1_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetReverseEndPadded();
    padder.SetArgs(&in, &out);
    REQUIRE(padder.Initialize());
    REQUIRE(padder.Execute());

    REQUIRE(out.GetSize() == 8);

    /* axis labels:
     *   for i < ax1_size: axis2(i) = axis1(ax1_size - 1 - i)
     *   for i >= ax1_size: axis2(i) = axis1(0) - (i - (ax1_size-1))*delta
     * axis2 = {1.5, 1.0, 0.5, 0.0, -0.5, -1.0, -1.5, -2.0}       */
    auto& axis = std::get< 0 >(out);
    double expected[8] = {1.5, 1.0, 0.5, 0.0, -0.5, -1.0, -1.5, -2.0};
    for(std::size_t i = 0; i < 8; i++)
    {
        CHECK_CLOSE(axis[i], expected[i], 1e-14);
    }

    return 0;
}

/* Case 12: nullptr safety                                           */
/* Note: SetArgs(nullptr) uses in-place mode which would crash on
 * ExecuteInPlace(nullptr). We use out-of-place mode to test the
 * nullptr guard in InitializeOutOfPlace.                     */
static int test_nullptr_safety()
{
    array1_type out;

    MHO_EndZeroPadder< array1_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetArgs(nullptr, &out);
    REQUIRE(padder.Initialize() == false);
    REQUIRE(padder.Execute() == false);

    return 0;
}

/* Case 13: in == out for out-of-place                               */
static int test_same_pointer_oop()
{
    array1_type arr = make_fixture_1d();

    MHO_EndZeroPadder< array1_type > padder;
    padder.SetPaddingFactor(2);
    padder.SetArgs(&arr, &arr); // same pointer for in and out
    REQUIRE(padder.Initialize() == false);
    REQUIRE(padder.Execute() == false);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if(test_endpadded_factor2())
        return 1;
    if(test_endpadded_paddedsize())
        return 1;
    if(test_reverse_endpadded_normfx_on())
        return 1;
    if(test_reverse_endpadded_normfx_off())
        return 1;
    if(test_2d_pad_both_axes())
        return 1;
    if(test_per_axis_pad_axis1())
        return 1;
    if(test_inplace_vs_oop_equivalence())
        return 1;
    if(test_preserve_workspace())
        return 1;
    if(test_no_preserve_workspace())
        return 1;
    if(test_table_axis_endpadded())
        return 1;
    if(test_table_axis_reverse_endpadded())
        return 1;
    if(test_nullptr_safety())
        return 1;
    if(test_same_pointer_oop())
        return 1;

    return 0;
}
