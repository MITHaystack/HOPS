#include "MHO_FastFourierTransform.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

using namespace hops;

template< typename T > static MHO_NDArrayWrapper< std::complex< T >, 1 > make_random_arr(std::size_t n, unsigned int seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution< T > d(-1.0, 1.0);
    MHO_NDArrayWrapper< std::complex< T >, 1 > a(n);
    for(std::size_t i = 0; i < n; ++i)
        a(i) = std::complex< T >(d(rng), d(rng));
    return a;
}

template< typename T > static MHO_NDArrayWrapper< std::complex< T >, 1 > make_impulse(std::size_t n)
{
    MHO_NDArrayWrapper< std::complex< T >, 1 > a(n);
    a.SetArray(std::complex< T >(0.0, 0.0));
    a(0) = std::complex< T >(1.0, 0.0);
    return a;
}

template< typename T > static MHO_NDArrayWrapper< std::complex< T >, 1 > make_tone(std::size_t n, std::size_t k0)
{
    MHO_NDArrayWrapper< std::complex< T >, 1 > a(n);
    for(std::size_t i = 0; i < n; ++i)
        a(i) = std::polar(T(1.0), T(2.0 * M_PI) * k0 * i / n);
    return a;
}

template< typename T > static MHO_NDArrayWrapper< std::complex< T >, 1 > make_constant(std::size_t n, T c)
{
    MHO_NDArrayWrapper< std::complex< T >, 1 > a(n);
    a.SetArray(std::complex< T >(c, 0.0));
    return a;
}

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    using ArrD = MHO_NDArrayWrapper< std::complex< double >, 1 >;
    using ArrF = MHO_NDArrayWrapper< std::complex< float >, 1 >;

    // --- Test 1: Lifecycle / nullptr ---
    {
        MHO_FastFourierTransform< double > fft;
        fft.SetArgs(static_cast< ArrD* >(nullptr));
        bool i = fft.Initialize();
        bool e = fft.Execute();
        REQUIRE(i == false);
        REQUIRE(e == false);
    }

    // --- Test 2: Radix-2 round-trip (N=64) ---
    {
        ArrD x = make_random_arr< double >(64, 42);
        ArrD x_orig = x; // copy
        MHO_FastFourierTransform< double > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        fft.SetBackward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        // IFFT(FFT(x)) == N*x, so divide by N
        for(std::size_t i = 0; i < 64; ++i)
            x(i) /= 64.0;
        double maxerr = 0.0;
        for(std::size_t i = 0; i < 64; ++i)
        {
            double e = std::abs(x(i) - x_orig(i));
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-10);
    }

    // --- Test 3: Bluestein round-trip (N=21) ---
    {
        ArrD x = make_random_arr< double >(21, 42);
        ArrD x_orig = x;
        MHO_FastFourierTransform< double > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        fft.SetBackward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        for(std::size_t i = 0; i < 21; ++i)
            x(i) /= 21.0;
        double maxerr = 0.0;
        for(std::size_t i = 0; i < 21; ++i)
        {
            double e = std::abs(x(i) - x_orig(i));
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-9);
    }

    // --- Test 4: Impulse transform -> constant spectrum ---
    {
        ArrD x = make_impulse< double >(32);
        MHO_FastFourierTransform< double > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        double maxerr = 0.0;
        for(std::size_t k = 0; k < 32; ++k)
        {
            double e = std::abs(std::abs(x(k)) - 1.0);
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-12);
    }

    // --- Test 5: Single tone (k0=3, N=16) ---
    {
        ArrD x = make_tone< double >(16, 3);
        MHO_FastFourierTransform< double > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        // Find argmax
        double maxMag = 0.0;
        std::size_t kMax = 0;
        for(std::size_t k = 0; k < 16; ++k)
        {
            double m = std::abs(x(k));
            if(m > maxMag)
            {
                maxMag = m;
                kMax = k;
            }
        }
        REQUIRE(kMax == 3);
        REQUIRE(maxMag > 14.0); // > 0.9*N = 14.4

        // Sum of |X[k]|^2 for k != k0 should be tiny
        double leak = 0.0;
        for(std::size_t k = 0; k < 16; ++k)
            if(k != 3)
                leak += std::abs(x(k)) * std::abs(x(k));
        REQUIRE(leak < 1e-12);
    }

    // --- Test 5b: Single tone Bluestein (k0=5, N=21) ---
    {
        ArrD x = make_tone< double >(21, 5);
        MHO_FastFourierTransform< double > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        // Find argmax
        double maxMag = 0.0;
        std::size_t kMax = 0;
        for(std::size_t k = 0; k < 21; ++k)
        {
            double m = std::abs(x(k));
            if(m > maxMag)
            {
                maxMag = m;
                kMax = k;
            }
        }
        REQUIRE(kMax == 5);
        REQUIRE(maxMag > 0.9 * 21.0); // > 0.9*N = 18.9

        // Sum of |X[k]|^2 for k != k0 should be tiny (looser tolerance for Bluestein)
        double leak = 0.0;
        for(std::size_t k = 0; k < 21; ++k)
            if(k != 5)
                leak += std::abs(x(k)) * std::abs(x(k));
        REQUIRE(leak < 1e-9);
    }

    // --- Test 6: Parseval energy conservation ---
    {
        ArrD x = make_random_arr< double >(64, 42);
        ArrD y = x;
        MHO_FastFourierTransform< double > fft;
        fft.SetForward();
        fft.SetArgs(&y);
        fft.Initialize();
        fft.Execute();
        // Unnormalized DFT: sum |X[k]|^2 == N * sum |x[n]|^2
        double timeEnergy = 0.0, freqEnergy = 0.0;
        for(std::size_t i = 0; i < 64; ++i)
        {
            timeEnergy += std::abs(x(i)) * std::abs(x(i));
            freqEnergy += std::abs(y(i)) * std::abs(y(i));
        }
        // freqEnergy / N should equal timeEnergy
        double relDiff = std::abs(freqEnergy / 64.0 - timeEnergy) / timeEnergy;
        REQUIRE(relDiff < 1e-10);
    }

    // --- Test 7: In-place vs out-of-place equivalence ---
    {
        ArrD x = make_random_arr< double >(32, 42);
        ArrD x_ip = x;
        ArrD x_oop_in = x;
        ArrD x_oop_out(32); // must match input size for InitializeOutOfPlace

        MHO_FastFourierTransform< double > fftIp;
        fftIp.SetForward();
        fftIp.SetArgs(&x_ip);
        fftIp.Initialize();
        fftIp.Execute();

        MHO_FastFourierTransform< double > fftOop;
        fftOop.SetForward();
        fftOop.SetArgs(&x_oop_in, &x_oop_out);
        bool iOop = fftOop.Initialize();
        REQUIRE(iOop == true);
        fftOop.Execute();

        double maxerr = 0.0;
        for(std::size_t i = 0; i < 32; ++i)
        {
            double e = std::abs(x_ip(i) - x_oop_out(i));
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-14);
    }

    // --- Test 8: Float specialization sanity ---
    {
        ArrF x = make_random_arr< float >(64, 42);
        ArrF x_orig = x;
        MHO_FastFourierTransform< float > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        fft.SetBackward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        for(std::size_t i = 0; i < 64; ++i)
            x(i) /= 64.0f;
        float maxerr = 0.0f;
        for(std::size_t i = 0; i < 64; ++i)
        {
            float e = std::abs(x(i) - x_orig(i));
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-4f);
    }

    // --- Test 9: Re-init on size change ---
    {
        MHO_FastFourierTransform< double > fft;
        ArrD x32 = make_impulse< double >(32);
        fft.SetForward();
        fft.SetArgs(&x32);
        bool i1 = fft.Initialize();
        REQUIRE(i1 == true);
        bool e1 = fft.Execute();
        REQUIRE(e1 == true);

        // Now switch to N=64
        ArrD x64 = make_impulse< double >(64);
        fft.SetArgs(&x64);
        bool i2 = fft.Initialize();
        REQUIRE(i2 == true);
        bool e2 = fft.Execute();
        REQUIRE(e2 == true);
        // Verify: impulse -> constant spectrum
        for(std::size_t k = 0; k < 64; ++k)
            REQUIRE(std::abs(std::abs(x64(k)) - 1.0) < 1e-12);
    }

    // --- Test 10: Idempotent re-Execute ---
    {
        ArrD x = make_random_arr< double >(32, 42);
        ArrD x_orig = x;
        MHO_FastFourierTransform< double > fft;
        fft.SetForward();
        fft.SetArgs(&x);
        fft.Initialize();
        fft.Execute();
        // Execute again (double forward)
        fft.Execute();
        // Now reverse twice to recover
        fft.SetBackward();
        fft.Initialize();
        fft.Execute();
        fft.Execute();
        // Each backward multiplies by N, so x == N*N*x_orig
        for(std::size_t i = 0; i < 32; ++i)
            x(i) /= (32.0 * 32.0);
        double maxerr = 0.0;
        for(std::size_t i = 0; i < 32; ++i)
        {
            double e = std::abs(x(i) - x_orig(i));
            if(e > maxerr)
                maxerr = e;
        }
        REQUIRE(maxerr < 1e-9);
    }

    return 0;
}
