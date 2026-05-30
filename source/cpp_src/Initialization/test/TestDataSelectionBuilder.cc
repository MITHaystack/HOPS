// Unit tests for MHO_DataSelectionBuilder: builds a pair of MHO_SelectRepack
// operators ("coarse_selection:vis"/":weight") when any data-selection criterion
// (fgroup / polprod_set / channel / AP) is configured in the parameter store;
// otherwise it builds nothing and returns false.

#include <string>
#include <vector>

#include "InitializationTestFixtures.hh"

#include "MHO_DataSelectionBuilder.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_SelectRepack.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // pol-product selection configured -> builds the coarse-selection operators
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore;
        pstore.Set("/config/polprod", std::string("XX"));
        pstore.Set("/config/polprod_set", std::vector< std::string >{"XX"});
        add_vis(store);
        add_weight(store);

        MHO_DataSelectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("coarse_selection", true));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_SelectRepack< visibility_type > >("coarse_selection:vis") != nullptr);
        REQUIRE(toolbox.GetOperatorAs< MHO_SelectRepack< weight_type > >("coarse_selection:weight") != nullptr);
    }

    // no selection criteria configured -> nothing to do, Build returns false
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        MHO_ParameterStore pstore; // empty
        add_vis(store);
        add_weight(store);

        MHO_DataSelectionBuilder builder(&toolbox, &store, &pstore);
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("coarse_selection", true));

        REQUIRE(!builder.Build());
    }

    // null parameter store -> guarded, Build fails (no crash)
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);

        MHO_DataSelectionBuilder builder(&toolbox, &store); // no parameter store
        builder.SetFormat(make_format());
        builder.SetAttributes(make_attributes("coarse_selection", true));

        REQUIRE(!builder.Build());
    }

    return 0;
}
