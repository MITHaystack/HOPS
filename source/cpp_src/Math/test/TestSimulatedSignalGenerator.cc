#include <iostream>

#include "MHO_SimulatedSignalGenerator.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

//Test-only concrete subclass that counts GenerateSample calls        */

class CountingGenerator: public MHO_SimulatedSignalGenerator
{
    public:
        mutable int call_count = 0;

        void Initialize() override {}

    protected:
        bool GenerateSample(double t, double& s) const override
        {
            ++call_count;
            s = t * 2.0;
            return true;
        }
};

int main(int /*argc*/, char** /*argv*/)
{
    // Case 1: Default sampling frequency is 0
    {
        CountingGenerator g;
        REQUIRE(g.GetSamplingFrequency() == 0.0);
    }

    // Case 2: SetSamplingFrequency takes absolute value
    {
        CountingGenerator g;
        g.SetSamplingFrequency(1e6);
        REQUIRE(g.GetSamplingFrequency() == 1e6);

        g.SetSamplingFrequency(-2e6);
        REQUIRE(g.GetSamplingFrequency() == 2e6);
    }

    // Case 3: GetSample delegates to GenerateSample
    {
        CountingGenerator g;
        g.SetSamplingFrequency(1.0);
        g.Initialize();
        double v;
        bool ok = g.GetSample(3.5, v);
        REQUIRE(ok);
        REQUIRE(v == 7.0);
        REQUIRE(g.call_count == 1);
    }

    // Case 4: Polymorphic call via base pointer
    {
        CountingGenerator g;
        MHO_SimulatedSignalGenerator* base = &g;
        base->Initialize();
        double v;
        base->GetSample(2.0, v);
        REQUIRE(v == 4.0);
        REQUIRE(g.call_count == 1);
    }

    // Case 5: Repeated GetSample calls accumulate count
    {
        CountingGenerator g;
        g.call_count = 0;
        for(int i = 0; i < 100; ++i)
        {
            double v;
            g.GetSample(i, v);
        }
        REQUIRE(g.call_count == 100);
    }

    return 0;
}
