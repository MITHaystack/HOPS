#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"
#include <array>
#include <complex>
#include <stdexcept>
#include <vector>

using namespace hops;

// REQUIRE_NEAR for real values maps to the shared CHECK_CLOSE assertion.
#define REQUIRE_NEAR(a, b, eps) CHECK_CLOSE(a, b, eps)

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    //  Fixtures
    // A: MHO_NDArrayWrapper<double, 3> dims {2,3,4}, A(i,j,k) = 100*i + 10*j + k
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    A(i, j, k) = static_cast< double >(100 * i + 10 * j + k);
    }

    //  Test Case 1: Construction and rank/size
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    A(i, j, k) = static_cast< double >(100 * i + 10 * j + k);

        REQUIRE(A.GetRank() == 3);
        REQUIRE(A.GetSize() == 24);
        REQUIRE(A.GetDimension(0) == 2);
        REQUIRE(A.GetDimension(1) == 3);
        REQUIRE(A.GetDimension(2) == 4);

        std::size_t dims5[5] = {3, 4, 5, 6, 7};
        MHO_NDArrayWrapper< int, 5 > B(dims5);
        REQUIRE(B.GetRank() == 5);
    }

    //  Test Case 2: Row-major strides
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        // strides {12, 4, 1}
        REQUIRE(A.GetStride(0) == 12);
        REQUIRE(A.GetStride(1) == 4);
        REQUIRE(A.GetStride(2) == 1);

        std::size_t dims5[5] = {3, 4, 5, 6, 7};
        MHO_NDArrayWrapper< int, 5 > B(dims5);
        // strides {840, 210, 42, 7, 1}
        REQUIRE(B.GetStride(0) == 840);
        REQUIRE(B.GetStride(1) == 210);
        REQUIRE(B.GetStride(2) == 42);
        REQUIRE(B.GetStride(3) == 7);
        REQUIRE(B.GetStride(4) == 1);
    }

    //  Test Case 3: at() bounds-check throws
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);

        bool threw = false;
        try
        {
            volatile double dummy = A.at(2, 0, 0);
            (void)dummy;
        }
        catch(const std::out_of_range&)
        {
            threw = true;
        }
        catch(...)
        {
            threw = false;
        }
        REQUIRE(threw);

        // In-bounds should not throw
        threw = false;
        try
        {
            volatile double dummy = A.at(1, 2, 3);
            (void)dummy;
        }
        catch(...)
        {
            threw = true;
        }
        REQUIRE(!threw);
    }

    //  Test Case 4: operator[] linear index
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    A(i, j, k) = static_cast< double >(100 * i + 10 * j + k);

        // {0,0,0} => offset 0
        {
            std::size_t idx[3] = {0, 0, 0};
            std::size_t offset = A.GetOffsetForIndices(idx);
            REQUIRE_NEAR(A[offset], A(0, 0, 0), 1e-12);
        }
        // {1,2,3} => offset 23
        {
            std::size_t idx[3] = {1, 2, 3};
            std::size_t offset = A.GetOffsetForIndices(idx);
            REQUIRE_NEAR(A[offset], A(1, 2, 3), 1e-12);
        }
        // {0,1,2} => offset 6
        {
            std::size_t idx[3] = {0, 1, 2};
            std::size_t offset = A.GetOffsetForIndices(idx);
            REQUIRE_NEAR(A[offset], A(0, 1, 2), 1e-12);
        }
    }

    //  Test Case 5: GetIndicesForOffset / GetOffsetForIndices round-trip
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);

        for(std::size_t offset = 0; offset < A.GetSize(); offset++)
        {
            auto indices = A.GetIndicesForOffset(offset);
            std::size_t roundtrip = A.GetOffsetForIndices(&(indices[0]));
            REQUIRE(roundtrip == offset);
        }
    }

    //  Test Case 6: Deep copy constructor
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    A(i, j, k) = static_cast< double >(100 * i + 10 * j + k);

        MHO_NDArrayWrapper< double, 3 > A2(A);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    REQUIRE_NEAR(A2(i, j, k), A(i, j, k), 1e-12);

        A2(0, 0, 0) = 999.0;
        REQUIRE_NEAR(A(0, 0, 0), 0.0, 1e-12); // A unchanged
    }

    //  Test Case 7: Assignment operator
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    A(i, j, k) = static_cast< double >(100 * i + 10 * j + k);

        MHO_NDArrayWrapper< double, 3 > D;
        D = A;
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    REQUIRE_NEAR(D(i, j, k), A(i, j, k), 1e-12);

        D(0, 0, 0) = 999.0;
        REQUIRE_NEAR(A(0, 0, 0), 0.0, 1e-12); // A unchanged
    }

    //  Test Case 8: Clone() returns deep copy on heap
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    A(i, j, k) = static_cast< double >(100 * i + 10 * j + k);

        MHO_NDArrayWrapper< double, 3 >* p = A.Clone();
        REQUIRE(p->GetRank() == 3);
        REQUIRE(p->GetSize() == 24);
        REQUIRE_NEAR(p->at(0, 0, 0), 0.0, 1e-12);
        p->at(0, 0, 0) = 999.0;
        REQUIRE_NEAR(A(0, 0, 0), 0.0, 1e-12); // A unchanged
        delete p;
    }

    //  Test Case 9: SubView write-through
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    A(i, j, k) = static_cast< double >(100 * i + 10 * j + k);

        // SubView(1) fixes axis 0 to index 1, returns 2-D view of dims {3,4}
        MHO_NDArrayView< double, 2 > sv = A.SubView(1);
        sv(2, 3) = 999.0;
        REQUIRE_NEAR(A(1, 2, 3), 999.0, 1e-12);

        // Restore A(1,2,3) = 123.0 for later test cases
        A(1, 2, 3) = 123.0;
        REQUIRE_NEAR(A(1, 2, 3), 123.0, 1e-12);
    }

    //  Test Case 10: SliceView write-through
    {
        std::size_t dims5[5] = {3, 4, 5, 6, 7};
        MHO_NDArrayWrapper< int, 5 > B(dims5);

        // Fill B with known values: B(i,j,k,l,m) = 10000*i + 1000*j + 100*k + 10*l + m
        for(std::size_t i = 0; i < 3; i++)
            for(std::size_t j = 0; j < 4; j++)
                for(std::size_t k = 0; k < 5; k++)
                    for(std::size_t l = 0; l < 6; l++)
                        for(std::size_t m = 0; m < 7; m++)
                            B(i, j, k, l, m) = static_cast< int >(10000 * i + 1000 * j + 100 * k + 10 * l + m);

        // Save values with j!=1 for later comparison
        std::vector< int > saved_others(3 * 4 * 5 * 6 * 7);
        for(std::size_t idx = 0; idx < B.GetSize(); idx++)
            saved_others[idx] = B[idx];

        // SliceView(":", 1, ":", ":", ":") fixes axis 1 to index 1
        MHO_NDArrayView< int, 4 > sl = B.SliceView(":", 1, ":", ":", ":");

        // SetArray(7) on the slice view
        sl.SetArray(7);

        // Verify B(i,1,k,l,m) == 7 for all valid i,k,l,m
        for(std::size_t i = 0; i < 3; i++)
            for(std::size_t k = 0; k < 5; k++)
                for(std::size_t l = 0; l < 6; l++)
                    for(std::size_t m = 0; m < 7; m++)
                        REQUIRE(B(i, 1, k, l, m) == 7);

        // Verify B with j!=1 is unchanged
        for(std::size_t i = 0; i < 3; i++)
            for(std::size_t j = 0; j < 4; j++)
                for(std::size_t k = 0; k < 5; k++)
                    for(std::size_t l = 0; l < 6; l++)
                        for(std::size_t m = 0; m < 7; m++)
                        {
                            if(j != 1)
                            {
                                int expected = static_cast< int >(10000 * i + 1000 * j + 100 * k + 10 * l + m);
                                REQUIRE(B(i, j, k, l, m) == expected);
                            }
                        }
    }

    //  Test Case 11: Scalar compound assignment (int)
    {
        std::size_t dims[2] = {3, 3};
        MHO_NDArrayWrapper< int, 2 > X(dims);
        X.SetArray(2);

        X += 3; // every element becomes 5
        X *= 2; // every element becomes 10

        for(std::size_t i = 0; i < 3; i++)
            for(std::size_t j = 0; j < 3; j++)
                REQUIRE(X(i, j) == 10);
    }

    //  Test Case 12: Scalar compound assignment (complex)
    {
        std::size_t dims[2] = {4, 4};
        MHO_NDArrayWrapper< std::complex< double >, 2 > C(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 4; j++)
                C(i, j) = std::complex< double >(static_cast< double >(i), -static_cast< double >(j));

        C += std::complex< double >(1, 0); // C(i,j) = complex(i+1, -j)
        C *= 2.0;                          // C(i,j) = 2.0 * complex(i+1, -j)

        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 4; j++)
            {
                std::complex< double > expected =
                    2.0 * (std::complex< double >(static_cast< double >(i), -static_cast< double >(j)) + 1.0);
                REQUIRE_NEAR(std::abs(C(i, j) - expected), 0.0, 1e-12);
            }
    }

    //  Test Case 13: Pointwise ops
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    A(i, j, k) = static_cast< double >(100 * i + 10 * j + k);

        MHO_NDArrayWrapper< double, 3 > A2(A);
        A2 *= A; // element-wise square
        A2 -= A; // element-wise subtract

        // After A2 *= A: A2(1,2,3) should be 123.0 * 123.0 = 15129.0
        {
            MHO_NDArrayWrapper< double, 3 > A_check(dims);
            for(std::size_t i = 0; i < 2; i++)
                for(std::size_t j = 0; j < 3; j++)
                    for(std::size_t k = 0; k < 4; k++)
                        A_check(i, j, k) = static_cast< double >(100 * i + 10 * j + k);
            MHO_NDArrayWrapper< double, 3 > A2_check(A_check);
            A2_check *= A_check;
            REQUIRE_NEAR(A2_check(1, 2, 3), 15129.0, 1e-12);

            // After A2 -= A: A2(1,2,3) = 15129.0 - 123.0 = 15006.0
            A2_check -= A_check;
            REQUIRE_NEAR(A2_check(1, 2, 3), 15006.0, 1e-12);
        }
    }

    //  Test Case 14: Size mismatch throws
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);

        std::size_t dimsD[3] = {2, 2, 2};
        MHO_NDArrayWrapper< double, 3 > D(dimsD);

        bool threw = false;
        try
        {
            A *= D;
        }
        catch(const std::out_of_range&)
        {
            threw = true;
        }
        catch(...)
        {
            threw = false;
        }
        REQUIRE(threw);
    }

    //  Test Case 15: ZeroArray
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    A(i, j, k) = static_cast< double >(100 * i + 10 * j + k);

        A.ZeroArray();
        for(std::size_t i = 0; i < 2; i++)
            for(std::size_t j = 0; j < 3; j++)
                for(std::size_t k = 0; k < 4; k++)
                    REQUIRE_NEAR(A(i, j, k), 0.0, 1e-12);
    }

    //  Test Case 16: Free templates
    {
        std::size_t dimsA[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dimsA);

        std::size_t dimsB[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > B(dimsB);

        // Same rank
        REQUIRE(HaveSameRank(&A, &B));

        // Same number of elements (2*3*4 == 24)
        REQUIRE(HaveSameNumberOfElements(&A, &B));

        // Same dimensions
        REQUIRE(HaveSameDimensions(&A, &B));

        // Permuted shape: same count, different dimensions
        std::size_t dimsP[3] = {4, 3, 2};
        MHO_NDArrayWrapper< double, 3 > P(dimsP);

        REQUIRE(HaveSameRank(&A, &P));
        REQUIRE(HaveSameNumberOfElements(&A, &P)); // both 24
        REQUIRE(!HaveSameDimensions(&A, &P));      // {2,3,4} != {4,3,2}

        // Different rank
        std::size_t dimsR2[2] = {4, 6};
        MHO_NDArrayWrapper< double, 2 > R2(dimsR2);

        REQUIRE(!HaveSameRank(&A, &R2));
        REQUIRE(HaveSameNumberOfElements(&A, &R2)); // both 24
        REQUIRE(!HaveSameDimensions(&A, &R2));      // different rank => false
    }

    //  Test Case 17: Iterators
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);

        std::size_t count = 0;
        for(auto it = A.begin(); it != A.end(); ++it)
        {
            (void)*it;
            count++;
        }
        REQUIRE(count == A.GetSize());
    }

    //  Test Case 18: Strided iterator
    {
        std::size_t dims[3] = {2, 3, 4};
        MHO_NDArrayWrapper< double, 3 > A(dims);

        std::size_t count = 0;
        for(auto it = A.stride_begin(4); it != A.stride_end(4); ++it)
        {
            (void)*it;
            count++;
        }
        REQUIRE(count == A.GetSize() / 4); // 24/4 = 6

        // First dereference equals A[0]
        auto it = A.stride_begin(4);
        REQUIRE(&(*it) == &(A[0]));
    }

    return 0;
}
