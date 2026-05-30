// Builder test for MHO_SamplerLabelerBuilder. Requires: "vis" container and a
// parameter store with ref/rem site_id and per-station (or generic) sampler
// channel-set info; bails out (Build fails) when no sampler info is present.

#include <string>
#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_SamplerLabeler.hh"
#include "MHO_SamplerLabelerBuilder.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // sampler info present -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/ref_station/site_id", std::string("Gs"));
        pstore.Set("/rem_station/site_id", std::string("Ef"));
        pstore.Set("/control/station/samplers", std::vector< std::string >{"abcd"});
        add_vis(store);

        MHO_SamplerLabelerBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("sampler_label", true));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_SamplerLabeler< visibility_type > >("sampler_label") != nullptr);
    }

    // no sampler info anywhere -> Build fails (nothing to label)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/ref_station/site_id", std::string("Gs"));
        pstore.Set("/rem_station/site_id", std::string("Ef"));
        add_vis(store);

        MHO_SamplerLabelerBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("sampler_label", true));

        REQUIRE(!builder.Build());
    }

    // null parameter store -> guarded, Build fails (no crash)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_SamplerLabelerBuilder builder(&toolbox, &store); // no parameter store
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("sampler_label", true));

        REQUIRE(!builder.Build());
    }

    return 0;
}
