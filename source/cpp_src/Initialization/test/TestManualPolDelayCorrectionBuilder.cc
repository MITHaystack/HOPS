// Builder test for MHO_ManualPolDelayCorrectionBuilder: value is a double (delay
// offset); needs "vis" and reads /control/config/ref_freq from the parameter
// store (read before the vis check, so a parameter store is required).

#include "InitializationTestFixtures.hh"

#include "MHO_ManualPolDelayCorrection.hh"
#include "MHO_ManualPolDelayCorrectionBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // vis present + parameter store with ref_freq -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/control/config/ref_freq", 8000.0);
        add_vis(store);

        MHO_ManualPolDelayCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pc_pol_delay", 5.0));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_ManualPolDelayCorrection >("pc_pol_delay") != nullptr);
    }

    // no "vis" in the store -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/control/config/ref_freq", 8000.0);

        MHO_ManualPolDelayCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pc_pol_delay", 5.0));

        REQUIRE(!builder.Build());
    }

    return 0;
}
