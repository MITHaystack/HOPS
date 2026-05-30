// Builder test for MHO_AdhocPhaseCorrectionBuilder. value is {algorithm_type}
// (one of "sinewave"/"polynomial"/"file"); needs "vis" and a parameter store
// with ref/rem site_id. Mode-specific parameters are all optional (IsPresent-
// guarded), so each mode builds with the minimal fixture.

#include <string>

#include "InitializationTestFixtures.hh"

#include "MHO_AdhocPhaseCorrection.hh"
#include "MHO_AdhocPhaseCorrectionBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static mho_json algo_attributes(const std::string& name, const std::string& algorithm_type)
{
    mho_json a;
    a["name"] = name;
    a["value"]["algorithm_type"] = algorithm_type;
    return a;
}

// build with the given algorithm_type and report whether Build succeeded and the
// operator was registered under op_name
static bool build_mode(const std::string& algorithm_type)
{
    MHO_OperatorToolbox toolbox;
    MHO_ContainerStore store;
    MHO_ParameterStore pstore;
    pstore.Set("/ref_station/site_id", std::string("Gs"));
    pstore.Set("/rem_station/site_id", std::string("Ef"));
    add_vis(store);

    MHO_AdhocPhaseCorrectionBuilder builder(&toolbox, &store, &pstore);
    builder.SetFormat(make_format());
    builder.SetAttributes(algo_attributes("adhoc_phase", algorithm_type));

    return builder.Build() && (toolbox.GetOperatorAs< MHO_AdhocPhaseCorrection >("adhoc_phase") != nullptr);
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // each recognized algorithm mode builds and registers the operator
    REQUIRE(build_mode("sinewave"));
    REQUIRE(build_mode("polynomial"));
    REQUIRE(build_mode("file"));

    // unrecognized algorithm_type -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/ref_station/site_id", std::string("Gs"));
        pstore.Set("/rem_station/site_id", std::string("Ef"));
        add_vis(store);

        MHO_AdhocPhaseCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(algo_attributes("adhoc_phase", "bogus"));

        REQUIRE(!builder.Build());
    }

    // missing "vis" -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/ref_station/site_id", std::string("Gs"));
        pstore.Set("/rem_station/site_id", std::string("Ef"));

        MHO_AdhocPhaseCorrectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(algo_attributes("adhoc_phase", "sinewave"));

        REQUIRE(!builder.Build());
    }

    // null parameter store -> guarded, Build fails (no crash)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_AdhocPhaseCorrectionBuilder builder(&toolbox, &store); // no parameter store
        builder.SetFormat(make_format());
        builder.SetAttributes(algo_attributes("adhoc_phase", "sinewave"));

        REQUIRE(!builder.Build());
    }

    return 0;
}
