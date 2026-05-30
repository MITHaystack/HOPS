// Builder test for MHO_LSBOffsetBuilder: value is a double (LSB phase offset);
// always builds when "vis" is present.

#include "InitializationTestFixtures.hh"

#include "MHO_LSBOffset.hh"
#include "MHO_LSBOffsetBuilder.hh"
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

        MHO_LSBOffsetBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("lsb_offset", 45.0));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_LSBOffset >("lsb_offset") != nullptr);
    }

    // no "vis" in the store -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;

        MHO_LSBOffsetBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("lsb_offset", 45.0));

        REQUIRE(!builder.Build());
    }

    return 0;
}
