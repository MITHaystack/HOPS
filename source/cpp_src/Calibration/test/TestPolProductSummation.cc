#include <cmath>
#include <complex>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_PolProductSummation.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* Standard fixture: npp=4, nchan=1, nap=1, nfreq=2.
   XX=(1,0), YY=(1,0), XY=(0,0), YX=(0,0) for all (ch,t,f). */
static void build_vis(visibility_type& vis,
                      std::complex<double> xx_val, std::complex<double> yy_val,
                      std::complex<double> xy_val, std::complex<double> yx_val)
{
    vis.Resize(4, 1, 1, 2);
    auto pax = &(std::get<POLPROD_AXIS>(vis));
    pax->at(0) = "XX";
    pax->at(1) = "YY";
    pax->at(2) = "XY";
    pax->at(3) = "YX";

    vis(0, 0, 0, 0) = xx_val;  vis(0, 0, 0, 1) = xx_val;
    vis(1, 0, 0, 0) = yy_val;  vis(1, 0, 0, 1) = yy_val;
    vis(2, 0, 0, 0) = xy_val;  vis(2, 0, 0, 1) = xy_val;
    vis(3, 0, 0, 0) = yx_val;  vis(3, 0, 0, 1) = yx_val;
}

// Standard weight fixture: npp=4, nchan=1, nap=1, nfreq=1; all 0.5
static void build_weights(weight_type& w)
{
    w.Resize(4, 1, 1, 1);
    w(0, 0, 0, 0) = 0.5;
    w(1, 0, 0, 0) = 0.5;
    w(2, 0, 0, 0) = 0.5;
    w(3, 0, 0, 0) = 0.5;
}

// Configure the operator with the standard pol-product set {"XX","YY","XY","YX"}.
static void configure_op(MHO_PolProductSummation& op, visibility_type& vis,
                         weight_type& w,
                         double ref_par, double rem_par)
{
    std::vector<std::string> ppset = {"XX", "YY", "XY", "YX"};
    op.SetPolProductSet(ppset);
    op.SetPolProductSumLabel("I");
    op.SetWeights(&w);
    op.SetReferenceParallacticAngle(ref_par);
    op.SetRemoteParallacticAngle(rem_par);
    op.SetArgs(&vis);
}

// Test cases

/* CASE 1 - pseudo-Stokes-I prefactors at dpar=0.
   With dpar=0: cos(0)=1, sin(0)=0.  prefac_sum forced to 2.0.
   XX *= 1/2, YY *= 1/2, XY *= 0/2, YX *= 0/2.
   Sum = (1*0.5)+(1*0.5)+0+0 = 1.0. */
static int test_pseudo_stokes_i_dpar0()
{
    visibility_type vis;
    weight_type w;
    build_vis(vis, std::complex<double>(1.0, 0.0),
                 std::complex<double>(1.0, 0.0),
                 std::complex<double>(0.0, 0.0),
                 std::complex<double>(0.0, 0.0));
    build_weights(w);

    MHO_PolProductSummation op;
    configure_op(op, vis, w, 0.0, 0.0);

    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Polprod axis collapses to 1
    REQUIRE(vis.GetDimension(POLPROD_AXIS) == 1);

    // Check value at both frequency points
    for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++) {
        CHECK_CLOSE(vis(0, 0, 0, f).real(), 1.0, 1e-9);
        CHECK_CLOSE(vis(0, 0, 0, f).imag(), 0.0, 1e-9);
    }

    return 0;
}

// CASE 2 - Output polprod label fixed to "I".
static int test_output_label()
{
    visibility_type vis;
    weight_type w;
    build_vis(vis, std::complex<double>(1.0, 0.0),
                 std::complex<double>(1.0, 0.0),
                 std::complex<double>(0.0, 0.0),
                 std::complex<double>(0.0, 0.0));
    build_weights(w);

    MHO_PolProductSummation op;
    configure_op(op, vis, w, 0.0, 0.0);

    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    auto pax = &(std::get<POLPROD_AXIS>(vis));
    REQUIRE(pax->at(0) == "I");

    return 0;
}

/* CASE 3 - Weight summation and n_summed_polprod tagging.
   NOTE: the weight averaging is not done, weights are SUMMED. 4 * 0.5 = 2.0. */
static int test_weight_summation()
{
    visibility_type vis;
    weight_type w;
    build_vis(vis, std::complex<double>(1.0, 0.0),
                 std::complex<double>(1.0, 0.0),
                 std::complex<double>(0.0, 0.0),
                 std::complex<double>(0.0, 0.0));
    build_weights(w);

    MHO_PolProductSummation op;
    configure_op(op, vis, w, 0.0, 0.0);

    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    // Weight polprod axis reduced to 1; summed value = 4 * 0.5 = 2.0
    REQUIRE(w.GetDimension(POLPROD_AXIS) == 1);
    CHECK_CLOSE(w(0, 0, 0, 0), 2.0, 1e-9);

    // n_summed_polprod tag equals ppset size (4)
    double n_summed;
    REQUIRE(w.Retrieve("n_summed_polprod", n_summed));
    CHECK_CLOSE(n_summed, 4.0, 1e-9);

    return 0;
}

