// Builder test for MHO_PolProductSummationBuilder. Requires: parameter store
// with /config/polprod, /config/polprod_set, /ref_station/parallactic_angle,
// /rem_station/parallactic_angle; and container objects "vis", "weight",
// "ref_sta", "rem_sta".

#include <string>
#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_PolProductSummation.hh"
#include "MHO_PolProductSummationBuilder.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static void fill_pstore(MHO_ParameterStore& p)
{
    p.Set("/config/polprod", std::string("XX"));
    p.Set("/config/polprod_set", std::vector< std::string >{"XX", "YY", "XY", "YX"});
    p.Set("/ref_station/parallactic_angle", 0.1);
    p.Set("/rem_station/parallactic_angle", 0.2);
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // all parameters + containers present -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        fill_pstore(pstore);
        add_vis(store);
        add_weight(store);
        add_station(store, "ref_sta");
        add_station(store, "rem_sta");

        MHO_PolProductSummationBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pol_sum", true));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_PolProductSummation >("pol_sum") != nullptr);
    }

    // missing station coordinate data -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        fill_pstore(pstore);
        add_vis(store);
        add_weight(store);
        // no ref_sta / rem_sta

        MHO_PolProductSummationBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pol_sum", true));

        REQUIRE(!builder.Build());
    }

    // null parameter store -> guarded, Build fails (no crash)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);
        add_station(store, "ref_sta");
        add_station(store, "rem_sta");

        MHO_PolProductSummationBuilder builder(&toolbox, &store); // no parameter store
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("pol_sum", true));

        REQUIRE(!builder.Build());
    }

    return 0;
}
