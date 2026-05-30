#include <cmath>
#include <complex>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_CircularFieldRotationCorrection.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Local oracle: replicate compute_field_rotations_fixed from
// MHO_CircularFieldRotationCorrection.cc (file-scope, not exposed).
// Used for CASE 3 and CASE 4 to verify sign-pattern math against
// hand-derived values.

enum class MountTypeLocal : int
{
    None = 0,
    Cassegrain = 1,
    NasmythLeft = 2,
    NasmythRight = 3
};

static void oracle_field_rotations(std::complex<double> cpolvalue[4],
                                   double par_angle[2],
                                   double elevation[2],
                                   MountTypeLocal mount_type[2])
{
    const double elmult[4] = {0.0, 0.0, -1.0, +1.0};
    const double em0 = elmult[static_cast<int>(mount_type[0])];
    const double em1 = elmult[static_cast<int>(mount_type[1])];

    double a_ref = par_angle[0] + em0 * elevation[0];
    double a_rem = par_angle[1] + em1 * elevation[1];

    // LL: +a_ref - a_rem
    cpolvalue[0] = std::exp(std::complex<double>(0, -1.0) * (+a_ref - a_rem));
    // RR: -a_ref + a_rem
    cpolvalue[1] = std::exp(std::complex<double>(0, -1.0) * (-a_ref + a_rem));
    // LR: +a_ref + a_rem
    cpolvalue[2] = std::exp(std::complex<double>(0, -1.0) * (+a_ref + a_rem));
    // RL: -a_ref - a_rem
    cpolvalue[3] = std::exp(std::complex<double>(0, -1.0) * (-a_ref - a_rem));
}

// Helpers

static visibility_type make_full_vis(double fill_re, double fill_im)
{
    visibility_type vis;
    vis.Resize(4, 2, 2, 2); // npol=4, nchan=2, nap=2, nspec=2
    auto pax = &(std::get<POLPROD_AXIS>(vis));
    pax->at(0) = "LL";
    pax->at(1) = "RR";
    pax->at(2) = "LR";
    pax->at(3) = "RL";
    std::complex<double> fill(fill_re, fill_im);
    for (std::size_t pp = 0; pp < 4; pp++)
    {
        for (std::size_t ch = 0; ch < 2; ch++)
        {
            for (std::size_t ap = 0; ap < 2; ap++)
            {
                for (std::size_t sp = 0; sp < 2; sp++)
                {
                    vis(pp, ch, ap, sp) = fill;
                }
            }
        }
    }
    return vis;
}

static std::vector<std::string> full_pp_set()
{
    return std::vector<std::string>{"LL", "RR", "LR", "RL"};
}

static int check_all_near(const visibility_type& vis,
                          double expected_re, double expected_im,
                          double tol)
{
    std::complex<double> expected(expected_re, expected_im);
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
    {
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
        {
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
            {
                for (std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                {
                    std::complex<double> val = vis(pp, ch, ap, sp);
                    CHECK_CLOSE(val.real(), expected.real(), tol);
                    CHECK_CLOSE(val.imag(), expected.imag(), tol);
                }
            }
        }
    }
    return 0;
}

// Case 1 -- Zero-angle no-op (public operator interface)
/* With "no_mount" and nullptr station data, parallactic angle and
   elevation are both 0.  Therefore radangle == 0 for all four
   pol-products => prefactor == exp(0) == (1,0) => no-op.              */

static int test_zero_angle_noop()
{
    visibility_type vis = make_full_vis(2.0, 0.0);
    std::vector<std::string> ppset = full_pp_set();

    MHO_CircularFieldRotationCorrection op;
    op.SetPolProductSet(ppset);
    op.SetReferenceMountType("no_mount");
    op.SetRemoteMountType("no_mount");
    // station data left as nullptr; fourfit ref time left default

    op.SetArgs(&vis);
    op.Initialize();
    op.Execute();

    // Every element should remain (2.0, 0.0) -- tolerance 1e-12
    return check_all_near(vis, 2.0, 0.0, 1e-12);
}

// Case 2 -- Pol-product not in set is zeroed

static int test_pol_not_in_set_zeroed()
{
    visibility_type vis = make_full_vis(2.0, 0.0);
    std::vector<std::string> ppset{"LL", "RR"};  // only LL and RR

    MHO_CircularFieldRotationCorrection op;
    op.SetPolProductSet(ppset);
    op.SetReferenceMountType("no_mount");
    op.SetRemoteMountType("no_mount");

    op.SetArgs(&vis);
    op.Initialize();
    op.Execute();

    // LL (pp=0) and RR (pp=1) should be unchanged
    for (std::size_t pp = 0; pp <= 1; pp++)
    {
        for (std::size_t ch = 0; ch < 2; ch++)
        {
            for (std::size_t ap = 0; ap < 2; ap++)
            {
                for (std::size_t sp = 0; sp < 2; sp++)
                {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), 2.0, 1e-12);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), 0.0, 1e-12);
                }
            }
        }
    }
    // LR (pp=2) and RL (pp=3) should be zeroed
    for (std::size_t pp = 2; pp <= 3; pp++)
    {
        for (std::size_t ch = 0; ch < 2; ch++)
        {
            for (std::size_t ap = 0; ap < 2; ap++)
            {
                for (std::size_t sp = 0; sp < 2; sp++)
                {
                    CHECK_CLOSE(vis(pp, ch, ap, sp).real(), 0.0, 1e-12);
                    CHECK_CLOSE(vis(pp, ch, ap, sp).imag(), 0.0, 1e-12);
                }
            }
        }
    }
    return 0;
}

