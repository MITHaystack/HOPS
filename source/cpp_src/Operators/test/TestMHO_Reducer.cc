#include <cmath>
#include <complex>
#include <iostream>
#include <memory>
#include <string>

#include "MHO_CompoundReductions.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_Reducer.hh"
#include "MHO_TableContainer.hh"

using namespace hops;

// Type aliases

using complex1d = MHO_NDArrayWrapper< std::complex< double >, 1 >;
using double1d = MHO_NDArrayWrapper< double, 1 >;
using complex2d = MHO_NDArrayWrapper< std::complex< double >, 2 >;

using table2_type = MHO_TableContainer< std::complex< double >, MHO_AxisPack< MHO_Axis< double >, MHO_Axis< double > > >;

// Fixtures

//1-D double array: {1,2,3,4,5}
static double1d make_fixture_1d()
{
    double1d arr;
    std::size_t dim = 5;
    arr.Resize(&dim);
    double data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    for(std::size_t i = 0; i < 5; i++)
        arr[i] = data[i];
    return arr;
}

//2-D complex array: in(i,j) = complex(i+1, j), shape 2x3
static complex2d make_fixture_2d()
{
    complex2d arr;
    std::size_t dim[] = {2, 3};
    arr.Resize(dim);
    for(std::size_t i = 0; i < 2; i++)
    {
        for(std::size_t j = 0; j < 3; j++)
        {
            arr(i, j) = std::complex< double >((double)(i + 1), (double)j);
        }
    }
    return arr;
}

//2-D TableContainer complex, shape 2x3, in(i,j) = complex(i+1, j)
static table2_type make_fixture_table2d()
{
    std::size_t dim[] = {2, 3};
    table2_type tbl;
    tbl.Resize(dim);
    for(std::size_t i = 0; i < 2; i++)
    {
        for(std::size_t j = 0; j < 3; j++)
        {
            tbl(i, j) = std::complex< double >((double)(i + 1), (double)j);
        }
    }
    // set axis labels
    auto& axis0 = std::get< 0 >(tbl);
    auto& axis1 = std::get< 1 >(tbl);
    axis0.Resize(2);
    axis1.Resize(3);
    axis0[0] = 0.0;
    axis0[1] = 1.0;
    axis1[0] = 10.0;
    axis1[1] = 20.0;
    axis1[2] = 30.0;
    return tbl;
}

#include "MHO_TestAssertions.hh"

// main()

