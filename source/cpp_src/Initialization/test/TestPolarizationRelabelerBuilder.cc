// Builder test for MHO_PolarizationRelabelerBuilder: value is {pol1, pol2};
// builds a MHO_PolarizationProductRelabeler for both "vis" and "weight"
// (registered as "<name>:vis"/"<name>:weight").

#include <string>

#include "InitializationTestFixtures.hh"

#include "MHO_Message.hh"
#include "MHO_PolarizationProductRelabeler.hh"
#include "MHO_PolarizationRelabelerBuilder.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static mho_json relabel_value(const std::string& p1, const std::string& p2)
{
    mho_json v;
    v["pol1"] = p1;
    v["pol2"] = p2;
    return v;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // vis + weight present -> builds both relabelers with :vis/:weight suffixes
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);

        MHO_PolarizationRelabelerBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pol_relabel", relabel_value("X", "Y")));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_PolarizationProductRelabeler< visibility_type > >("pol_relabel:vis") != nullptr);
        REQUIRE(toolbox.GetOperatorAs< MHO_PolarizationProductRelabeler< weight_type > >("pol_relabel:weight") != nullptr);
    }

    // missing "weight" -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_PolarizationRelabelerBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pol_relabel", relabel_value("X", "Y")));

        REQUIRE(!builder.Build());
    }

    return 0;
}