// Case 3 -- Sign pattern for known angles (local oracle)
/* par_angle = {0.30, 0.10} rad, elevation = {0,0}, mount = {None,None}
   a_ref = 0.30, a_rem = 0.10:
     LL: radangle = +0.20 => exp(-i*0.20)
     RR: radangle = -0.20 => exp(+i*0.20)
     LR: radangle = +0.40 => exp(-i*0.40)
     RL: radangle = -0.40 => exp(+i*0.40)                                       */

static int test_sign_pattern()
{
    double par_angle[2] = {0.30, 0.10};
    double elevation[2] = {0.0, 0.0};
    MountTypeLocal mount[2] = {MountTypeLocal::None, MountTypeLocal::None};

    std::complex<double> cpol[4];
    oracle_field_rotations(cpol, par_angle, elevation, mount);

    const double tol = 1e-12;

    // LL: radangle = +0.20 => exp(-i*0.20)
    CHECK_CLOSE(cpol[0].real(), std::cos(0.20), tol);
    CHECK_CLOSE(cpol[0].imag(), -std::sin(0.20), tol);

    // RR: radangle = -0.20 => exp(+i*0.20)
    CHECK_CLOSE(cpol[1].real(), std::cos(-0.20), tol);
    CHECK_CLOSE(cpol[1].imag(), -std::sin(-0.20), tol);

    // LR: radangle = +0.40 => exp(-i*0.40)
    CHECK_CLOSE(cpol[2].real(), std::cos(0.40), tol);
    CHECK_CLOSE(cpol[2].imag(), -std::sin(0.40), tol);

    // RL: radangle = -0.40 => exp(+i*0.40)
    CHECK_CLOSE(cpol[3].real(), std::cos(-0.40), tol);
    CHECK_CLOSE(cpol[3].imag(), -std::sin(-0.40), tol);

    // Also verify unit magnitude
    for (int i = 0; i < 4; i++)
    {
        CHECK_CLOSE(std::abs(cpol[i]), 1.0, tol);
    }

    return 0;
}

// Case 4 -- Nasmyth elevation multiplier (local oracle)
/* par_angle={0,0}, elevation={0.5,0}, mount={NasmythLeft, None}
   elmult[NasmythLeft]=-1 => a_ref = 0 + (-1)*0.5 = -0.5, a_rem = 0
   LL: radangle = a_ref - a_rem = -0.5 => exp(-i*(-0.5)) = exp(+i*0.5)
   RR: radangle = -a_ref + a_rem = +0.5 => exp(-i*(+0.5)) = exp(-i*0.5)          */

static int test_nasmyth_elevation_multiplier()
{
    double par_angle[2] = {0.0, 0.0};
    double elevation[2] = {0.5, 0.0};
    MountTypeLocal mount[2] = {MountTypeLocal::NasmythLeft, MountTypeLocal::None};

    std::complex<double> cpol[4];
    oracle_field_rotations(cpol, par_angle, elevation, mount);

    const double tol = 1e-12;

    // a_ref = -0.5, a_rem = 0
    // LL: radangle = -0.5 => exp(-i*(-0.5)) = exp(+i*0.5)
    CHECK_CLOSE(cpol[0].real(), std::cos(0.5), tol);
    CHECK_CLOSE(cpol[0].imag(), std::sin(0.5), tol);

    // RR: radangle = +0.5 => exp(-i*0.5)
    CHECK_CLOSE(cpol[1].real(), std::cos(0.5), tol);
    CHECK_CLOSE(cpol[1].imag(), -std::sin(0.5), tol);

    // LR: radangle = -0.5 => exp(+i*0.5)
    CHECK_CLOSE(cpol[2].real(), std::cos(0.5), tol);
    CHECK_CLOSE(cpol[2].imag(), std::sin(0.5), tol);

    // RL: radangle = +0.5 => exp(-i*0.5)
    CHECK_CLOSE(cpol[3].real(), std::cos(0.5), tol);
    CHECK_CLOSE(cpol[3].imag(), -std::sin(0.5), tol);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_zero_angle_noop()) return 1;
    if (test_pol_not_in_set_zeroed()) return 1;
    if (test_sign_pattern()) return 1;
    if (test_nasmyth_elevation_multiplier()) return 1;

    return 0;
}