int main()
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal); // suppress non-fatal messages

    // ---- Test Case 1: 1-D sum reduction out-of-place (double) ----
    {
        double1d in = make_fixture_1d();
        double1d out;
        MHO_Reducer< double1d, MHO_CompoundSum > reducer;
        reducer.SetArgs(&in, &out);
        reducer.ReduceAxis(0);
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        std::size_t out_dim[1];
        out.GetDimensions(out_dim);
        REQUIRE(out_dim[0] == 1);
        CHECK_CLOSE(out[0], 15.0, 1e-14);
    }

    // ---- Test Case 2: 1-D product reduction out-of-place ----
    {
        double1d in = make_fixture_1d();
        double1d out;
        MHO_Reducer< double1d, MHO_CompoundMultiply > reducer;
        reducer.SetArgs(&in, &out);
        reducer.ReduceAxis(0);
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        std::size_t out_dim[1];
        out.GetDimensions(out_dim);
        REQUIRE(out_dim[0] == 1);
        CHECK_CLOSE(out[0], 120.0, 1e-14);
    }

    // ---- Test Case 3: 2-D row-sum (complex) - reduce axis 1 ----
    {
        complex2d in = make_fixture_2d();
        complex2d out;
        MHO_Reducer< complex2d, MHO_CompoundSum > reducer;
        reducer.SetArgs(&in, &out);
        reducer.ReduceAxis(1);
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        std::size_t out_dim[2];
        out.GetDimensions(out_dim);
        REQUIRE(out_dim[0] == 2);
        REQUIRE(out_dim[1] == 1);
        REQUIRE_CLOSE_CPLX(out(0, 0), std::complex< double >(3.0, 3.0), 1e-14);
        REQUIRE_CLOSE_CPLX(out(1, 0), std::complex< double >(6.0, 3.0), 1e-14);
    }

    // ---- Test Case 4: 2-D column-sum (complex) - reduce axis 0 ----
    {
        complex2d in = make_fixture_2d();
        complex2d out;
        MHO_Reducer< complex2d, MHO_CompoundSum > reducer;
        reducer.SetArgs(&in, &out);
        reducer.ReduceAxis(0);
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        std::size_t out_dim[2];
        out.GetDimensions(out_dim);
        REQUIRE(out_dim[0] == 1);
        REQUIRE(out_dim[1] == 3);
        REQUIRE_CLOSE_CPLX(out(0, 0), std::complex< double >(3.0, 0.0), 1e-14);
        REQUIRE_CLOSE_CPLX(out(0, 1), std::complex< double >(3.0, 2.0), 1e-14);
        REQUIRE_CLOSE_CPLX(out(0, 2), std::complex< double >(3.0, 4.0), 1e-14);
    }

    // ---- Test Case 5: Multi-axis reduction - reduce both axes ----
    {
        complex2d in = make_fixture_2d();
        complex2d out;
        MHO_Reducer< complex2d, MHO_CompoundSum > reducer;
        reducer.SetArgs(&in, &out);
        reducer.ReduceAxis(0);
        reducer.ReduceAxis(1);
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        std::size_t out_dim[2];
        out.GetDimensions(out_dim);
        REQUIRE(out_dim[0] == 1);
        REQUIRE(out_dim[1] == 1);
        REQUIRE_CLOSE_CPLX(out(0, 0), std::complex< double >(9.0, 6.0), 1e-14);
    }

    // ---- Test Case 6: In-place sum reduction ----
    {
        double1d arr = make_fixture_1d();
        MHO_Reducer< double1d, MHO_CompoundSum > reducer;
        reducer.SetArgs(&arr); // in-place mode
        reducer.ReduceAxis(0);
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        std::size_t out_dim[1];
        arr.GetDimensions(out_dim);
        REQUIRE(out_dim[0] == 1);
        CHECK_CLOSE(arr[0], 15.0, 1e-14);
    }

    // ---- Test Case 7: Invalid axis index (>=rank) - silently rejected ----
    {
        double1d in = make_fixture_1d();
        double1d out;
        MHO_Reducer< double1d, MHO_CompoundSum > reducer;
        reducer.SetArgs(&in, &out);
        // axis 7 >= rank 1 -> silently rejected (msg_error already suppressed to eFatal)
        reducer.ReduceAxis(7);
        // No axis selected -> InitializeOutOfPlace should still succeed but
        // output dimensions should equal input (no reduction applied)
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        std::size_t in_dim[1], out_dim[1];
        in.GetDimensions(in_dim);
        out.GetDimensions(out_dim);
        // output should be same shape as input since no valid axis was reduced
        REQUIRE(out_dim[0] == in_dim[0]);
        // and values should match input (identity copy)
        for(std::size_t i = 0; i < in_dim[0]; i++)
        {
            CHECK_CLOSE(out[i], in[i], 1e-14);
        }
    }

    // ---- Test Case 8: ClearAxisSelection + re-Initialize on different axis ----
    {
        complex2d in = make_fixture_2d();
        complex2d out;
        MHO_Reducer< complex2d, MHO_CompoundSum > reducer;
        reducer.SetArgs(&in, &out);
        reducer.ReduceAxis(1); // first select axis 1
        reducer.ClearAxisSelection();
        reducer.ReduceAxis(0); // then switch to axis 0
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        std::size_t out_dim[2];
        out.GetDimensions(out_dim);
        REQUIRE(out_dim[0] == 1);
        REQUIRE(out_dim[1] == 3);
        // column sums: out(0,j) = in(0,j)+in(1,j) = (1,0)+(2,0)=(3,0), etc.
        REQUIRE_CLOSE_CPLX(out(0, 0), std::complex< double >(3.0, 0.0), 1e-14);
        REQUIRE_CLOSE_CPLX(out(0, 1), std::complex< double >(3.0, 2.0), 1e-14);
        REQUIRE_CLOSE_CPLX(out(0, 2), std::complex< double >(3.0, 4.0), 1e-14);
    }

    // ---- Test Case 9: Re-Initialize after axis change - reuse operator ----
    {
        MHO_Reducer< double1d, MHO_CompoundSum > reducer;
        double1d in1 = make_fixture_1d();
        double1d out1;
        reducer.SetArgs(&in1, &out1);
        reducer.ReduceAxis(0);
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        CHECK_CLOSE(out1[0], 15.0, 1e-14);

        // Reuse operator with new data (no axis change needed for 1-D)
        double1d in2;
        std::size_t dim = 3;
        in2.Resize(&dim);
        in2[0] = 10.0;
        in2[1] = 20.0;
        in2[2] = 30.0;
        double1d out2;
        reducer.SetArgs(&in2, &out2);
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());
        CHECK_CLOSE(out2[0], 60.0, 1e-14);
    }

    // ---- Test Case 10: Identity initialization ----
    {
        // Sum of {0,0,0,0} = 0
        double1d in_sum;
        std::size_t dim = 4;
        in_sum.Resize(&dim);
        in_sum.SetArray(0.0);
        double1d out_sum;
        {
            MHO_Reducer< double1d, MHO_CompoundSum > reducer;
            reducer.SetArgs(&in_sum, &out_sum);
            reducer.ReduceAxis(0);
            REQUIRE(reducer.Initialize());
            REQUIRE(reducer.Execute());
        }
        CHECK_CLOSE(out_sum[0], 0.0, 1e-14);

        // Product of {1,1,1,1} = 1
        double1d in_prod;
        in_prod.Resize(&dim);
        in_prod.SetArray(1.0);
        double1d out_prod;
        {
            MHO_Reducer< double1d, MHO_CompoundMultiply > reducer;
            reducer.SetArgs(&in_prod, &out_prod);
            reducer.ReduceAxis(0);
            REQUIRE(reducer.Initialize());
            REQUIRE(reducer.Execute());
        }
        CHECK_CLOSE(out_prod[0], 1.0, 1e-14);
    }

    // ---- Test Case 11: TableContainer axis-label collapse ----
    {
        table2_type in = make_fixture_table2d();
        table2_type out;
        MHO_Reducer< table2_type, MHO_CompoundSum > reducer;
        reducer.SetArgs(&in, &out);
        reducer.ReduceAxis(1);
        REQUIRE(reducer.Initialize());
        REQUIRE(reducer.Execute());

        std::size_t out_dim[2];
        out.GetDimensions(out_dim);
        REQUIRE(out_dim[0] == 2);
        REQUIRE(out_dim[1] == 1);

        // Data values: row sums
        REQUIRE_CLOSE_CPLX(out(0, 0), std::complex< double >(3.0, 3.0), 1e-14);
        REQUIRE_CLOSE_CPLX(out(1, 0), std::complex< double >(6.0, 3.0), 1e-14);

        // Axis 0 should be copied (not reduced)
        auto& o_axis0 = std::get< 0 >(out);
        auto& i_axis0 = std::get< 0 >(in);
        REQUIRE(o_axis0.GetSize() == i_axis0.GetSize());
        for(std::size_t i = 0; i < o_axis0.GetSize(); i++)
        {
            CHECK_CLOSE(o_axis0[i], i_axis0[i], 1e-14);
        }

        // Axis 1 should be collapsed to size 1, label = first label of original axis
        auto& o_axis1 = std::get< 1 >(out);
        auto& i_axis1 = std::get< 1 >(in);
        REQUIRE(o_axis1.GetSize() == 1);
        CHECK_CLOSE(o_axis1[0], i_axis1[0], 1e-14);
    }

    return 0;
}
