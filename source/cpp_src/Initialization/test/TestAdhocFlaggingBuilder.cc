// Builder test for MHO_AdhocFlaggingBuilder. The flag-file path is only stored
// at build time (read later at execute), so no real file is needed. Requires:
// format with operator_category; "weight" container; parameter store with
// ref/rem site_id. The operator is registered under the fixed name
// "adhoc_flagging".

#include <string>

#include "InitializationTestFixtures.hh"

#include "MHO_AdhocFlagging.hh"
#include "MHO_AdhocFlaggingBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static mho_json flag_format()
{
    mho_json f = make_format();
    f["operator_category"] = "flagging";
    return f;
}

static mho_json flag_attributes()
{
    mho_json a;
    a["name"] = "adhoc_flag";
    a["value"]["flag_file"] = std::string("/tmp/does_not_need_to_exist.flag");
    return a;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // weight + parameter store present -> builds, registered as "adhoc_flagging"
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/ref_station/site_id", std::string("Gs"));
        pstore.Set("/rem_station/site_id", std::string("Ef"));
        add_weight(store);

        MHO_AdhocFlaggingBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(flag_format());
        builder.SetAttributes(flag_attributes());

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_AdhocFlagging >("adhoc_flagging") != nullptr);
    }

    // missing "weight" -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/ref_station/site_id", std::string("Gs"));
        pstore.Set("/rem_station/site_id", std::string("Ef"));

        MHO_AdhocFlaggingBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(flag_format());
        builder.SetAttributes(flag_attributes());

        REQUIRE(!builder.Build());
    }

    // null parameter store -> guarded, Build fails (no crash)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_weight(store);

        MHO_AdhocFlaggingBuilder builder(&toolbox, &store); // no parameter store
        builder.SetFormat(flag_format());
        builder.SetAttributes(flag_attributes());

        REQUIRE(!builder.Build());
    }

    return 0;
}
