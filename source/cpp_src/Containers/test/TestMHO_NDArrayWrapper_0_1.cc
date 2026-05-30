#include <cmath>
#include <complex>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// REQUIRE_NEAR for real values maps to CHECK_CLOSE; complex values use REQUIRE_CLOSE_CPLX.
#define REQUIRE_NEAR(a, b, eps) CHECK_CLOSE(a, b, eps)
#define REQUIRE_NEAR_CPLX(a, b, eps) REQUIRE_CLOSE_CPLX(a, b, eps)

int main()
{
    //  Test Case 1: Rank-0 default construction + value ctor
    {
        // Default construction: rank==0, size==1
        MHO_NDArrayWrapper< double, 0 > a;
        REQUIRE(a.GetRank() == 0);
        REQUIRE(a.GetSize() == 1);

        // Value construction
        MHO_NDArrayWrapper< double, 0 > b(3.14);
        REQUIRE(b.GetRank() == 0);
        REQUIRE(b.GetSize() == 1);
        REQUIRE_NEAR(b.GetData(), 3.14, 1e-12);

        // Same for int
        MHO_NDArrayWrapper< int, 0 > c(42);
        REQUIRE(c.GetData() == 42);

        // Same for complex
        MHO_NDArrayWrapper< std::complex< double >, 0 > d(std::complex< double >(1.0, 2.0));
        REQUIRE_NEAR_CPLX(d.GetData(), std::complex< double >(1.0, 2.0), 1e-12);
    }

    //  Test Case 2: Rank-0 SetData/GetData round-trip
    {
        MHO_NDArrayWrapper< double, 0 > a;
        a.SetData(42.0);
        REQUIRE_NEAR(a.GetData(), 42.0, 1e-12);

        a.SetData(-7.5);
        REQUIRE_NEAR(a.GetData(), -7.5, 1e-12);

        MHO_NDArrayWrapper< int, 0 > b;
        b.SetData(99);
        REQUIRE(b.GetData() == 99);

        MHO_NDArrayWrapper< std::complex< double >, 0 > c;
        c.SetData(std::complex< double >(3.0, 4.0));
        REQUIRE_NEAR_CPLX(c.GetData(), std::complex< double >(3.0, 4.0), 1e-12);
    }

    //  Test Case 3: Rank-0 SetArray sets single value
    {
        MHO_NDArrayWrapper< double, 0 > a(0.0);
        a.SetArray(5.5);
        REQUIRE_NEAR(a.GetData(), 5.5, 1e-12);

        MHO_NDArrayWrapper< int, 0 > b(0);
        b.SetArray(17);
        REQUIRE(b.GetData() == 17);
    }

    //  Test Case 4: Rank-0 ZeroArray on POD types
    {
        MHO_NDArrayWrapper< double, 0 > a(9.0);
        a.ZeroArray();
        REQUIRE_NEAR(a.GetData(), 0.0, 1e-12);

        MHO_NDArrayWrapper< int, 0 > b(9);
        b.ZeroArray();
        REQUIRE(b.GetData() == 0);
    }

    //  Test Case 5: Rank-0 copy ctor + operator= produce independent values
    {
        MHO_NDArrayWrapper< double, 0 > a(3.14);
        MHO_NDArrayWrapper< double, 0 > b(a); // copy ctor
        REQUIRE_NEAR(b.GetData(), 3.14, 1e-12);

        b.SetData(999.0);
        REQUIRE_NEAR(a.GetData(), 3.14, 1e-12); // a unchanged

        MHO_NDArrayWrapper< double, 0 > c;
        c = a; // operator=
        REQUIRE_NEAR(c.GetData(), 3.14, 1e-12);

        c.SetData(111.0);
        REQUIRE_NEAR(a.GetData(), 3.14, 1e-12); // a still unchanged
    }

    //  Test Case 6: Complex rank-0 round-trip
    {
        std::complex< double > original(1.5, 2.5);
        MHO_NDArrayWrapper< std::complex< double >, 0 > a(original);
        REQUIRE_NEAR_CPLX(a.GetData(), original, 1e-12);

        a.SetData(std::complex< double >(3.0, 4.0));
        REQUIRE_NEAR_CPLX(a.GetData(), std::complex< double >(3.0, 4.0), 1e-12);

        MHO_NDArrayWrapper< std::complex< double >, 0 > b(a);
        REQUIRE_NEAR_CPLX(b.GetData(), std::complex< double >(3.0, 4.0), 1e-12);

        b.SetData(std::complex< double >(0, 0));
        REQUIRE_NEAR_CPLX(b.GetData(), std::complex< double >(0, 0), 1e-12);
        REQUIRE_NEAR_CPLX(a.GetData(), std::complex< double >(3.0, 4.0), 1e-12); // a unchanged
    }

    //  Test Case 7: Rank-1 default construction is empty
    {
        MHO_NDArrayWrapper< double, 1 > a;
        REQUIRE(a.GetRank() == 1);
        REQUIRE(a.GetSize() == 0);
        REQUIRE(a.GetStride(0) == 0);
    }

    //  Test Case 8: Rank-1 construction with dim sets size and stride==1
    {
        std::size_t N = 8;
        std::size_t dims = N;
        MHO_NDArrayWrapper< double, 1 > a(&dims);
        REQUIRE(a.GetRank() == 1);
        REQUIRE(a.GetSize() == 8);
        REQUIRE(a.GetStride(0) == 1);

        // Also test single-size constructor
        MHO_NDArrayWrapper< double, 1 > b(5);
        REQUIRE(b.GetSize() == 5);
        REQUIRE(b.GetStride(0) == 1);

        // int type
        MHO_NDArrayWrapper< int, 1 > c(8);
        REQUIRE(c.GetSize() == 8);
        REQUIRE(c.GetStride(0) == 1);
    }

    //  Test Case 9: Rank-1 operator() and operator[] return same element
    {
        std::size_t N = 8;
        MHO_NDArrayWrapper< double, 1 > a(N);
        for(std::size_t i = 0; i < N; i++)
        {
            a(i) = static_cast< double >(i) * 0.25;
        }
        for(std::size_t i = 0; i < N; i++)
        {
            REQUIRE_NEAR(a(i), a[i], 1e-12);
            REQUIRE_NEAR(a(i), static_cast< double >(i) * 0.25, 1e-12);
        }
    }

    //  Test Case 10: Rank-1 at() bounds-checks (throws std::out_of_range)
    {
        MHO_NDArrayWrapper< double, 1 > a(8);
        for(std::size_t i = 0; i < 8; i++)
            a(i) = static_cast< double >(i) * 0.25;

        // In-bounds access should not throw
        bool threw = false;
        try
        {
            volatile double dummy = a.at(7);
            (void)dummy;
        }
        catch(...)
        {
            threw = true;
        }
        REQUIRE(!threw);

        // Out-of-bounds access should throw
        threw = true;
        try
        {
            volatile double dummy = a.at(8);
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

        // Another out-of-bounds
        threw = true;
        try
        {
            volatile double dummy = a.at(100);
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
    }

    //  Test Case 11: Rank-1 construction from pointer copies data
    {
        std::size_t N = 8;
        double raw[8];
        for(std::size_t i = 0; i < N; i++)
            raw[i] = static_cast< double >(i) * 0.25;

        MHO_NDArrayWrapper< double, 1 > a(raw, N);
        REQUIRE(a.GetSize() == 8);
        for(std::size_t i = 0; i < N; i++)
        {
            REQUIRE_NEAR(a(i), raw[i], 1e-12);
        }

        // Mutating original should NOT affect the wrapper (copy-in semantics)
        raw[0] = 9999.0;
        REQUIRE_NEAR(a(0), 0.0, 1e-12);
    }

    //  Test Case 12: Rank-1 Resize wipes contents and re-sets stride
    {
        MHO_NDArrayWrapper< double, 1 > a(8);
        for(std::size_t i = 0; i < 8; i++)
            a(i) = static_cast< double >(i) * 0.25;

        // Resize to 4
        std::size_t newDim = 4;
        a.Resize(&newDim);
        REQUIRE(a.GetSize() == 4);
        REQUIRE(a.GetStride(0) == 1);

        // Also test single-arg resize
        a.Resize(10);
        REQUIRE(a.GetSize() == 10);
        REQUIRE(a.GetStride(0) == 1);
    }

    //  Test Case 13: Rank-1 copy ctor + operator= produce independent storage
    {
        std::size_t N = 8;
        MHO_NDArrayWrapper< double, 1 > a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = static_cast< double >(i) * 0.25;

        // Copy ctor
        MHO_NDArrayWrapper< double, 1 > b(a);
        for(std::size_t i = 0; i < N; i++)
        {
            REQUIRE_NEAR(b(i), a(i), 1e-12);
        }

        b(0) = 9999.0;
        REQUIRE_NEAR(a(0), 0.0, 1e-12); // a unchanged

        // Operator=
        MHO_NDArrayWrapper< double, 1 > c(3);
        c = a;
        REQUIRE(c.GetSize() == 8);
        for(std::size_t i = 0; i < N; i++)
        {
            REQUIRE_NEAR(c(i), a(i), 1e-12);
        }

        c(0) = 111.0;
        REQUIRE_NEAR(a(0), 0.0, 1e-12); // a still unchanged
    }

    //  Test Case 14: Rank-1 SetArray and ZeroArray
    {
        std::size_t N = 8;
        MHO_NDArrayWrapper< double, 1 > a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = static_cast< double >(i) * 0.25;

        a.SetArray(42.0);
        for(std::size_t i = 0; i < N; i++)
        {
            REQUIRE_NEAR(a(i), 42.0, 1e-12);
        }

        a.ZeroArray();
        for(std::size_t i = 0; i < N; i++)
        {
            REQUIRE_NEAR(a(i), 0.0, 1e-12);
        }

        // Same for int
        MHO_NDArrayWrapper< int, 1 > b(N);
        b.SetArray(7);
        for(std::size_t i = 0; i < N; i++)
        {
            REQUIRE(b(i) == 7);
        }
        b.ZeroArray();
        for(std::size_t i = 0; i < N; i++)
        {
            REQUIRE(b(i) == 0);
        }
    }

    //  Test Case 15: Rank-1 scalar compound assignment (v += 1; v *= 2)
    {
        std::size_t N = 8;
        MHO_NDArrayWrapper< double, 1 > a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = static_cast< double >(i) * 0.25;

        // Save originals
        std::vector< double > orig(N);
        for(std::size_t i = 0; i < N; i++)
            orig[i] = a(i);

        a += 1.0;
        a *= 2.0;

        for(std::size_t i = 0; i < N; i++)
        {
            double expected = 2.0 * (orig[i] + 1.0);
            REQUIRE_NEAR(a(i), expected, 1e-12);
        }
    }

    //  Test Case 16: Rank-1 pointwise compound assignment (v += w, same shape)
    {
        std::size_t N = 8;
        MHO_NDArrayWrapper< double, 1 > a(N);
        MHO_NDArrayWrapper< double, 1 > w(N);

        for(std::size_t i = 0; i < N; i++)
        {
            a(i) = static_cast< double >(i) * 0.25;
            w(i) = static_cast< double >(i) * 0.5;
        }

        // Save originals
        std::vector< double > orig(N);
        for(std::size_t i = 0; i < N; i++)
            orig[i] = a(i);

        a += w;

        for(std::size_t i = 0; i < N; i++)
        {
            double expected = orig[i] + w(i);
            REQUIRE_NEAR(a(i), expected, 1e-12);
        }
    }

    //  Test Case 17: Rank-1 pointwise op throws on size mismatch
    {
        MHO_NDArrayWrapper< double, 1 > a(8);
        MHO_NDArrayWrapper< double, 1 > b(4);

        bool threw = false;
        try
        {
            a += b;
        }
        catch(const std::out_of_range&)
        {
            threw = true;
        }
        catch(...)
        {}
        REQUIRE(threw);

        // Also test *=
        threw = false;
        try
        {
            a *= b;
        }
        catch(const std::out_of_range&)
        {
            threw = true;
        }
        catch(...)
        {}
        REQUIRE(threw);
    }

    //  Test Case 18: Rank-1 GetOffsetForIndices / GetIndicesForOffset
    {
        std::size_t N = 8;
        MHO_NDArrayWrapper< double, 1 > a(N);

        for(std::size_t i = 0; i < N; i++)
        {
            std::size_t offset = a.GetOffsetForIndices(&i);
            REQUIRE(offset == i);

            auto indices = a.GetIndicesForOffset(offset);
            REQUIRE(indices[0] == i);
        }
    }

    //  Test Case 19: Rank-1 iterators count==8, sum matches expected
    {
        std::size_t N = 8;
        MHO_NDArrayWrapper< double, 1 > a(N);
        for(std::size_t i = 0; i < N; i++)
            a(i) = static_cast< double >(i) * 0.25;

        // Expected sum: 0*0.25 + 1*0.25 + ... + 7*0.25 = 0.25 * (0+1+..+7) = 0.25 * 28 = 7.0
        double expected_sum = 7.0;
        double actual_sum = 0.0;
        std::size_t count = 0;

        for(auto it = a.begin(); it != a.end(); ++it)
        {
            actual_sum += *it;
            count++;
        }

        REQUIRE(count == 8);
        REQUIRE_NEAR(actual_sum, expected_sum, 1e-12);

        // Also test const_iterator
        count = 0;
        actual_sum = 0.0;
        for(auto it = a.cbegin(); it != a.cend(); ++it)
        {
            actual_sum += *it;
            count++;
        }
        REQUIRE(count == 8);
        REQUIRE_NEAR(actual_sum, expected_sum, 1e-12);
    }

    //  Test Case 20: Complex rank-1 round-trip
    {
        std::size_t N = 8;
        MHO_NDArrayWrapper< std::complex< double >, 1 > a(N);

        for(std::size_t i = 0; i < N; i++)
            a(i) = std::complex< double >(static_cast< double >(i), 2.0 * static_cast< double >(i));

        for(std::size_t i = 0; i < N; i++)
        {
            std::complex< double > expected(static_cast< double >(i), 2.0 * static_cast< double >(i));
            REQUIRE_NEAR_CPLX(a(i), expected, 1e-12);
        }

        // Compound assignment with complex scalar
        a *= std::complex< double >(0, 1);
        for(std::size_t i = 0; i < N; i++)
        {
            std::complex< double > original(static_cast< double >(i), 2.0 * static_cast< double >(i));
            std::complex< double > expected = std::complex< double >(0, 1) * original;
            REQUIRE_NEAR_CPLX(a(i), expected, 1e-12);
        }

        // Pointwise add with another complex array
        MHO_NDArrayWrapper< std::complex< double >, 1 > b(N);
        for(std::size_t i = 0; i < N; i++)
            b(i) = std::complex< double >(0.5, 1.0);

        a += b;
        for(std::size_t i = 0; i < N; i++)
        {
            std::complex< double > original(static_cast< double >(i), 2.0 * static_cast< double >(i));
            std::complex< double > expected = std::complex< double >(0, 1) * original + std::complex< double >(0.5, 1.0);
            REQUIRE_NEAR_CPLX(a(i), expected, 1e-12);
        }
    }

    return 0;
}
