// Builder test for MHO_NotchesBuilder: value is a list of frequency limits (must
// be an even count, paired); needs both "vis" and "weight".

#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_Notches.hh"
#include "MHO_NotchesBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // even number of limits, vis+weight present -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);

        std::vector< double > limits = {8000.0, 8010.0, 8050.0, 8060.0};
        MHO_NotchesBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("notches", limits));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_Notches >("notches") != nullptr);
    }

    // odd number of limits -> rejected, Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);

        std::vector< double > limits = {8000.0, 8010.0, 8050.0};
        MHO_NotchesBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("notches", limits));

        REQUIRE(!builder.Build());
    }

    // missing "weight" -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        std::vector< double > limits = {8000.0, 8010.0};
        MHO_NotchesBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("notches", limits));

        REQUIRE(!builder.Build());
    }

    return 0;
}
