// Builder test for MHO_MultitonePhaseCorrectionBuilder. The operator is only
// built for op_name "ref_multitone_pcal"/"rem_multitone_pcal", when the
// station's pc_mode is "multitone" (the default), and when the matching
// multitone pcal object ("ref_pcal"/"rem_pcal") and "vis" are present.

#include "InitializationTestFixtures.hh"

#include "MHO_Message.hh"
#include "MHO_MultitonePhaseCorrection.hh"
#include "MHO_MultitonePhaseCorrectionBuilder.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static void fill_pstore(MHO_ParameterStore& p)
{
    p.Set("/ref_station/site_id", std::string("Gs"));
    p.Set("/rem_station/site_id", std::string("Ef"));
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // ref pcal + vis present, default pc_mode "multitone" -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        fill_pstore(pstore);
        add_vis(store);
        add_weight(store);
        add_pcal(store, "ref_pcal");

        MHO_MultitonePhaseCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("ref_multitone_pcal", true));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_MultitonePhaseCorrection >("ref_multitone_pcal") != nullptr);
    }

    // pc_mode forced to "manual" -> not a multitone station, Build returns false
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        fill_pstore(pstore);
        pstore.Set("/control/station/pc_mode", std::string("manual"));
        add_vis(store);
        add_pcal(store, "ref_pcal");

        MHO_MultitonePhaseCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("ref_multitone_pcal", true));

        REQUIRE(!builder.Build());
    }

    // no pcal data for the station -> Build returns false (not necessarily an error)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        fill_pstore(pstore);
        add_vis(store);
        // no "ref_pcal"

        MHO_MultitonePhaseCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("ref_multitone_pcal", true));

        REQUIRE(!builder.Build());
    }

    // null parameter store -> guarded, Build fails (no crash)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_pcal(store, "ref_pcal");

        MHO_MultitonePhaseCorrectionBuilder builder(&toolbox, &store); // no parameter store
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("ref_multitone_pcal", true));

        REQUIRE(!builder.Build());
    }

    return 0;
}
