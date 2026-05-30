// Builder test for MHO_ManualChannelDelayCorrectionBuilder: value is
// {channel_names, pc_delays}; needs a non-empty channel->delay map and "vis".

#include <string>
#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_ManualChannelDelayCorrection.hh"
#include "MHO_ManualChannelDelayCorrectionBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static mho_json delay_value(const std::string& names, const std::vector< double >& delays)
{
    mho_json v;
    v["channel_names"] = names;
    v["pc_delays"] = delays;
    return v;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // valid channel/delay map + vis present -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_ManualChannelDelayCorrectionBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pc_delays_x", delay_value("abcd", {1.0, 2.0, 3.0, 4.0})));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_ManualChannelDelayCorrection >("pc_delays_x") != nullptr);
    }

    // empty map -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_ManualChannelDelayCorrectionBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pc_delays_x", delay_value("", {})));

        REQUIRE(!builder.Build());
    }

    // valid map but no "vis" -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;

        MHO_ManualChannelDelayCorrectionBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pc_delays_x", delay_value("abcd", {1.0, 2.0, 3.0, 4.0})));

        REQUIRE(!builder.Build());
    }

    return 0;
}
