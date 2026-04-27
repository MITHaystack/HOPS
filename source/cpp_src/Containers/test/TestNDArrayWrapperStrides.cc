#include <cstdlib>
#include <iostream>

#include "MHO_NDArrayWrapper.hh"

using namespace hops;

// Regression test for the fStrides[0] typo bug in MHO_NDArrayWrapper::Construct().
// The bug zeroed only fStrides[0] on every loop iteration, leaving fStrides[1..RANK-1]
// uninitialized after default construction.

static int nfail = 0;

#define CHECK_EQ(label, got, expected)                                                                                      \
    do                                                                                                                      \
    {                                                                                                                       \
        if((got) != (expected))                                                                                             \
        {                                                                                                                   \
            std::cerr << "FAIL " << label << ": got " << (got) << ", expected " << (expected) << std::endl;                \
            ++nfail;                                                                                                        \
        }                                                                                                                   \
        else                                                                                                                \
        {                                                                                                                   \
            std::cout << "PASS " << label << std::endl;                                                                    \
        }                                                                                                                   \
    } while(0)

int main(int /*argc*/, char** /*argv*/)
{
    // --- Test 1: default construction zeros ALL strides ---
    {
        MHO_NDArrayWrapper< double, 4 > arr;
        CHECK_EQ("default_stride[0]", arr.GetStride(0), (std::size_t)0);
        CHECK_EQ("default_stride[1]", arr.GetStride(1), (std::size_t)0);
        CHECK_EQ("default_stride[2]", arr.GetStride(2), (std::size_t)0);
        CHECK_EQ("default_stride[3]", arr.GetStride(3), (std::size_t)0);
    }

    // --- Test 2: construction with dims sets correct row-major strides ---
    // dims = {2, 3, 4, 5}  =>  strides = {60, 20, 5, 1}
    {
        std::size_t dims[4] = {2, 3, 4, 5};
        MHO_NDArrayWrapper< double, 4 > arr(dims);
        CHECK_EQ("dims_stride[0]", arr.GetStride(0), (std::size_t)60);
        CHECK_EQ("dims_stride[1]", arr.GetStride(1), (std::size_t)20);
        CHECK_EQ("dims_stride[2]", arr.GetStride(2), (std::size_t)5);
        CHECK_EQ("dims_stride[3]", arr.GetStride(3), (std::size_t)1);
    }

    // --- Test 3: Resize recomputes all strides correctly ---
    {
        MHO_NDArrayWrapper< double, 3 > arr;
        arr.Resize(2, 5, 7);
        // strides = {35, 7, 1}
        CHECK_EQ("resize_stride[0]", arr.GetStride(0), (std::size_t)35);
        CHECK_EQ("resize_stride[1]", arr.GetStride(1), (std::size_t)7);
        CHECK_EQ("resize_stride[2]", arr.GetStride(2), (std::size_t)1);
    }

    return nfail == 0 ? 0 : 1;
}
