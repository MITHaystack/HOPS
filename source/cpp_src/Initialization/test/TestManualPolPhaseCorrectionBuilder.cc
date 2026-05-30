// Builder test for MHO_ManualPolPhaseCorrectionBuilder: value is a double
// (phase offset); needs "vis".

#include "InitializationTestFixtures.hh"

#include "MHO_ManualPolPhaseCorrection.hh"
#include "MHO_ManualPolPhaseCorrectionBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // vis present -> builds and registers the operator
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_ManualPolPhaseCorrectionBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pc_pol_phase", 30.0));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_ManualPolPhaseCorrection >("pc_pol_phase") != nullptr);
    }

    // no "vis" in the store -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;

        MHO_ManualPolPhaseCorrectionBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pc_pol_phase", 30.0));

        REQUIRE(!builder.Build());
    }

    return 0;
}
