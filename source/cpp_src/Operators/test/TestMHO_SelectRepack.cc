#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

#include "MHO_Axis.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_SelectRepack.hh"
#include "MHO_TableContainer.hh"

using namespace hops;

// Type aliases

using arr2d = MHO_NDArrayWrapper< double, 2 >;
using arr3d = MHO_NDArrayWrapper< double, 3 >;

using table2_type = MHO_TableContainer< double, MHO_AxisPack< MHO_Axis< int >, MHO_Axis< double > > >;

#include "MHO_TestAssertions.hh"

// Fixture helpers

//2-D array 4x5: a(i,j) = i*100 + j
static arr2d make_fixture_2d()
{
    arr2d arr;
    std::size_t dim[] = {4, 5};
    arr.Resize(dim);
    for(std::size_t i = 0; i < 4; ++i)
        for(std::size_t j = 0; j < 5; ++j)
            arr(i, j) = static_cast< double >(i * 100 + j);
    return arr;
}

//3-D array 3x4x2: a(i,j,k) = i*100 + j*10 + k
static arr3d make_fixture_3d()
{
    arr3d arr;
    std::size_t dim[] = {3, 4, 2};
    arr.Resize(dim);
    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 4; ++j)
            for(std::size_t k = 0; k < 2; ++k)
                arr(i, j, k) = static_cast< double >(i * 100 + j * 10 + k);
    return arr;
}

//2-D TableContainer 4x5: a(i,j) = i*100+j; axis0(i)=i; axis1(j)=j*0.5
static table2_type make_fixture_table()
{
    std::size_t dim[] = {4, 5};
    table2_type tbl;
    tbl.Resize(dim);
    for(std::size_t i = 0; i < 4; ++i)
        for(std::size_t j = 0; j < 5; ++j)
            tbl(i, j) = static_cast< double >(i * 100 + j);

    auto& axis0 = std::get< 0 >(tbl);
    auto& axis1 = std::get< 1 >(tbl);
    axis0.Resize(4);
    axis1.Resize(5);
    for(std::size_t i = 0; i < 4; ++i)
        axis0[i] = static_cast< int >(i);
    for(std::size_t j = 0; j < 5; ++j)
        axis1[j] = static_cast< double >(j) * 0.5;
    return tbl;
}

// Test cases

//CASE 1 - Single-axis selection (partial branch), 2-D
static int test_case1()
{
    arr2d in = make_fixture_2d();
    arr2d out;
    MHO_SelectRepack< arr2d > op;
    op.SelectAxisItems(1, std::vector< std::size_t >{0, 2, 4});
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out.GetDimensionArray();
    REQUIRE(odim[0] == 4);
    REQUIRE(odim[1] == 3);

    for(std::size_t i = 0; i < 4; ++i)
    {
        CHECK_CLOSE(out(i, 0), static_cast< double >(i * 100 + 0), 1e-12);
        CHECK_CLOSE(out(i, 1), static_cast< double >(i * 100 + 2), 1e-12);
        CHECK_CLOSE(out(i, 2), static_cast< double >(i * 100 + 4), 1e-12);
    }
    return 0;
}

//CASE 2 - Two-axis selection (partial branch), 2-D
static int test_case2()
{
    arr2d in = make_fixture_2d();
    arr2d out;
    MHO_SelectRepack< arr2d > op;
    op.SelectAxisItems(0, std::vector< std::size_t >{0, 3});
    op.SelectAxisItems(1, std::vector< std::size_t >{1, 4});
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out.GetDimensionArray();
    REQUIRE(odim[0] == 2);
    REQUIRE(odim[1] == 2);

    CHECK_CLOSE(out(0, 0), 1.0, 1e-12);   // 0*100+1
    CHECK_CLOSE(out(0, 1), 4.0, 1e-12);   // 0*100+4
    CHECK_CLOSE(out(1, 0), 301.0, 1e-12); // 3*100+1
    CHECK_CLOSE(out(1, 1), 304.0, 1e-12); // 3*100+4
    return 0;
}

