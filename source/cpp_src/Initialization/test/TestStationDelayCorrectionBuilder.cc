// Builder test for MHO_StationDelayCorrectionBuilder: value is a double (delay,
// ns); always builds when "vis" is present. NOTE: this builder reads
// /control/config/ref_freq from the parameter store, so a parameter store is
// required

#include "InitializationTestFixtures.hh"

#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_StationDelayCorrection.hh"
#include "MHO_StationDelayCorrectionBuilder.hh"
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

        MHO_StationDelayCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("station_delay", 12.5));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_StationDelayCorrection >("station_delay") != nullptr);
    }

    // no "vis" in the store -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/control/config/ref_freq", 8000.0);

        MHO_StationDelayCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("station_delay", 12.5));

        REQUIRE(!builder.Build());
    }

    return 0;
}
