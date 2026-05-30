// Unit tests for MHO_OperatorBuilderManager: registers the default operator
// builders (only those whose format key is present in the control format),
// reports builder counts per category, and builds a category's operators from
// the control statements into the toolbox.

#include <string>

#include "InitializationTestFixtures.hh"

#include "MHO_DCBlock.hh"
#include "MHO_FringeData.hh"
#include "MHO_Message.hh"
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // fringe data owns the container + parameter stores the builders draw from
    MHO_FringeData fdata;
    add_vis(*fdata.GetContainerStore());
    add_weight(*fdata.GetContainerStore());

    // control format: two operators with categories + priorities. Other default
    // builders are skipped because their format keys are absent.
    mho_json fmt;
    fmt["dc_block"] = mho_json{{"operator_category", "flagging"}, {"priority", 1.0}};
    fmt["min_weight"] = mho_json{{"operator_category", "selection"}, {"priority", 1.0}};

    MHO_OperatorToolbox toolbox;
    MHO_OperatorBuilderManager manager(&toolbox, &fdata, fmt);
    manager.CreateDefaultBuilders();

    // both formatted builders registered into their categories
    REQUIRE(manager.GetNBuildersInCategory("flagging") >= 1);
    REQUIRE(manager.GetNBuildersInCategory("selection") >= 1);

    // build the "flagging" category from a control statement enabling dc_block
    mho_json control = mho_json::array();
    mho_json block;
    block["value"] = mho_json::array();
    block["statements"] = mho_json::array();
    {
        mho_json stmt;
        stmt["name"] = "dc_block";
        stmt["value"] = true;
        block["statements"].push_back(stmt);
    }
    control.push_back(block);

    manager.SetControlStatements(&control);
    manager.BuildOperatorCategory("flagging");

    REQUIRE(toolbox.GetOperatorAs< MHO_DCBlock >("dc_block") != nullptr);

    // an unsupported category is a no-op (just warns) -- must not crash
    manager.BuildOperatorCategory("not_a_real_category");

    return 0;
}
