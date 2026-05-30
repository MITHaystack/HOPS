// Builder test for MHO_ManualChannelPhaseCorrectionBuilder: builds a per-channel
// phase-correction operator from a {channel_names, pc_phases} attribute map.
// Needs "vis" and a non-empty channel->phase map; reads fFormat["priority"].

#include <string>
#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_ManualChannelPhaseCorrection.hh"
#include "MHO_ManualChannelPhaseCorrectionBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static mho_json pcphase_attributes()
{
    mho_json attr;
    attr["name"] = "pc_phases_x";
    attr["value"]["channel_names"] = std::string("abcd");
    attr["value"]["pc_phases"] = std::vector< double >{10.0, 20.0, 30.0, 40.0};
    return attr;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // valid channel/phase map + vis present -> builds and registers the operator
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_ManualChannelPhaseCorrectionBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(pcphase_attributes());

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_ManualChannelPhaseCorrection >("pc_phases_x") != nullptr);
    }

    // valid map but no "vis" in the store -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;

        MHO_ManualChannelPhaseCorrectionBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(pcphase_attributes());

        REQUIRE(!builder.Build());
    }

    return 0;
}
