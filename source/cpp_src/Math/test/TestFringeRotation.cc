#include "MHO_FringeRotation.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include <cmath>
#include <complex>
#include <iostream>
#include <limits>

using namespace hops;

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // --- Case 1: Zero arguments yield unit phasor ---
    {
        MHO_FringeRotation rot;
        auto c = rot.vrot(0, 0, 0, 0, 0);
        REQUIRE_CLOSE_CPLX(c, std::complex< double >(1.0, 0.0), 1e-12);
    }

    // --- Case 2: Pure delay rate at ref_freq cancellation ---
    {
        MHO_FringeRotation rot;
        auto c = rot.vrot(1.0, 8000.0, 8000.0, 1e-12, 0.0);
        // theta = 8000*1e-12*1.0 = 8e-9
        REQUIRE_CLOSE_CPLX(c, std::complex< double >(0.9999999999999988, -5.026548245743667e-08), 1e-9);
    }

    // --- Case 3: Pure mbd term, freq == ref_freq gives unit phasor ---
    {
        MHO_FringeRotation rot;
        auto c = rot.vrot(0.0, 8000.0, 8000.0, 0.0, 1.0);
        REQUIRE_CLOSE_CPLX(c, std::complex< double >(1.0, 0.0), 1e-12);
    }

    // --- Case 4: Pure mbd term at freq != ref_freq ---
    {
        MHO_FringeRotation rot;
        auto c = rot.vrot(0.0, 8032.0, 8000.0, 0.0, 1e-3);
        // theta = 1e-3*(8032-8000) = 0.032
        REQUIRE_CLOSE_CPLX(c, std::complex< double >(0.9798550523842469, -0.19970998051440703), 1e-9);
    }

    // --- Case 5: Combined delay-rate + mbd term ---
    {
        MHO_FringeRotation rot;
        auto c = rot.vrot(2.5, 8064.0, 8000.0, 2e-13, 5e-4);
        // theta = 8064*2e-13*2.5 + 5e-4*(8064-8000) = 4.032e-9 + 0.032
        REQUIRE_CLOSE_CPLX(c, std::complex< double >(0.9798550473248332, -0.19971000533786196), 1e-9);
    }

    // --- Case 6: Sideband correction OFF when fSideband == 0 ---
    {
        MHO_FringeRotation rot;
        rot.SetSideband(0);
        rot.SetNSBDBins(32);
        rot.SetSBDMaxBin(5);
        rot.SetSBDMax(0.7);
        rot.SetSBDSeparation(1.0);
        double corr = rot.CalcSidebandCorrection(0.1);
        REQUIRE(corr == 0.0);
    }

    // --- Case 7: Sideband correction, USB, optimize_closure OFF ---
    {
        MHO_FringeRotation rot;
        rot.SetSideband(1);
        rot.SetNSBDBins(32);
        rot.SetSBDMaxBin(5);
        rot.SetSBDMax(0.7);
        rot.SetSBDSeparation(0.5);
        rot.SetOptimizeClosureFalse();
        double corr = rot.CalcSidebandCorrection(0.1);
        // corr = (32-5)*0.125*1 + (0.125*0.7*1)/0.5 = 3.375 + 0.175 = 3.55
        CHECK_CLOSE(corr, 3.55, 1e-15);
    }

    // --- Case 8: Sideband correction, LSB, optimize_closure ON ---
    {
        MHO_FringeRotation rot;
        rot.SetSideband(-1);
        rot.SetNSBDBins(32);
        rot.SetSBDMaxBin(5);
        rot.SetSBDMax(0.7);
        rot.SetSBDSeparation(0.5);
        rot.SetOptimizeClosureTrue();
        double corr = rot.CalcSidebandCorrection(0.1);
        // corr = (32-5)*0.125*(-1) + 0.125*0.1*(-1)/0.5 = -3.375 - 0.025 = -3.40
        CHECK_CLOSE(corr, -3.40, 1e-15);
    }

    // --- Case 9: vrot picks up the sideband correction ---
    {
        MHO_FringeRotation rot;
        rot.SetSideband(1);
        rot.SetNSBDBins(32);
        rot.SetSBDMaxBin(5);
        rot.SetSBDMax(0.7);
        rot.SetSBDSeparation(0.5);
        rot.SetOptimizeClosureFalse();
        auto c = rot.vrot(0.0, 8000.0, 8000.0, 0.0, 0.1);
        // theta = 3.55 (mod 1 = 0.55); vrot = exp(-2*pi*i*0.55)
        REQUIRE_CLOSE_CPLX(c, std::complex< double >(-0.9510565162951544, 0.3090169943749449), 1e-9);
    }

    // --- Case 10: Re-use / idempotency ---
    {
        MHO_FringeRotation rot;
        auto first = rot.vrot(1.0, 8000.0, 8000.0, 1e-12, 0.0);
        for(int i = 0; i < 1000; i++)
        {
            auto c = rot.vrot(1.0, 8000.0, 8000.0, 1e-12, 0.0);
            // vrot is const with no internal state - all calls should be bitwise identical
            REQUIRE(c.real() == first.real() && c.imag() == first.imag());
        }
    }

    return 0;
}
