#include "MHO_FastFourierTransform.hh"
#include "MHO_Message.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"
#include "MHO_TestAssertions.hh"
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

using namespace hops;

//---- fixture helpers ----

template< typename T, std::size_t R >
MHO_NDArrayWrapper< std::complex< T >, R > make_random_nd(const std::size_t (&dims)[R], unsigned int seed)
{
    MHO_NDArrayWrapper< std::complex< T >, R > a(dims);
    std::mt19937 rng(seed);
    std::uniform_real_distribution< T > d(-1.0, 1.0);
    for(std::size_t i = 0; i < a.GetSize(); ++i)
        a[i] = std::complex< T >(d(rng), d(rng));
    return a;
}

template< typename T, std::size_t R > MHO_NDArrayWrapper< std::complex< T >, R > make_impulse_nd(const std::size_t (&dims)[R])
{
    MHO_NDArrayWrapper< std::complex< T >, R > a(dims);
    a.SetArray(std::complex< T >(0.0, 0.0));
    a[0] = std::complex< T >(1.0, 0.0);
    return a;
}

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    using Arr1 = MHO_NDArrayWrapper< std::complex< double >, 1 >;
    using Arr3 = MHO_NDArrayWrapper< std::complex< double >, 3 >;

    //----- Fixtures -----
    const std::size_t d3a[] = {8, 8, 8};
    Arr3 x3a = make_random_nd< double, 3 >(d3a, 42);

    const std::size_t d3b[] = {4, 8, 5};
    Arr3 x3b = make_random_nd< double, 3 >(d3b, 42);

    const std::size_t d3c[] = {8, 4, 4};
    Arr3 x3c = make_impulse_nd< double, 3 >(d3c);

    //============================================================
    //Test1: Round-trip on uniform 3-D (all radix-2)
    //Forward + backward FFT on {8,8,8}. IFFT(FFT(x)) = total_size * x
    //============================================================
    {
        Arr3 x = x3a;
        Arr3 x_orig = x;

        MHO_MultidimensionalFastFourierTransform< Arr3 > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        fft.SetBackward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();

        double scale = 8.0 * 8.0 * 8.0; // 512
        for(std::size_t i = 0; i < x.GetSize(); ++i)
            x[i] /= scale;

        double maxerr = 0.0;
        for(std::size_t i = 0; i < x.GetSize(); ++i)
        {
            double e = std::abs(x[i] - x_orig[i]);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-10);
    }

    //============================================================
    //Test2: Round-trip on non-uniform 3-D (radix-2 + Bluestein)
    //{4,8,5} -- the size-5 axis forces Bluestein.
    //============================================================
    {
        Arr3 x = x3b;
        Arr3 x_orig = x;

        MHO_MultidimensionalFastFourierTransform< Arr3 > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        fft.SetBackward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();

        double scale = 4.0 * 8.0 * 5.0; // 160
        for(std::size_t i = 0; i < x.GetSize(); ++i)
            x[i] /= scale;

        double maxerr = 0.0;
        for(std::size_t i = 0; i < x.GetSize(); ++i)
        {
            double e = std::abs(x[i] - x_orig[i]);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-9);
    }

    //============================================================
    //Test3: Impulse -> constant spectrum
    //Forward FFT of impulse; |out[i,j,k]| should equal 1.
    //============================================================
    {
        Arr3 x = x3c;

        MHO_MultidimensionalFastFourierTransform< Arr3 > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();

        double maxerr = 0.0;
        for(std::size_t i = 0; i < x.GetSize(); ++i)
        {
            double e = std::abs(std::abs(x[i]) - 1.0);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-12);
    }

    //============================================================
    //Test4: Per-axis selection (transform only axis 1)
    //Compare against 1-D FFT applied to each row independently.
    //============================================================
    {
        Arr3 x = x3a;

        MHO_MultidimensionalFastFourierTransform< Arr3 > fft;
        fft.DeselectAllAxes();
        fft.SelectAxis(1);
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();

        // Build reference: 1-D FFT on each row along axis 1
        Arr3 ref = x3a;
        const std::size_t rowD[1] = {8};
        for(std::size_t i0 = 0; i0 < 8; ++i0)
        {
            for(std::size_t i2 = 0; i2 < 8; ++i2)
            {
                Arr1 row(rowD);
                for(std::size_t j = 0; j < 8; ++j)
                    row[j] = ref(i0, j, i2);

                MHO_FastFourierTransform< double > fft1;
                fft1.SetForward();
                fft1.SetArgs(&row);
                fft1.Initialize();
                fft1.Execute();

                for(std::size_t j = 0; j < 8; ++j)
                    ref(i0, j, i2) = row[j];
            }
        }

        double maxerr = 0.0;
        for(std::size_t i = 0; i < x.GetSize(); ++i)
        {
            double e = std::abs(x[i] - ref[i]);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-12);
    }

    //============================================================
    //Test5: Separability across axes
    //Transform {0,1,2} all-at-once vs. sequentially on each axis.
    //============================================================
    {
        Arr3 xA = x3a;
        MHO_MultidimensionalFastFourierTransform< Arr3 > fftA;
        fftA.SetForward();
        fftA.SetArgs(&xA);
        fftA.Initialize();
        fftA.Execute();

        Arr3 xB = x3a;
        for(int ax = 0; ax < 3; ++ax)
        {
            MHO_MultidimensionalFastFourierTransform< Arr3 > fftB;
            fftB.DeselectAllAxes();
            fftB.SelectAxis(static_cast< std::size_t >(ax));
            fftB.SetForward();
            fftB.SetArgs(&xB);
            fftB.Initialize();
            fftB.Execute();
        }

        double maxerr = 0.0;
        for(std::size_t i = 0; i < xA.GetSize(); ++i)
        {
            double e = std::abs(xA[i] - xB[i]);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-12);
    }

    //============================================================
    //Test6: In-place vs out-of-place
    //============================================================
    {
        Arr3 x_ip = x3a;
        Arr3 x_oop_in = x3a;
        Arr3 x_oop_out(x3a.GetDimensions());

        MHO_MultidimensionalFastFourierTransform< Arr3 > fftIp;
        fftIp.SetForward();
        fftIp.SetArgs(&x_ip);
        fftIp.Initialize();
        fftIp.Execute();

        MHO_MultidimensionalFastFourierTransform< Arr3 > fftOop;
        fftOop.SetForward();
        fftOop.SetArgs(&x_oop_in, &x_oop_out);
        fftOop.Initialize();
        fftOop.Execute();

        double maxerr = 0.0;
        for(std::size_t i = 0; i < x_ip.GetSize(); ++i)
        {
            double e = std::abs(x_ip[i] - x_oop_out[i]);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-14);
    }

    //============================================================
    //Test7: Cross-check vs 1-D native FFT
    //============================================================
    {
        const std::size_t d1d[] = {64};
        Arr1 x1 = make_random_nd< double, 1 >(d1d, 42);
        Arr1 x_md = x1;

        MHO_MultidimensionalFastFourierTransform< Arr1 > fftMd;
        fftMd.SetForward();
        fftMd.SetArgs(&x_md);
        fftMd.Initialize();
        fftMd.Execute();

        MHO_FastFourierTransform< double > fft1;
        fft1.SetForward();
        fft1.SetArgs(&x1);
        fft1.Initialize();
        fft1.Execute();

        double maxerr = 0.0;
        for(std::size_t i = 0; i < x1.GetSize(); ++i)
        {
            double e = std::abs(x1[i] - x_md[i]);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-14);
    }

    //============================================================
    //Test8: Re-init on dimension change
    //============================================================
    {
        MHO_MultidimensionalFastFourierTransform< Arr3 > fft;

        // First: {8,8,8}
        Arr3 x8 = x3a;
        fft.SetForward();
        fft.SetArgs(&x8);
        REQUIRE(fft.Initialize() == true);
        fft.Execute();

        // Second: {4,8,5} with impulse input
        Arr3 xNew = make_impulse_nd< double, 3 >(d3b);
        fft.SetArgs(&xNew);
        REQUIRE(fft.Initialize() == true);
        fft.Execute();

        // Independent reference
        Arr3 xRef = make_impulse_nd< double, 3 >(d3b);
        MHO_MultidimensionalFastFourierTransform< Arr3 > fftRef;
        fftRef.SetForward();
        fftRef.SetArgs(&xRef);
        fftRef.Initialize();
        fftRef.Execute();

        double maxerr = 0.0;
        for(std::size_t i = 0; i < xNew.GetSize(); ++i)
        {
            double e = std::abs(xNew[i] - xRef[i]);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-12);
    }

    //============================================================
    //Test9: Axis-label transformation on MHO_TableContainer
    //============================================================
    {
        using AxisPackType = MHO_AxisPack< MHO_Axis< double >, MHO_Axis< double > >;
        using TC2 = MHO_TableContainer< std::complex< double >, AxisPackType >;

        const std::size_t d2[] = {8, 8};
        TC2 tc(d2);
        std::mt19937 rng(42);
        std::uniform_real_distribution< double > ud(-1.0, 1.0);
        for(std::size_t idx = 0; idx < tc.GetSize(); ++idx)
            tc[idx] = std::complex< double >(ud(rng), ud(rng));

        // Set uniform axis labels: axis[k] = k for both axes
        {
            MHO_Axis< double >& a0 = std::get< 0 >(*static_cast< AxisPackType* >(&tc));
            for(std::size_t k = 0; k < 8; ++k)
                a0[k] = (double)k;
        }
        {
            MHO_Axis< double >& a1 = std::get< 1 >(*static_cast< AxisPackType* >(&tc));
            for(std::size_t k = 0; k < 8; ++k)
                a1[k] = (double)k;
        }

        MHO_MultidimensionalFastFourierTransform< TC2 > fft;
        fft.SetForward();
        fft.EnableAxisLabelTransformation();
        fft.SetArgs(&tc);
        fft.Initialize();
        fft.Execute();

        // Verify axis labels for both axes
        // Expected after TransformAxis (N=8, delta=1):
        //   i < 4: i/8  =>  0, 1/8, 2/8, 3/8
        //   i >= 4: (i-8)/8  =>  -4/8, -3/8, -2/8, -1/8
        const double expectedLabel[8] = {0.0, 0.125, 0.25, 0.375, -0.5, -0.375, -0.25, -0.125};
        {
            MHO_Axis< double >& a0 = std::get< 0 >(*static_cast< AxisPackType* >(&tc));
            for(std::size_t k = 0; k < 8; ++k)
                REQUIRE(std::abs(a0[k] - expectedLabel[k]) < 1e-14);
        }
        {
            MHO_Axis< double >& a1 = std::get< 1 >(*static_cast< AxisPackType* >(&tc));
            for(std::size_t k = 0; k < 8; ++k)
                REQUIRE(std::abs(a1[k] - expectedLabel[k]) < 1e-14);
        }
    }

    //============================================================
    //Test10: Idempotent / re-Execute
    //Execute forward FFT twice, then backward FFT twice.
    //After scaling, result should match original.
    //============================================================
    {
        const std::size_t d10[] = {8, 8, 8};
        Arr3 x = make_random_nd< double, 3 >(d10, 99);
        Arr3 x_orig = x;

        MHO_MultidimensionalFastFourierTransform< Arr3 > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        REQUIRE(fft.Execute() == true);
        REQUIRE(fft.Execute() == true); // second execute

        fft.SetBackward();
        fft.Initialize();
        REQUIRE(fft.Execute() == true);
        REQUIRE(fft.Execute() == true); // second execute

        // Each backward scales by N=8 per axis (total 512)
        // Two backward passes: scale = 512 * 512 = 262144
        double scale = (8.0 * 8.0 * 8.0) * (8.0 * 8.0 * 8.0);
        for(std::size_t i = 0; i < x.GetSize(); ++i)
            x[i] /= scale;

        double maxerr = 0.0;
        for(std::size_t i = 0; i < x.GetSize(); ++i)
        {
            double e = std::abs(x[i] - x_orig[i]);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-9);
    }

    //============================================================
    //Test11: nullptr safety
    //============================================================
    {
        MHO_MultidimensionalFastFourierTransform< Arr3 > fft;
        fft.SetArgs(static_cast< Arr3* >(nullptr));
        REQUIRE(fft.Initialize() == false);
        REQUIRE(fft.Execute() == false);
    }

    return 0;
}
