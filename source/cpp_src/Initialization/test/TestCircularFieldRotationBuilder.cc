// Builder test for MHO_CircularFieldRotationBuilder. Requires: parameter store
// with /config/polprod_set, /vex/scan/fourfit_reftime, /ref_station/site_id,
// /rem_station/site_id, and an antenna mount_type; container objects "vis",
// "ref_sta", "rem_sta".

#include <string>
#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_CircularFieldRotationCorrection.hh"
#include "MHO_CircularFieldRotationBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static void fill_pstore(MHO_ParameterStore& p, bool with_mount)
{
    p.Set("/config/polprod_set", std::vector< std::string >{"RR", "LL"});
    p.Set("/vex/scan/fourfit_reftime", std::string("2024y001d00h00m00s"));
    p.Set("/ref_station/site_id", std::string("Gs"));
    p.Set("/rem_station/site_id", std::string("Ef"));
    if(with_mount)
    {
        p.Set("/control/station/mount_type", std::string("ALTAZ"));
    }
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // all parameters + containers present -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        fill_pstore(pstore, true);
        add_vis(store);
        add_station(store, "ref_sta");
        add_station(store, "rem_sta");

        MHO_CircularFieldRotationBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("field_rotation", true));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_CircularFieldRotationCorrection >("field_rotation") != nullptr);
    }

    // no antenna mount_type info -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        fill_pstore(pstore, false); // no mount_type
        add_vis(store);
        add_station(store, "ref_sta");
        add_station(store, "rem_sta");

        MHO_CircularFieldRotationBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("field_rotation", true));

        REQUIRE(!builder.Build());
    }

    // null parameter store -> guarded, Build fails (no crash)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_station(store, "ref_sta");
        add_station(store, "rem_sta");

        MHO_CircularFieldRotationBuilder builder(&toolbox, &store); // no parameter store
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("field_rotation", true));

        REQUIRE(!builder.Build());
    }

    return 0;
}
