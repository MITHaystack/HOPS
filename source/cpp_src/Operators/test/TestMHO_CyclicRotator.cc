#include <iostream>
#include <memory>
#include <string>

#include "MHO_CyclicRotator.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"

using namespace hops;

#include "MHO_TestAssertions.hh"

// Type aliases

using array1d_int_type = MHO_NDArrayWrapper< int, 1 >;
using array2d_int_type = MHO_NDArrayWrapper< int, 2 >;
using array3d_type = MHO_NDArrayWrapper< double, 3 >;
using table1d_type = MHO_TableContainer< double, MHO_AxisPack< MHO_Axis< double > > >;

// Fixtures

// (a) 1-D int, size 4, {0,1,2,3}
static array1d_int_type make_fixture_1d()
{
    array1d_int_type arr;
    arr.Resize(4);
    for(std::size_t i = 0; i < 4; i++)
        arr[i] = static_cast< int >(i);
    return arr;
}

// (b) 2-D int, size 3x4, v[i][j] = 10*i + j
static array2d_int_type make_fixture_2d()
{
    array2d_int_type arr;
    arr.Resize(3, 4);
    for(std::size_t i = 0; i < 3; i++)
    {
        for(std::size_t j = 0; j < 4; j++)
        {
            arr(i, j) = 10 * static_cast< int >(i) + static_cast< int >(j);
        }
    }
    return arr;
}

// (c) 3-D double, size 2x3x4, sequential
static array3d_type make_fixture_3d()
{
    array3d_type arr;
    arr.Resize(2, 3, 4);
    std::size_t count = 0;
    for(std::size_t i = 0; i < 2; i++)
    {
        for(std::size_t j = 0; j < 3; j++)
        {
            for(std::size_t k = 0; k < 4; k++)
            {
                arr(i, j, k) = static_cast< double >(count);
                count++;
            }
        }
    }
    return arr;
}

// (d) 1-D TableContainer<double>, size 5, values={1,2,3,4,5}, labels={10,20,30,40,50}
static table1d_type make_fixture_table1d()
{
    std::size_t dim = 5;
    table1d_type tbl;
    tbl.Resize(&dim);
    for(std::size_t i = 0; i < 5; i++)
        tbl[i] = static_cast< double >(i + 1);

    auto& axis = std::get< 0 >(tbl);
    axis.Resize(5);
    for(std::size_t i = 0; i < 5; i++)
        axis[i] = static_cast< double >((i + 1) * 10);

    return tbl;
}

// main()

