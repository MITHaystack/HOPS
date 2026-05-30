#include <complex>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "MHO_Message.hh"
#include "MHO_NDArrayView.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// REQUIRE_NEAR for real values maps to CHECK_CLOSE; complex values use REQUIRE_CLOSE_CPLX.
#define REQUIRE_NEAR(a, b, eps) CHECK_CLOSE(a, b, eps)
#define REQUIRE_NEAR_CPLX(a, b, eps) REQUIRE_CLOSE_CPLX(a, b, eps)

int main()
{
    //  Test Case 1: Construction from raw ptr + dims + strides
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        std::size_t d[2] = {6, 8};
        std::size_t s[2] = {8, 1};
        MHO_NDArrayView< double, 2 > view(A.GetData(), d, s);

        REQUIRE(view.GetRank() == 2);
        REQUIRE(view.GetSize() == 48);
        REQUIRE(view.GetDimension(0) == 6);
        REQUIRE(view.GetDimension(1) == 8);
        REQUIRE(view.GetStride(0) == 8);
        REQUIRE(view.GetStride(1) == 1);
    }

    //  Test Case 2: Construction with nullptr arguments is empty
    {
        MHO_Message::GetInstance().SetMessageLevel(eSilent);
        MHO_NDArrayView< double, 2 > v(nullptr, nullptr, nullptr);
        MHO_Message::GetInstance().SetMessageLevel(eDebug);

        REQUIRE(v.GetSize() == 0);
        REQUIRE(v.GetDimension(0) == 0);
        REQUIRE(v.GetStride(0) == 0);
    }

    //  Test Case 3: Element access matches parent
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        MHO_NDArrayView< double, 2 > view = A.SubView(2);

        for(std::size_t j = 0; j < 6; j++)
        {
            for(std::size_t k = 0; k < 8; k++)
            {
                double expected = 100.0 * 2 + 10.0 * j + 1.0 * k;
                REQUIRE_NEAR(view(j, k), A(2, j, k), 1e-10);
                REQUIRE_NEAR(view(j, k), expected, 1e-10);
            }
        }
    }

    //  Test Case 4: at() bounds-checks
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        MHO_NDArrayView< double, 2 > view = A.SubView(0);

        // at() should throw for out-of-bounds
        bool threw = false;
        try
        {
            volatile double dummy = view.at(99, 0);
            (void)dummy;
        }
        catch(const std::out_of_range&)
        {
            threw = true;
        }
        catch(...)
        {}
        REQUIRE(threw);

        // operator() does NOT throw for out-of-bounds (no bounds check)
        threw = false;
        try
        {
            volatile double dummy = view(99, 0);
            (void)dummy;
        }
        catch(...)
        {
            threw = true;
        }
        REQUIRE(!threw);
    }

    //  Test Case 5: Write-through to parent
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        MHO_NDArrayView< double, 2 > view = A.SubView(2);
        view(1, 2) = -7.5;

        REQUIRE_NEAR(A(2, 1, 2), -7.5, 1e-10);
    }

    //  Test Case 6: SliceView returns correct dims/strides
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        MHO_NDArrayView< double, 2 > sl = A.SliceView(":", 3, ":");

        REQUIRE(sl.GetRank() == 2);
        REQUIRE(sl.GetDimension(0) == 4);
        REQUIRE(sl.GetDimension(1) == 8);
        REQUIRE(sl.GetStride(0) == 48);
        REQUIRE(sl.GetStride(1) == 1);

        // verify write-through
        for(std::size_t i = 0; i < 4; i++)
        {
            for(std::size_t k = 0; k < 8; k++)
            {
                double expected = 100.0 * i + 10.0 * 3 + 1.0 * k;
                REQUIRE_NEAR(sl(i, k), A(i, 3, k), 1e-10);
                REQUIRE_NEAR(sl(i, k), expected, 1e-10);
            }
        }

        // write through view, check parent changed
        sl(0, 0) = 999.0;
        REQUIRE_NEAR(A(0, 3, 0), 999.0, 1e-10);
    }

    //  Test Case 7: View-to-view Copy on same shape
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        MHO_NDArrayView< double, 2 > v1 = A.SubView(0);
        MHO_NDArrayView< double, 2 > v2 = A.SubView(1);

        v2.Copy(v1);

        for(std::size_t j = 0; j < 6; j++)
        {
            for(std::size_t k = 0; k < 8; k++)
            {
                REQUIRE_NEAR(A(1, j, k), A(0, j, k), 1e-10);
            }
        }
    }

    //  Test Case 8: Copy on shape mismatch - unchanged, logs error
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        // v1 has shape {6, 8}
        MHO_NDArrayView< double, 2 > v1 = A.SubView(0);

        // Create a separate small array for v2 with shape {4, 4}
        std::size_t smallDims[2] = {4, 4};
        MHO_NDArrayWrapper< double, 2 > B(smallDims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 4; j++)
                B(i, j) = -100.0 * i - 100.0 * j;

        MHO_NDArrayView< double, 2 > v2(B.GetData(), smallDims, B.GetStrides());

        double saved = v2(0, 0);

        MHO_Message::GetInstance().SetMessageLevel(eSilent);
        v2.Copy(v1); // v1 is {6,8}, v2 is {4,4} - mismatch, should not modify v2
        MHO_Message::GetInstance().SetMessageLevel(eDebug);

        REQUIRE_NEAR(v2(0, 0), saved, 1e-10);
        // Also check another element
        REQUIRE_NEAR(v2(2, 3), B(2, 3), 1e-10);
    }

    //  Test Case 9: Scalar compound assignment
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        MHO_NDArrayView< double, 2 > sv = A.SubView(0);
        // Save originals
        std::vector< double > originals(48);
        for(std::size_t j = 0; j < 6; j++)
            for(std::size_t k = 0; k < 8; k++)
                originals[j * 8 + k] = A(0, j, k);

        sv += 1.0;
        sv *= 2.0;

        for(std::size_t j = 0; j < 6; j++)
        {
            for(std::size_t k = 0; k < 8; k++)
            {
                double expected = 2.0 * (originals[j * 8 + k] + 1.0);
                REQUIRE_NEAR(A(0, j, k), expected, 1e-10);
            }
        }
    }

    //  Test Case 10: Pointwise compound assignment, same-shape views
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        MHO_NDArrayView< double, 2 > v1 = A.SubView(0);
        MHO_NDArrayView< double, 2 > v2 = A.SubView(1);

        // snapshot v1 original values
        std::vector< double > snap(48);
        for(std::size_t j = 0; j < 6; j++)
            for(std::size_t k = 0; k < 8; k++)
                snap[j * 8 + k] = A(0, j, k);

        v1 += v2;

        for(std::size_t j = 0; j < 6; j++)
        {
            for(std::size_t k = 0; k < 8; k++)
            {
                double expected = snap[j * 8 + k] + A(1, j, k);
                REQUIRE_NEAR(A(0, j, k), expected, 1e-10);
            }
        }
    }

    //  Test Case 11: Pointwise op throws on size mismatch
    {
        std::size_t dims1[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims1);

        std::size_t dims2[2] = {4, 8};
        MHO_NDArrayWrapper< double, 2 > B(dims2);

        MHO_NDArrayView< double, 2 > v1 = A.SubView(0);                      // size 48
        MHO_NDArrayView< double, 2 > v2(B.GetData(), dims2, B.GetStrides()); // size 32

        bool threw = false;
        try
        {
            v1 += v2;
        }
        catch(const std::out_of_range&)
        {
            threw = true;
        }
        catch(...)
        {}
        REQUIRE(threw);
    }

    //  Test Case 12: Iterator traversal visits GetSize() elements
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        MHO_NDArrayView< double, 2 > view = A.SubView(0);

        std::size_t cnt = 0;
        for(auto it = view.begin(); it != view.end(); ++it)
        {
            (void)*it; // prevent unused-value warning
            cnt++;
        }
        REQUIRE(cnt == view.GetSize());
    }

    //  Test Case 13: Iterator dereference matches indexed access in row-major order
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        MHO_NDArrayView< double, 2 > view = A.SubView(0);

        auto it = view.begin();
        // First element is (0, 0)
        REQUIRE_NEAR(*it, view(0, 0), 1e-10);
        ++it;
        // Second element is (0, 1)
        REQUIRE_NEAR(*it, view(0, 1), 1e-10);
        ++it;
        ++it;
        // Fourth element is (0, 3)
        REQUIRE_NEAR(*it, view(0, 3), 1e-10);

        // Walk through all elements and verify row-major order
        it = view.begin();
        std::size_t j = 0, k = 0;
        for(std::size_t idx = 0; idx < view.GetSize(); idx++)
        {
            REQUIRE_NEAR(*it, view(j, k), 1e-10);
            ++it;
            k++;
            if(k >= view.GetDimension(1))
            {
                k = 0;
                j++;
            }
        }
    }

    //  Test Case 14: Complex element type
    {
        std::size_t dims[2] = {4, 4};
        MHO_NDArrayWrapper< std::complex< double >, 2 > C(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 4; j++)
                C(i, j) = std::complex< double >(static_cast< double >(i), static_cast< double >(j));

        MHO_NDArrayView< std::complex< double >, 1 > col = C.SliceView(":", 1);

        REQUIRE(col.GetRank() == 1);
        REQUIRE(col.GetDimension(0) == 4);

        // column 1: C(i, 1) = complex(i, 1)
        // multiply by i: i * complex(i, 1) = complex(-1, i)
        col *= std::complex< double >(0, 1);

        for(std::size_t i = 0; i < 4; i++)
        {
            std::complex< double > expected(-1.0, static_cast< double >(i));
            REQUIRE_NEAR_CPLX(C(i, 1), expected, 1e-10);
        }
    }

    //  Test Case 15: SetArray / ZeroArray
    {
        std::size_t dims[3] = {4, 6, 8};
        MHO_NDArrayWrapper< double, 3 > A(dims);
        for(std::size_t i = 0; i < 4; i++)
            for(std::size_t j = 0; j < 6; j++)
                for(std::size_t k = 0; k < 8; k++)
                    A(i, j, k) = 100.0 * i + 10.0 * j + 1.0 * k;

        MHO_NDArrayView< double, 2 > view = A.SubView(0);

        view.SetArray(0.0);

        for(std::size_t j = 0; j < 6; j++)
        {
            for(std::size_t k = 0; k < 8; k++)
            {
                REQUIRE_NEAR(A(0, j, k), 0.0, 1e-10);
            }
        }

        view.ZeroArray(); // should still be 0
        for(std::size_t j = 0; j < 6; j++)
        {
            for(std::size_t k = 0; k < 8; k++)
            {
                REQUIRE_NEAR(A(0, j, k), 0.0, 1e-10);
            }
        }
    }

    return 0;
}
