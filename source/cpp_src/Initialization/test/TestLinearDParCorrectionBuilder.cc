// Builder test for MHO_LinearDParCorrectionBuilder: needs "vis" and reads
// /config/polprod_set plus /ref_station/parallactic_angle and
// /rem_station/parallactic_angle from the parameter store.

#include <string>
#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_LinearDParCorrection.hh"
#include "MHO_LinearDParCorrectionBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static void fill_pstore(MHO_ParameterStore& p)
{
    p.Set("/config/polprod_set", std::vector< std::string >{"XX", "YY"});
    p.Set("/ref_station/parallactic_angle", 0.1);
    p.Set("/rem_station/parallactic_angle", 0.2);
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // all required parameters + vis present -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        fill_pstore(pstore);
        add_vis(store);

        MHO_LinearDParCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("linear_dpar", true));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_LinearDParCorrection >("linear_dpar") != nullptr);
    }

    // polprod_set absent -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore; // empty
        add_vis(store);

        MHO_LinearDParCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("linear_dpar", true));

        REQUIRE(!builder.Build());
    }

    return 0;
}
