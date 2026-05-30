// Builder test for MHO_DCBlockBuilder: value is a bool gate; needs "vis".

#include "InitializationTestFixtures.hh"

#include "MHO_DCBlock.hh"
#include "MHO_DCBlockBuilder.hh"
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

        MHO_DCBlockBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("dc_block", true));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_DCBlock >("dc_block") != nullptr);
    }

    // value=false -> Build succeeds but no operator is created (gated off)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_DCBlockBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("dc_block", false));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_DCBlock >("dc_block") == nullptr);
    }

    // value=true but no "vis" in the store -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;

        MHO_DCBlockBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("dc_block", true));

        REQUIRE(!builder.Build());
    }

    return 0;
}
