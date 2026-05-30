// Unit tests for MHO_ParameterManager::ConfigureAll: walks the control statements,
// applies every "parameter" statement to the parameter store (via
// MHO_ParameterConfigurator) and erases it, leaving non-parameter (e.g. operator)
// statements in place.

#include <string>

#include "MHO_Message.hh"
#include "MHO_ParameterManager.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static mho_json fmt_entry(const std::string& stmt_type, const std::string& ptype, const std::string& vtype)
{
    mho_json e;
    e["statement_type"] = stmt_type;
    if(!ptype.empty())
    {
        e["parameter_type"] = ptype;
    }
    if(!vtype.empty())
    {
        e["type"] = vtype;
    }
    return e;
}

static mho_json statement(const std::string& name, const std::string& stmt_type, const mho_json& value)
{
    mho_json s;
    s["name"] = name;
    s["statement_type"] = stmt_type;
    s["value"] = value;
    return s;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // format: two parameters and one operator
    mho_json fmt;
    fmt["p_int"] = fmt_entry("parameter", "config", "int");
    fmt["p_str"] = fmt_entry("parameter", "config", "string");
    fmt["some_op"] = fmt_entry("operator", "", "");

    // one control block with a condition value, two parameter statements and one operator statement
    mho_json control = mho_json::array();
    mho_json block;
    block["value"] = mho_json::array(); // no conditions
    block["statements"] = mho_json::array();
    block["statements"].push_back(statement("p_int", "parameter", 42));
    block["statements"].push_back(statement("p_str", "parameter", std::string("hi")));
    block["statements"].push_back(statement("some_op", "operator", true));
    control.push_back(block);

    MHO_ParameterStore pstore;
    MHO_ParameterManager pm(&pstore, fmt);
    pm.SetControlStatements(&control);
    pm.ConfigureAll();

    // the parameter statements were applied to the store
    REQUIRE(pstore.GetAs< int >("/control/config/p_int") == 42);
    REQUIRE(pstore.GetAs< std::string >("/control/config/p_str") == "hi");

    // the parameter statements were consumed (erased); only the operator statement remains
    REQUIRE(control[0]["statements"].size() == 1);
    REQUIRE(control[0]["statements"][0]["name"].get< std::string >() == "some_op");

    return 0;
}
