// Unit tests for MHO_ParameterConfigurator: applies a single "parameter" control
// statement to the parameter store at /control/<parameter_type>/<name>, with the
// value decoded according to the format's "type". Station parameters are stored
// per-station-code (/control/station/<code>/<name>) using the condition tokens.

#include <string>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_StationIdentifier.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// format entry: {statement_type:"parameter", parameter_type:<ptype>, type:<vtype>}
static mho_json fmt_entry(const std::string& ptype, const std::string& vtype)
{
    mho_json e;
    e["statement_type"] = "parameter";
    e["parameter_type"] = ptype;
    e["type"] = vtype;
    return e;
}

// drive Configure() for one parameter statement
static bool run(MHO_ParameterConfigurator& cfg, const std::string& name, const mho_json& value, const mho_json& conditions)
{
    mho_json attr;
    attr["name"] = name;
    attr["statement_type"] = "parameter";
    attr["value"] = value;
    cfg.SetConditions(conditions);
    cfg.SetAttributes(attr);
    return cfg.Configure();
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // register a station so the station-parameter path maps deterministically
    MHO_StationIdentifier::GetInstance()->Insert("WESTFORD", "Wf", "E");

    // full format map covering every value type
    mho_json fmt;
    fmt["p_int"] = fmt_entry("config", "int");
    fmt["p_real"] = fmt_entry("config", "real");
    fmt["p_bool"] = fmt_entry("config", "bool");
    fmt["p_str"] = fmt_entry("config", "string");
    fmt["p_lint"] = fmt_entry("config", "list_int");
    fmt["p_lreal"] = fmt_entry("config", "list_real");
    fmt["p_lstr"] = fmt_entry("config", "list_string");
    fmt["p_comp"] = fmt_entry("config", "compound");
    fmt["p_bad"] = fmt_entry("config", "bogus_type");
    fmt["pc_mode"] = fmt_entry("station", "string");

    MHO_ParameterStore pstore;
    MHO_ParameterConfigurator cfg(&pstore, fmt);
    mho_json no_cond = mho_json::array();

    // scalar value types
    REQUIRE(run(cfg, "p_int", 42, no_cond));
    REQUIRE(pstore.GetAs< int >("/control/config/p_int") == 42);

    REQUIRE(run(cfg, "p_real", 3.14, no_cond));
    CHECK_CLOSE(pstore.GetAs< double >("/control/config/p_real"), 3.14, 1e-12);

    REQUIRE(run(cfg, "p_bool", true, no_cond));
    REQUIRE(pstore.GetAs< bool >("/control/config/p_bool") == true);

    REQUIRE(run(cfg, "p_str", std::string("hello"), no_cond));
    REQUIRE(pstore.GetAs< std::string >("/control/config/p_str") == "hello");

    // list value types
    REQUIRE(run(cfg, "p_lint", std::vector< int >{1, 2, 3}, no_cond));
    {
        std::vector< int > v;
        REQUIRE(pstore.Get("/control/config/p_lint", v));
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 1 && v[2] == 3);
    }

    REQUIRE(run(cfg, "p_lreal", std::vector< double >{1.5, 2.5}, no_cond));
    {
        std::vector< double > v;
        REQUIRE(pstore.Get("/control/config/p_lreal", v));
        REQUIRE(v.size() == 2);
        CHECK_CLOSE(v[1], 2.5, 1e-12);
    }

    REQUIRE(run(cfg, "p_lstr", std::vector< std::string >{"a", "b"}, no_cond));
    {
        std::vector< std::string > v;
        REQUIRE(pstore.Get("/control/config/p_lstr", v));
        REQUIRE(v.size() == 2);
        REQUIRE(v[0] == "a");
    }

    // compound value -> stored as a json object, accessible by sub-path
    {
        mho_json comp;
        comp["k"] = std::string("val");
        REQUIRE(run(cfg, "p_comp", comp, no_cond));
        REQUIRE(pstore.GetAs< std::string >("/control/config/p_comp/k") == "val");
    }

    // station parameter -> stored per station code from the "station Wf" condition
    {
        mho_json cond = mho_json::array();
        cond.push_back("station");
        cond.push_back("Wf");
        REQUIRE(run(cfg, "pc_mode", std::string("manual"), cond));
        REQUIRE(pstore.GetAs< std::string >("/control/station/Wf/pc_mode") == "manual");
    }

    // unknown value type -> Configure returns false
    REQUIRE(run(cfg, "p_bad", 0, no_cond) == false);

    // non-parameter statement -> no-op, returns true
    {
        mho_json attr;
        attr["name"] = "p_int";
        attr["statement_type"] = "operator";
        attr["value"] = 7;
        cfg.SetConditions(no_cond);
        cfg.SetAttributes(attr);
        REQUIRE(cfg.Configure() == true);
    }

    return 0;
}
