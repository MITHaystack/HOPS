// Builder test for MHO_NotchCombBuilder: value is {offset, period, width} (with
// width <= period); op_category comes from fFormat["operator_category"]; needs
// both "vis" and "weight".

#include "InitializationTestFixtures.hh"

#include "MHO_Message.hh"
#include "MHO_NotchComb.hh"
#include "MHO_NotchCombBuilder.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;
using namespace hops::inittest;

static mho_json comb_format()
{
    mho_json f = make_format();
    f["operator_category"] = "flagging";
    return f;
}

static mho_json comb_value(double offset, double period, double width)
{
    mho_json v;
    v["offset"] = offset;
    v["period"] = period;
    v["width"] = width;
    return v;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // valid comb, vis+weight present -> builds and registers
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);

        MHO_NotchCombBuilder builder(&toolbox, &store);
        builder.SetFormat(comb_format());
        builder.SetAttributes(make_attributes("notch_comb", comb_value(0.0, 10.0, 2.0)));

        REQUIRE(builder.Build());
        REQUIRE(toolbox.GetOperatorAs< MHO_NotchComb >("notch_comb") != nullptr);
    }

    // width > period -> rejected
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);
        add_weight(store);

        MHO_NotchCombBuilder builder(&toolbox, &store);
        builder.SetFormat(comb_format());
        builder.SetAttributes(make_attributes("notch_comb", comb_value(0.0, 5.0, 20.0)));

        REQUIRE(!builder.Build());
    }

    // missing "weight" -> Build fails
    {
        MHO_OperatorToolbox toolbox;
        MHO_ContainerStore store;
        add_vis(store);

        MHO_NotchCombBuilder builder(&toolbox, &store);
        builder.SetFormat(comb_format());
        builder.SetAttributes(make_attributes("notch_comb", comb_value(0.0, 10.0, 2.0)));

        REQUIRE(!builder.Build());
    }

    return 0;
}
