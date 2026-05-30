#include <cmath>
#include <iostream>
#include <vector>

#include "MHO_SingleToneSignal.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    // Case 1: Value at t = 0, zero phase
    {
        MHO_SingleToneSignal sig;
        sig.SetToneFrequency(25000.0);
        sig.SetPhaseOffset(0.0);
        sig.Initialize(); //no op

        double s;
        bool ok = sig.GetSample(0.0, s);
        REQUIRE(ok);
        CHECK_CLOSE(s, 0.0, 1e-12);
    }

    // Case 2: Value at t = 0, non-zero phase
    {
        MHO_SingleToneSignal sig;
        sig.SetToneFrequency(25000.0);
        sig.SetPhaseOffset(M_PI / 6.0);

        double s;
        sig.GetSample(0.0, s);
        CHECK_CLOSE(s, 0.5, 1e-12);
    }

    // Case 3: Quarter-period peak
    {
        MHO_SingleToneSignal sig;
        sig.SetToneFrequency(1.0);
        sig.SetPhaseOffset(0.0);

        double s;
        sig.GetSample(0.25, s);
        CHECK_CLOSE(s, 1.0, 1e-12);
    }

    // Case 4: Half-period zero crossing
    {
        MHO_SingleToneSignal sig;
        sig.SetToneFrequency(1.0);
        sig.SetPhaseOffset(0.0);

        double s;
        sig.GetSample(0.5, s);
        CHECK_CLOSE(s, 0.0, 1e-12);
    }

    // Case 5: Explicit formula match over sample grid
    {
        MHO_SingleToneSignal sig;
        sig.SetToneFrequency(25000.0);
        sig.SetPhaseOffset(0.3);
        double sample_freq = 1e6;

        for(int i = 0; i < 100; ++i)
        {
            double t = static_cast< double >(i) / sample_freq;
            double s, ref;
            sig.GetSample(t, s);
            ref = std::sin(0.3 + 2.0 * M_PI * 25000.0 * t);
            CHECK_CLOSE(s, ref, 1e-12);
        }
    }

    // Case 6: Negative time
    {
        MHO_SingleToneSignal sig;
        sig.SetToneFrequency(1.0);
        sig.SetPhaseOffset(0.0);

        double s;
        sig.GetSample(-0.25, s);
        CHECK_CLOSE(s, -1.0, 1e-12);
    }

    // Case 7: SamplingFrequency accessor / independence
    {
        MHO_SingleToneSignal sig;
        sig.SetToneFrequency(1.0);
        sig.SetPhaseOffset(0.0);
        sig.SetSamplingFrequency(-2e6);

        double f = sig.GetSamplingFrequency();
        CHECK_CLOSE(f, 2e6, 1e-12);

        double s;
        sig.GetSample(0.5, s);
        CHECK_CLOSE(s, 0.0, 1e-12);
    }

    // Case 8: Re-use / idempotency
    {
        MHO_SingleToneSignal sig;
        sig.SetToneFrequency(1.0);
        sig.SetPhaseOffset(0.0);

        double s1, s2;
        sig.GetSample(0.25, s1);
        sig.GetSample(0.25, s2);
        REQUIRE(s1 == s2);
    }

    return 0;
}
