// Builder test for MHO_PassbandBuilder: value is exactly two frequency limits;
// needs both "vis" and "weight".

#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_Passband.hh"
#include "MHO_PassbandBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // exactly two limits, vis+weight present -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);

        std::vector< double > limits = {8000.0, 8060.0};
        MHO_PassbandBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("passband", limits));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_Passband >("passband") != nullptr);
    }

    // wrong number of limits (not 2) -> rejected, Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);

        std::vector< double > limits = {8000.0, 8010.0, 8060.0};
        MHO_PassbandBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("passband", limits));

        REQUIRE(!builder.Build());
    }

    // missing "weight" -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        std::vector< double > limits = {8000.0, 8060.0};
        MHO_PassbandBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("passband", limits));

        REQUIRE(!builder.Build());
    }

    return 0;
}