//CASE 3 - All-axes-selected branch, 2-D (exercises size()==rank path)
static int test_case3()
{
    arr2d in = make_fixture_2d();
    arr2d out;
    MHO_SelectRepack< arr2d > op;
    op.SelectAxisItems(0, std::vector< std::size_t >{1, 2});
    op.SelectAxisItems(1, std::vector< std::size_t >{0, 3, 4});
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out.GetDimensionArray();
    REQUIRE(odim[0] == 2);
    REQUIRE(odim[1] == 3);

    // out(0,*) = row 1 of in -> {100, 103, 104}
    CHECK_CLOSE(out(0, 0), 100.0, 1e-12);
    CHECK_CLOSE(out(0, 1), 103.0, 1e-12);
    CHECK_CLOSE(out(0, 2), 104.0, 1e-12);
    // out(1,*) = row 2 of in -> {200, 203, 204}
    CHECK_CLOSE(out(1, 0), 200.0, 1e-12);
    CHECK_CLOSE(out(1, 1), 203.0, 1e-12);
    CHECK_CLOSE(out(1, 2), 204.0, 1e-12);
    return 0;
}

//CASE 4 - No selection -> full copy
static int test_case4()
{
    arr2d in = make_fixture_2d();
    arr2d out;
    MHO_SelectRepack< arr2d > op;
    // no SelectAxisItems calls
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out.GetDimensionArray();
    REQUIRE(odim[0] == 4);
    REQUIRE(odim[1] == 5);

    for(std::size_t i = 0; i < 4; ++i)
        for(std::size_t j = 0; j < 5; ++j)
            CHECK_CLOSE(out(i, j), in(i, j), 1e-12);
    return 0;
}

//CASE 5 - Select-all-indices identity
static int test_case5()
{
    arr2d in = make_fixture_2d();
    arr2d out;
    MHO_SelectRepack< arr2d > op;
    op.SelectAxisItems(0, std::vector< std::size_t >{0, 1, 2, 3});
    op.SelectAxisItems(1, std::vector< std::size_t >{0, 1, 2, 3, 4});
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out.GetDimensionArray();
    REQUIRE(odim[0] == 4);
    REQUIRE(odim[1] == 5);

    for(std::size_t i = 0; i < 4; ++i)
        for(std::size_t j = 0; j < 5; ++j)
            CHECK_CLOSE(out(i, j), in(i, j), 1e-12);
    return 0;
}

//CASE 6 - Unsorted indices are auto-sorted
static int test_case6()
{
    arr2d in = make_fixture_2d();
    arr2d out;
    MHO_SelectRepack< arr2d > op;
    op.SelectAxisItems(1, std::vector< std::size_t >{4, 0, 2}); // out of order
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out.GetDimensionArray();
    REQUIRE(odim[0] == 4);
    REQUIRE(odim[1] == 3);

    // Should match Case 1: sorted to {0,2,4}
    for(std::size_t i = 0; i < 4; ++i)
    {
        CHECK_CLOSE(out(i, 0), static_cast< double >(i * 100 + 0), 1e-12);
        CHECK_CLOSE(out(i, 1), static_cast< double >(i * 100 + 2), 1e-12);
        CHECK_CLOSE(out(i, 2), static_cast< double >(i * 100 + 4), 1e-12);
    }
    return 0;
}

//CASE 7 - 3-D selection on a single middle axis
static int test_case7()
{
    arr3d in = make_fixture_3d();
    arr3d out;
    MHO_SelectRepack< arr3d > op;
    op.SelectAxisItems(1, std::vector< std::size_t >{1, 3});
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out.GetDimensionArray();
    REQUIRE(odim[0] == 3);
    REQUIRE(odim[1] == 2);
    REQUIRE(odim[2] == 2);

    for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t k = 0; k < 2; ++k)
        {
            CHECK_CLOSE(out(i, 0, k), static_cast< double >(i * 100 + 1 * 10 + k), 1e-12);
            CHECK_CLOSE(out(i, 1, k), static_cast< double >(i * 100 + 3 * 10 + k), 1e-12);
        }
    return 0;
}

