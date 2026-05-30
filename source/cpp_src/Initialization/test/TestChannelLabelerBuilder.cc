// Builder test for MHO_ChannelLabelerBuilder: builds a channel-labeling operator
// for both "vis" and "weight" (registered as "chan_ids:vis"/"chan_ids:weight")
// from a {channel_names, channel_frequencies} attribute map. Requires both
// containers and fFormat["priority"].

#include <string>
#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_ChannelLabeler.hh"
#include "MHO_ChannelLabelerBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static mho_json channel_attributes()
{
    mho_json attr;
    attr["name"] = "chan_ids";
    attr["value"]["channel_names"] = std::string("abcd");
    attr["value"]["channel_frequencies"] = std::vector< double >{8000.0, 8016.0, 8032.0, 8048.0};
    return attr;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // vis + weight present -> builds both labelers, registered with :vis/:weight suffixes
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);

        MHO_ChannelLabelerBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(channel_attributes());

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_ChannelLabeler< visibility_type > >("chan_ids:vis") != nullptr);
        REQUIRE(toolbox.GetOperatorAs< MHO_ChannelLabeler< weight_type > >("chan_ids:weight") != nullptr);
    }

    // missing "weight" -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_ChannelLabelerBuilder builder(&toolbox, &store);
        builder.SetFormat(make_format());
        builder.SetAttributes(channel_attributes());

        REQUIRE(!builder.Build());
    }

    return 0;
}
