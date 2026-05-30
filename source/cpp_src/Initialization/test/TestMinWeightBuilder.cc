// Builder test for MHO_MinWeightBuilder: value is a double (and a gate); needs "weight".

#include "InitializationTestFixtures.hh"

#include "MHO_MinWeight.hh"
#include "MHO_MinWeightBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // nonzero value, weight present -> builds and registers the operator
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_weight(store);

        MHO_MinWeightBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("min_weight", 0.5));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_MinWeight >("min_weight") != nullptr);
    }

    // value == 0 -> Build succeeds but no operator created (gated off)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_weight(store);

        MHO_MinWeightBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("min_weight", 0.0));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_MinWeight >("min_weight") == nullptr);
    }

    // nonzero value but no "weight" in the store -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;

        MHO_MinWeightBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("min_weight", 0.5));

        REQUIRE(!builder.Build());
    }

    return 0;
}
