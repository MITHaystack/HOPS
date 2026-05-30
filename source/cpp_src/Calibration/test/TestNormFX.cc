#include <iostream>
#include <string>
#include <complex>

#include "MHO_NormFX.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Trivial concrete subclass of abstract MHO_NormFX

class TrivialNormFX : public MHO_NormFX
{
    public:
        bool init_ip  = false;
        bool init_oop = false;
        bool exec_ip  = false;
        bool exec_oop = false;

        weight_type* SeenWeights() const { return fWeights; }

    protected:
        bool InitializeInPlace(XArgType* /*in*/) override
        { init_ip = true; return true; }

        bool InitializeOutOfPlace(const XArgType* /*in*/, XArgType* /*out*/) override
        { init_oop = true; return true; }

        bool ExecuteInPlace(XArgType* /*in*/) override
        { exec_ip = true; return true; }

        bool ExecuteOutOfPlace(const XArgType* /*in*/, XArgType* /*out*/) override
        { exec_oop = true; return true; }
};

// Fixtures

static void build_vis(visibility_type& vis)
{
    vis.Resize(1, 1, 1, 4);
    for (std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for (std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for (std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for (std::size_t fr = 0; fr < vis.GetDimension(FREQ_AXIS); fr++)
                    vis(pp, ch, ap, fr) = std::complex<double>(1.0, 0.0);
}

static void build_weight(weight_type& w)
{
    w.Resize(1, 1, 1, 1);
}

// Test cases

// CASE 1 - Default weight pointer is null
static int test_default_weights_null()
{
    TrivialNormFX op;
    REQUIRE(op.SeenWeights() == nullptr);
    return 0;
}

// CASE 2 - SetWeights stores the pointer
static int test_setweights_stores_pointer()
{
    visibility_type vis;
    weight_type     w;
    build_weight(w);

    TrivialNormFX op;
    op.SetWeights(&w);
    REQUIRE(op.SeenWeights() == &w);
    return 0;
}

// CASE 3 - Out-of-place lifecycle dispatch
static int test_out_of_place_dispatch()
{
    visibility_type vis;
    visibility_type out;
    weight_type     w;
    build_vis(vis);
    build_weight(w);

    TrivialNormFX op;
    op.SetWeights(&w);
    op.SetArgs(&vis, &out);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(op.init_oop  == true);
    REQUIRE(op.exec_oop  == true);
    REQUIRE(op.init_ip   == false);
    REQUIRE(op.exec_ip   == false);
    return 0;
}

// CASE 4 - In-place lifecycle dispatch
static int test_in_place_dispatch()
{
    visibility_type vis;
    weight_type     w;
    build_vis(vis);
    build_weight(w);

    TrivialNormFX op;
    op.SetWeights(&w);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());

    REQUIRE(op.init_ip  == true);
    REQUIRE(op.exec_ip  == true);
    REQUIRE(op.init_oop == false);
    REQUIRE(op.exec_oop == false);
    return 0;
}

// CASE 5 - SetWeights re-settable / reset to null
static int test_setweights_resettability()
{
    visibility_type vis;
    weight_type     w;
    build_weight(w);

    TrivialNormFX op;
    op.SetWeights(&w);
    REQUIRE(op.SeenWeights() == &w);
    op.SetWeights(nullptr);
    REQUIRE(op.SeenWeights() == nullptr);
    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if (test_default_weights_null())      return 1;
    if (test_setweights_stores_pointer()) return 1;
    if (test_out_of_place_dispatch())     return 1;
    if (test_in_place_dispatch())         return 1;
    if (test_setweights_resettability())  return 1;

    return 0;
}