int main()
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // ---- Test Case 1: 1-D left rotation by +1 ----
    {
        array1d_int_type arr = make_fixture_1d();
        MHO_CyclicRotator< array1d_int_type > rot;
        rot.SetOffset(0, 1);
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 3);
        REQUIRE(arr[3] == 0);
    }

    // ---- Test Case 2: 1-D right rotation by -1 ----
    {
        array1d_int_type arr = make_fixture_1d();
        MHO_CyclicRotator< array1d_int_type > rot;
        rot.SetOffset(0, -1);
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        REQUIRE(arr[0] == 3);
        REQUIRE(arr[1] == 0);
        REQUIRE(arr[2] == 1);
        REQUIRE(arr[3] == 2);
    }

    // ---- Test Case 3: 1-D large positive (mod wrap) ----
    {
        array1d_int_type arr = make_fixture_1d();
        MHO_CyclicRotator< array1d_int_type > rot;
        rot.SetOffset(0, 5); // +5 mod 4 = +1, same as left rotation by 1
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 3);
        REQUIRE(arr[3] == 0);
    }

    // ---- Test Case 4: 1-D large negative ----
    {
        array1d_int_type arr = make_fixture_1d();
        MHO_CyclicRotator< array1d_int_type > rot;
        rot.SetOffset(0, -5); // -5 mod 4 = -1, same as right rotation by 1
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        REQUIRE(arr[0] == 3);
        REQUIRE(arr[1] == 0);
        REQUIRE(arr[2] == 1);
        REQUIRE(arr[3] == 2);
    }

    // ---- Test Case 5: 1-D rotation by 0 ----
    {
        array1d_int_type arr = make_fixture_1d();
        MHO_CyclicRotator< array1d_int_type > rot;
        rot.SetOffset(0, 0);
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        REQUIRE(arr[0] == 0);
        REQUIRE(arr[1] == 1);
        REQUIRE(arr[2] == 2);
        REQUIRE(arr[3] == 3);
    }

    // ---- Test Case 6: 1-D rotation by N (full cycle) ----
    {
        array1d_int_type arr = make_fixture_1d();
        MHO_CyclicRotator< array1d_int_type > rot;
        rot.SetOffset(0, 4); // full cycle
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        REQUIRE(arr[0] == 0);
        REQUIRE(arr[1] == 1);
        REQUIRE(arr[2] == 2);
        REQUIRE(arr[3] == 3);
    }

    // ---- Test Case 7: 2-D rotation along axis 1 only ----
    {
        array2d_int_type arr = make_fixture_2d();
        MHO_CyclicRotator< array2d_int_type > rot;
        rot.SetOffset(0, 0);
        rot.SetOffset(1, 2);
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        // row 0: {0,1,2,3} -> left by 2 -> {2,3,0,1}
        REQUIRE(arr(0, 0) == 2);
        REQUIRE(arr(0, 1) == 3);
        REQUIRE(arr(0, 2) == 0);
        REQUIRE(arr(0, 3) == 1);
        // row 1: {10,11,12,13} -> {12,13,10,11}
        REQUIRE(arr(1, 0) == 12);
        REQUIRE(arr(1, 1) == 13);
        REQUIRE(arr(1, 2) == 10);
        REQUIRE(arr(1, 3) == 11);
        // row 2: {20,21,22,23} -> {22,23,20,21}
        REQUIRE(arr(2, 0) == 22);
        REQUIRE(arr(2, 1) == 23);
        REQUIRE(arr(2, 2) == 20);
        REQUIRE(arr(2, 3) == 21);
    }

    // ---- Test Case 8: 2-D rotation along both axes ----
    {
        array2d_int_type arr = make_fixture_2d();
        MHO_CyclicRotator< array2d_int_type > rot;
        rot.SetOffset(0, 1);
        rot.SetOffset(1, 2);
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        // result[i][j] = old[(i+1)%3][(j+2)%4]
        // = {{12,13,10,11},{22,23,20,21},{2,3,0,1}}
        int expected[3][4] = {
            {12, 13, 10, 11},
            {22, 23, 20, 21},
            {2,  3,  0,  1 }
        };
        for(std::size_t i = 0; i < 3; i++)
        {
            for(std::size_t j = 0; j < 4; j++)
            {
                REQUIRE(arr(i, j) == expected[i][j]);
            }
        }
    }

    // ---- Test Case 9: 3-D rotation along axis 2 only ----
    {
        array3d_type arr = make_fixture_3d();
        MHO_CyclicRotator< array3d_type > rot;
        rot.SetOffset(0, 0);
        rot.SetOffset(1, 0);
        rot.SetOffset(2, 1);
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        // Each row (axis 2) is rotated left by 1.
        // Original axis-2 slices are {0,1,2,3},{4,5,6,7},{8,9,10,11},
        // {12,13,14,15},{16,17,18,19},{20,21,22,23}
        // After left-rotate by 1:
        // {1,2,3,0},{5,6,7,4},{9,10,11,8},{13,14,15,12},{17,18,19,16},{21,22,23,20}
        double expected[2][3][4] = {
            {{1, 2, 3, 0},     {5, 6, 7, 4},     {9, 10, 11, 8}  },
            {{13, 14, 15, 12}, {17, 18, 19, 16}, {21, 22, 23, 20}}
        };
        for(std::size_t i = 0; i < 2; i++)
        {
            for(std::size_t j = 0; j < 3; j++)
            {
                for(std::size_t k = 0; k < 4; k++)
                {
                    REQUIRE(arr(i, j, k) == expected[i][j][k]);
                }
            }
        }
    }

    // ---- Test Case 10: Out-of-place equivalence ----
    {
        array1d_int_type arr_ip = make_fixture_1d();
        array1d_int_type arr_oop = make_fixture_1d();
        array1d_int_type out;

        // In-place
        {
            MHO_CyclicRotator< array1d_int_type > rot;
            rot.SetOffset(0, 2);
            rot.SetArgs(&arr_ip);
            REQUIRE(rot.Initialize());
            REQUIRE(rot.Execute());
        }
        // Out-of-place
        {
            MHO_CyclicRotator< array1d_int_type > rot;
            rot.SetOffset(0, 2);
            rot.SetArgs(&arr_oop, &out);
            REQUIRE(rot.Initialize());
            REQUIRE(rot.Execute());
        }
        // Compare results
        for(std::size_t i = 0; i < 4; i++)
        {
            REQUIRE(arr_ip[i] == out[i]);
        }
    }

    // ---- Test Case 11: Re-init after SetOffset ----
    {
        array1d_int_type arr = make_fixture_1d();
        MHO_CyclicRotator< array1d_int_type > rot;

        // First rotation: +1
        rot.SetOffset(0, 1);
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        REQUIRE(arr[0] == 1);

        // Second rotation: -1 (undo the first)
        rot.SetOffset(0, -1);
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());
        REQUIRE(arr[0] == 0);
        REQUIRE(arr[1] == 1);
        REQUIRE(arr[2] == 2);
        REQUIRE(arr[3] == 3);
    }

    // ---- Test Case 12: TableContainer axis rotation ----
    {
        table1d_type arr = make_fixture_table1d();
        MHO_CyclicRotator< table1d_type > rot;
        rot.SetOffset(0, 1);
        rot.SetArgs(&arr);
        REQUIRE(rot.Initialize());
        REQUIRE(rot.Execute());

        // Values: {1,2,3,4,5} -> left by 1 -> {2,3,4,5,1}
        REQUIRE(arr[0] == 2.0);
        REQUIRE(arr[1] == 3.0);
        REQUIRE(arr[2] == 4.0);
        REQUIRE(arr[3] == 5.0);
        REQUIRE(arr[4] == 1.0);

        // Labels: {10,20,30,40,50} -> right by 1 -> {50,10,20,30,40}
        // The RotateAxis operator uses axis2(i) = tmp((i - offset) % n),
        // which rotates labels in the opposite direction to the data.
        auto& axis = std::get< 0 >(arr);
        REQUIRE(axis[0] == 50.0);
        REQUIRE(axis[1] == 10.0);
        REQUIRE(axis[2] == 20.0);
        REQUIRE(axis[3] == 30.0);
        REQUIRE(axis[4] == 40.0);
    }

    // ---- Test Case 13: Invalid dimension index ----
    {
        array1d_int_type arr = make_fixture_1d();

        // Save original values
        int original[4] = {0, 1, 2, 3};

        // Suppress msg_error
        MHO_Message::GetInstance().SetMessageLevel(eSilent);

        MHO_CyclicRotator< array1d_int_type > rot;
        rot.SetOffset(5, 1); // dim 5 > rank 1, should only log msg_error
        rot.SetArgs(&arr);
        bool init = rot.Initialize();
        bool exe = rot.Execute();

        // Restore message level
        MHO_Message::GetInstance().SetMessageLevel(eFatal);

        REQUIRE(init == true); // Initialize still succeeds (offset for dim 0 is 0)
        REQUIRE(exe == true);  // Execute still succeeds (no-op since offset[0]=0)

        // Array should be unchanged since no valid offset was set
        for(std::size_t i = 0; i < 4; i++)
        {
            REQUIRE(arr[i] == original[i]);
        }
    }

    // ---- Test Case 14: nullptr input ----
    {
        array1d_int_type out;
        MHO_CyclicRotator< array1d_int_type > rot;
        rot.SetArgs(nullptr, &out);
        REQUIRE(rot.Initialize() == false);
        REQUIRE(rot.Execute() == false);
    }

    return 0;
}