//CASE 8 - In-place execution
static int test_case8()
{
    arr2d in = make_fixture_2d();
    MHO_SelectRepack< arr2d > op;
    op.SelectAxisItems(1, std::vector< std::size_t >{0, 2, 4});
    op.SetArgs(&in); // in-place mode
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto idim = in.GetDimensionArray();
    REQUIRE(idim[0] == 4);
    REQUIRE(idim[1] == 3);

    for(std::size_t i = 0; i < 4; ++i)
    {
        CHECK_CLOSE(in(i, 0), static_cast< double >(i * 100 + 0), 1e-12);
        CHECK_CLOSE(in(i, 1), static_cast< double >(i * 100 + 2), 1e-12);
        CHECK_CLOSE(in(i, 2), static_cast< double >(i * 100 + 4), 1e-12);
    }
    return 0;
}

//CASE 9 - Table axis-label repacking
static int test_case9()
{
    table2_type in = make_fixture_table();
    table2_type out;
    MHO_SelectRepack< table2_type > op;
    op.SelectAxisItems(1, std::vector< std::size_t >{0, 2, 4});
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out.GetDimensionArray();
    REQUIRE(odim[0] == 4);
    REQUIRE(odim[1] == 3);

    // Data matches Case 1
    for(std::size_t i = 0; i < 4; ++i)
    {
        CHECK_CLOSE(out(i, 0), static_cast< double >(i * 100 + 0), 1e-12);
        CHECK_CLOSE(out(i, 1), static_cast< double >(i * 100 + 2), 1e-12);
        CHECK_CLOSE(out(i, 2), static_cast< double >(i * 100 + 4), 1e-12);
    }

    // Axis 0 unchanged
    auto& o_axis0 = std::get< 0 >(out);
    auto& i_axis0 = std::get< 0 >(in);
    REQUIRE(o_axis0.GetSize() == i_axis0.GetSize());
    for(std::size_t i = 0; i < o_axis0.GetSize(); ++i)
        CHECK_CLOSE(static_cast< double >(o_axis0[i]), static_cast< double >(i_axis0[i]), 1e-12);

    // Axis 1: labels should be {0.0, 1.0, 2.0} (= {0,2,4} * 0.5)
    auto& o_axis1 = std::get< 1 >(out);
    REQUIRE(o_axis1.GetSize() == 3);
    CHECK_CLOSE(o_axis1[0], 0.0, 1e-12);
    CHECK_CLOSE(o_axis1[1], 1.0, 1e-12);
    CHECK_CLOSE(o_axis1[2], 2.0, 1e-12);
    return 0;
}

//CASE 10 - Reset() clears selections
static int test_case10()
{
    arr2d in = make_fixture_2d();
    arr2d out;
    MHO_SelectRepack< arr2d > op;
    op.SelectAxisItems(1, std::vector< std::size_t >{0, 2, 4});
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Verify first run produced 4x3
    {
        auto odim = out.GetDimensionArray();
        REQUIRE(odim[0] == 4);
        REQUIRE(odim[1] == 3);
    }

    // Reset and re-run on fresh data (no selection)
    op.Reset();
    arr2d in2 = make_fixture_2d();
    arr2d out2;
    op.SetArgs(&in2, &out2);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out2.GetDimensionArray();
    REQUIRE(odim[0] == 4);
    REQUIRE(odim[1] == 5);

    for(std::size_t i = 0; i < 4; ++i)
        for(std::size_t j = 0; j < 5; ++j)
            CHECK_CLOSE(out2(i, j), in2(i, j), 1e-12);
    return 0;
}

//CASE 11 - Single-index selection (edge)
static int test_case11()
{
    arr2d in = make_fixture_2d();
    arr2d out;
    MHO_SelectRepack< arr2d > op;
    op.SelectAxisItems(0, std::vector< std::size_t >{2});
    op.SetArgs(&in, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto odim = out.GetDimensionArray();
    REQUIRE(odim[0] == 1);
    REQUIRE(odim[1] == 5);

    for(std::size_t j = 0; j < 5; ++j)
        CHECK_CLOSE(out(0, j), static_cast< double >(200 + j), 1e-12);
    return 0;
}

// main()

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
    if(test_case9())
        return 1;
    if(test_case10())
        return 1;
    if(test_case11())
        return 1;

    return 0;
}
