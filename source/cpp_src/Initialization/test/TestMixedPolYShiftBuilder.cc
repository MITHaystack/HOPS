// Builder test for MHO_MixedPolYShiftBuilder: value is a bool gate; needs "vis".

#include "InitializationTestFixtures.hh"

#include "MHO_MixedPolYShift.hh"
#include "MHO_MixedPolYShiftBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // value=true, vis present -> builds and registers the operator
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_MixedPolYShiftBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("mixed_pol_y_shift", true));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_MixedPolYShift >("mixed_pol_y_shift") != nullptr);
    }

    // value=false -> Build succeeds but no operator created (gated off)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_MixedPolYShiftBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("mixed_pol_y_shift", false));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_MixedPolYShift >("mixed_pol_y_shift") == nullptr);
    }

    // value=true but no "vis" in the store -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;

        MHO_MixedPolYShiftBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("mixed_pol_y_shift", true));

        REQUIRE(!builder.Build());
    }

    return 0;
}