/* CASE 4 - Non-trivial parallactic angle (dpar = 90 degrees).
   cos(pi/2) ~ 0, sin(pi/2) = 1.
   All four pol-products filled with (1,0).
   XX factor = cos(pi/2) ~ 0, YY = cos(pi/2) ~ 0,
   YX factor = sin(pi/2) = 1, XY factor = sin(-pi/2) = -1.
   prefac_sum = 2 (label "I").
   Sum ~ (1*0/2) + (1*0/2) + (1*(-1)/2) + (1*1/2) = 0.0.
   Use slightly wider tolerance for floating-point cos(pi/2). */
static int test_nonzero_dpar()
{
    visibility_type vis;
    weight_type w;
    build_vis(vis, std::complex<double>(1.0, 0.0),
                 std::complex<double>(1.0, 0.0),
                 std::complex<double>(1.0, 0.0),
                 std::complex<double>(1.0, 0.0));
    build_weights(w);

    MHO_PolProductSummation op;
    configure_op(op, vis, w, 0.0, 90.0);

    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(vis.GetDimension(POLPROD_AXIS) == 1);
    for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++) {
        CHECK_CLOSE(vis(0, 0, 0, f).real(), 0.0, 1e-12);
        CHECK_CLOSE(vis(0, 0, 0, f).imag(), 0.0, 1e-12);
    }

    return 0;
}

/* CASE 5 - Out-of-place path.
   Same numeric result as Case 1, but written to a separate output array. */
static int test_out_of_place()
{
    visibility_type vis;
    weight_type w;
    visibility_type out;
    build_vis(vis, std::complex<double>(1.0, 0.0),
                 std::complex<double>(1.0, 0.0),
                 std::complex<double>(0.0, 0.0),
                 std::complex<double>(0.0, 0.0));
    build_weights(w);

    // Pre-size output to match input dimensions (reducer needs it)
    out.Resize(vis.GetDimension(POLPROD_AXIS), vis.GetDimension(CHANNEL_AXIS),
               vis.GetDimension(TIME_AXIS), vis.GetDimension(FREQ_AXIS));

    MHO_PolProductSummation op;
    configure_op(op, vis, w, 0.0, 0.0);
    // Override SetArgs for out-of-place mode
    op.SetArgs(&vis, &out);

    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(out.GetDimension(POLPROD_AXIS) == 1);
    for (std::size_t f = 0; f < out.GetDimension(FREQ_AXIS); f++) {
        CHECK_CLOSE(out(0, 0, 0, f).real(), 1.0, 1e-9);
        CHECK_CLOSE(out(0, 0, 0, f).imag(), 0.0, 1e-9);
    }

    // n_summed_polprod tag should still be set on weights
    double n_summed;
    REQUIRE(w.Retrieve("n_summed_polprod", n_summed));
    CHECK_CLOSE(n_summed, 4.0, 1e-9);

    return 0;
}

/* CASE 6 - Label outside the pol-product set -> prefactor 1.0.
   ppset = {"XX","YY"} but vis still has 4 pol-products (XX,YY,XY,YX).
   GetPrefactor returns 1.0 for labels not in the set.
   |ppset| > 1 so XX/YY use cos(0)=1; XY/YX get 1.0 (early return).
   prefac_sum = |1|+|1|+|1|+|1| = 4.0 (label "I" forces 2.0).
   PreMultiply: XX *= 1/2, YY *= 1/2, XY *= 1/2, YX *= 1/2.
   With XX=YY=(1,0), XY=YX=(0,0): sum = 0.5+0.5+0+0 = 1.0.
   n_summed_polprod = ppset.size() = 2. */
static int test_label_not_in_set()
{
    visibility_type vis;
    weight_type w;
    build_vis(vis, std::complex<double>(1.0, 0.0),
                 std::complex<double>(1.0, 0.0),
                 std::complex<double>(0.0, 0.0),
                 std::complex<double>(0.0, 0.0));
    build_weights(w);

    MHO_PolProductSummation op;
    std::vector<std::string> small_ppset = {"XX", "YY"};
    op.SetPolProductSet(small_ppset);
    op.SetPolProductSumLabel("I");
    op.SetWeights(&w);
    op.SetReferenceParallacticAngle(0.0);
    op.SetRemoteParallacticAngle(0.0);
    op.SetArgs(&vis);

    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(vis.GetDimension(POLPROD_AXIS) == 1);
    for (std::size_t f = 0; f < vis.GetDimension(FREQ_AXIS); f++) {
        CHECK_CLOSE(vis(0, 0, 0, f).real(), 1.0, 1e-9);
        CHECK_CLOSE(vis(0, 0, 0, f).imag(), 0.0, 1e-9);
    }

    // n_summed_polprod equals ppset size (2), not the actual polprod count (4)
    double n_summed;
    REQUIRE(w.Retrieve("n_summed_polprod", n_summed));
    CHECK_CLOSE(n_summed, 2.0, 1e-9);

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_pseudo_stokes_i_dpar0())  return 1;
    if (test_output_label())           return 1;
    if (test_weight_summation())       return 1;
    if (test_nonzero_dpar())           return 1;
    if (test_out_of_place())           return 1;
    if (test_label_not_in_set())       return 1;

    return 0;
}
