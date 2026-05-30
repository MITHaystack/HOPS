#include <cmath>
#include <iostream>
#include <vector>

#include "MHO_GaussianWhiteNoiseSignal.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

//Helper: compute sample mean and population variance

static void sample_stats(const std::vector< double >& x, double& mean, double& variance)
{
    const double N = static_cast< double >(x.size());
    double sum = 0.0;
    for(std::size_t i = 0; i < x.size(); ++i)
        sum += x[i];
    mean = sum / N;

    double vsum = 0.0;
    for(std::size_t i = 0; i < x.size(); ++i)
    {
        double d = x[i] - mean;
        vsum += d * d;
    }
    variance = vsum / N;
}

int main(int /*argc*/, char** /*argv*/)
{
    // Case 1: Statistical mean/variance (mean=0, stddev=1)
    {
        constexpr std::size_t N = 1000000;
        MHO_GaussianWhiteNoiseSignal sig;
        sig.SetRandomSeed(123);
        sig.SetMean(0.0);
        sig.SetStandardDeviation(1.0);
        sig.Initialize();

        std::vector< double > samples(N);
        for(std::size_t i = 0; i < N; ++i)
            sig.GetSample(0.0, samples[i]);

        double mean, variance;
        sample_stats(samples, mean, variance);

        CHECK_CLOSE(mean, 0.0, 5e-3);
        CHECK_CLOSE(variance, 1.0, 1e-2);
    }

    // Case 2: Seed reproducibility (same seed -> identical sequence)
    {
        constexpr std::size_t N = 1000;
        MHO_GaussianWhiteNoiseSignal a;
        MHO_GaussianWhiteNoiseSignal b;
        a.SetRandomSeed(42);
        a.SetMean(0.0);
        a.SetStandardDeviation(1.0);
        a.Initialize();
        b.SetRandomSeed(42);
        b.SetMean(0.0);
        b.SetStandardDeviation(1.0);
        b.Initialize();

        for(std::size_t i = 0; i < N; ++i)
        {
            double va, vb;
            a.GetSample(0.0, va);
            b.GetSample(0.0, vb);
            REQUIRE(va == vb);
        }
    }

    // Case 3: Different seed -> different sequence
    {
        constexpr std::size_t N = 1000;
        MHO_GaussianWhiteNoiseSignal a;
        MHO_GaussianWhiteNoiseSignal b;
        a.SetRandomSeed(1);
        a.SetMean(0.0);
        a.SetStandardDeviation(1.0);
        a.Initialize();
        b.SetRandomSeed(2);
        b.SetMean(0.0);
        b.SetStandardDeviation(1.0);
        b.Initialize();

        int mismatches = 0;
        for(std::size_t i = 0; i < N; ++i)
        {
            double va, vb;
            a.GetSample(0.0, va);
            b.GetSample(0.0, vb);
            if(va != vb)
                ++mismatches;
        }
        REQUIRE(mismatches > 0);
    }

    // Case 4: Non-zero mean and stddev
    {
        constexpr std::size_t N = 1000000;
        MHO_GaussianWhiteNoiseSignal sig;
        sig.SetRandomSeed(7);
        sig.SetMean(5.0);
        sig.SetStandardDeviation(2.0);
        sig.Initialize();

        std::vector< double > samples(N);
        for(std::size_t i = 0; i < N; ++i)
            sig.GetSample(0.0, samples[i]);

        double mean, variance;
        sample_stats(samples, mean, variance);

        CHECK_CLOSE(mean, 5.0, 1e-2);
        CHECK_CLOSE(variance, 4.0, 5e-2);
    }

    // Case 5: SetStandardDeviation takes absolute value
    {
        constexpr std::size_t N = 1000000;
        MHO_GaussianWhiteNoiseSignal sig;
        sig.SetRandomSeed(7);
        sig.SetMean(0.0);
        sig.SetStandardDeviation(-3.0); //negative - should be fabs'd
        sig.Initialize();

        std::vector< double > samples(N);
        for(std::size_t i = 0; i < N; ++i)
            sig.GetSample(0.0, samples[i]);

        double mean, variance;
        sample_stats(samples, mean, variance);

        CHECK_CLOSE(variance, 9.0, 1e-1);
    }

    // Case 6: Re-Initialize() resets the sequence
    {
        constexpr std::size_t N = 500;
        MHO_GaussianWhiteNoiseSignal sig;
        sig.SetRandomSeed(99);
        sig.SetMean(0.0);
        sig.SetStandardDeviation(1.0);
        sig.Initialize();

        std::vector< double > seqA(N);
        for(std::size_t i = 0; i < N; ++i)
            sig.GetSample(0.0, seqA[i]);

        //Re-initialize - should rebuild generator from stored seed
        sig.Initialize();

        std::vector< double > seqB(N);
        for(std::size_t i = 0; i < N; ++i)
            sig.GetSample(0.0, seqB[i]);

        for(std::size_t i = 0; i < N; ++i)
            REQUIRE(seqA[i] == seqB[i]);
    }

    return 0;
}
