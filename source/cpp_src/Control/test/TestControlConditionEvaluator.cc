#include <iostream>
#include <string>
#include <vector>

#include "MHO_ControlConditionEvaluator.hh"
#include "MHO_Message.hh"
#include "MHO_StationIdentifier.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Evaluate a condition built from a vector of token strings.
// We must materialize the mho_json into a named local because Evaluate()
// takes a non-const lvalue reference.
static bool eval_cond(MHO_ControlConditionEvaluator& ev, const std::vector< std::string >& value)
{
    mho_json c;
    c["statement_type"] = "conditional";
    c["line_number"] = 1;
    c["value"] = value;
    c["name"] = "if";
    return ev.Evaluate(c);
}

int main()
{
    // Suppress all messages so warnings from unregistered stations don't pollute output
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    MHO_ControlConditionEvaluator ev;
    ev.SetPassInformation("GE", "1234+567", "X", "288-210210");

    // CASE 1 - Literal true / false
    {
        REQUIRE(eval_cond(ev, {"true"}) == true);
        REQUIRE(eval_cond(ev, {"false"}) == false);
    }

    // CASE 2 - station predicate, 1-char mk4 id
    {
        REQUIRE(eval_cond(ev, {"station", "G"}) == true);
        REQUIRE(eval_cond(ev, {"station", "E"}) == true);
        REQUIRE(eval_cond(ev, {"station", "Z"}) == false);
        REQUIRE(eval_cond(ev, {"station", "?"}) == true);
    }

    // CASE 3 - baseline predicate, 2-char mk4 form
    {
        REQUIRE(eval_cond(ev, {"baseline", "GE"}) == true);
        REQUIRE(eval_cond(ev, {"baseline", "EG"}) == false);
        REQUIRE(eval_cond(ev, {"baseline", "?E"}) == true);
        REQUIRE(eval_cond(ev, {"baseline", "G?"}) == true);
        REQUIRE(eval_cond(ev, {"baseline", "??"}) == true);
        REQUIRE(eval_cond(ev, {"baseline", "ZE"}) == false);
    }

    // CASE 4 - source predicate
    {
        REQUIRE(eval_cond(ev, {"source", "1234+567"}) == true);
        REQUIRE(eval_cond(ev, {"source", "9999-999"}) == false);
        REQUIRE(eval_cond(ev, {"source", "?"}) == true);
    }

    // CASE 5 - f_group predicate
    {
        REQUIRE(eval_cond(ev, {"f_group", "X"}) == true);
        REQUIRE(eval_cond(ev, {"f_group", "S"}) == false);
        REQUIRE(eval_cond(ev, {"f_group", "?"}) == true);
    }

    // CASE 6 - scan predicate (single, <, >, range)
    {
        REQUIRE(eval_cond(ev, {"scan", "288-210210"}) == true);
        REQUIRE(eval_cond(ev, {"scan", "288-210211"}) == false);
        REQUIRE(eval_cond(ev, {"scan", "<", "288-210300"}) == true);
        REQUIRE(eval_cond(ev, {"scan", "<", "288-210100"}) == false);
        REQUIRE(eval_cond(ev, {"scan", ">", "288-210100"}) == true);
        REQUIRE(eval_cond(ev, {"scan", ">", "288-210300"}) == false);
        REQUIRE(eval_cond(ev, {"scan", "288-210000", "to", "288-220000"}) == true);
        REQUIRE(eval_cond(ev, {"scan", "288-210300", "to", "288-220000"}) == false);
    }

    // CASE 7 - NOT operator
    {
        REQUIRE(eval_cond(ev, {"not", "station", "Z"}) == true);  // not(false) -> true
        REQUIRE(eval_cond(ev, {"not", "station", "G"}) == false); // not(true)  -> false
    }

    // CASE 8 - AND
    {
        REQUIRE(eval_cond(ev, {"station", "G", "and", "station", "E"}) == true);
        REQUIRE(eval_cond(ev, {"station", "G", "and", "station", "Z"}) == false);
    }

    // CASE 9 - OR
    {
        REQUIRE(eval_cond(ev, {"station", "G", "or", "station", "Z"}) == true);
        REQUIRE(eval_cond(ev, {"station", "Y", "or", "station", "Z"}) == false);
    }

    // CASE 10 - Precedence NOT > AND > OR (no parens)
    {
        // false OR (true AND true) = true
        REQUIRE(eval_cond(ev, {"station", "Z", "or", "station", "G", "and", "station", "E"}) == true);
        // (not true) OR true = false OR true = true
        REQUIRE(eval_cond(ev, {"not", "station", "G", "or", "station", "E"}) == true);
    }

    // CASE 11 - Parentheses regrouping
    {
        // (false OR true) AND true = true
        REQUIRE(eval_cond(ev, {"(", "station", "Z", "or", "station", "G", ")", "and", "station", "E"}) == true);
        // (false OR true) AND false = false
        REQUIRE(eval_cond(ev, {"(", "station", "Z", "or", "station", "G", ")", "and", "station", "Y"}) == false);
    }

    // CASE 12 - Implicit AND of adjacent groups
    {
        REQUIRE(eval_cond(ev, {"station", "G", "station", "E"}) == true);
        REQUIRE(eval_cond(ev, {"station", "G", "station", "Z"}) == false);
    }

    // CASE 12a - Malformed baselines (EvaluateBaseline boundary)
    {
        // size < 2 is rejected outright
        REQUIRE(eval_cond(ev, {"baseline", "G"}) == false);
        // a 3+ char baseline with no "-" delimiter cannot be split -> false
        REQUIRE(eval_cond(ev, {"baseline", "GE_"}) == false);
    }

    // CASE 12b - Scan range endpoints are exclusive (strict < on both ends)
    {
        REQUIRE(eval_cond(ev, {"scan", "288-210210", "to", "288-220000"}) == false); // == low
        REQUIRE(eval_cond(ev, {"scan", "288-210000", "to", "288-210210"}) == false); // == high
    }

    // CASE 13 - GetApplicableStatements filters a full document
    {
        mho_json doc;
        mho_json c1;
        c1["statement_type"] = "conditional";
        c1["line_number"] = 1;
        c1["value"] = std::vector< std::string >{"true"};
        c1["name"] = "if";
        c1["statements"] = mho_json::array();
        c1["statements"].push_back("S0");

        mho_json c2;
        c2["statement_type"] = "conditional";
        c2["line_number"] = 1;
        c2["value"] = std::vector< std::string >{"station", "G"};
        c2["name"] = "if";
        c2["statements"] = mho_json::array();
        c2["statements"].push_back("S1");

        mho_json c3;
        c3["statement_type"] = "conditional";
        c3["line_number"] = 1;
        c3["value"] = std::vector< std::string >{"station", "Z"};
        c3["name"] = "if";
        c3["statements"] = mho_json::array();
        c3["statements"].push_back("S2");

        doc["conditions"] = mho_json::array();
        doc["conditions"].push_back(c1);
        doc["conditions"].push_back(c2);
        doc["conditions"].push_back(c3);

        mho_json out = ev.GetApplicableStatements(doc);
        REQUIRE(out.is_array());
        REQUIRE(out.size() == 2);
        REQUIRE(out[0]["value"].size() == 1);
        REQUIRE(out[0]["value"][0].get< std::string >() == "true");
        REQUIRE(out[1]["value"].size() == 2);
        REQUIRE(out[1]["value"][0].get< std::string >() == "station");
        REQUIRE(out[1]["value"][1].get< std::string >() == "G");
    }

    // CASE 13a - Malformed conditions now throw (formerly std::exit(1)).
    {
        // unrecognized token -> ERROR_STATE
        REQUIRE_THROWS(eval_cond(ev, {"bogus_token"}));
        // unmatched parentheses (close with no open, and open with no close)
        REQUIRE_THROWS(eval_cond(ev, {"true", ")"}));
        REQUIRE_THROWS(eval_cond(ev, {"(", "true"}));
        // 'not' with no operand
        REQUIRE_THROWS(eval_cond(ev, {"not"}));
        // 'and' missing first / second operand
        REQUIRE_THROWS(eval_cond(ev, {"and", "true"}));
        REQUIRE_THROWS(eval_cond(ev, {"true", "and"}));
        // 'or' missing first / second operand
        REQUIRE_THROWS(eval_cond(ev, {"or", "true"}));
        REQUIRE_THROWS(eval_cond(ev, {"true", "or"}));
    }

    // CASE 14 - Multi-character station via singleton (RUN LAST - singleton state)
    {
        MHO_StationIdentifier::GetInstance()->Insert("GGAO12M", "Gs", "G");
        MHO_StationIdentifier::GetInstance()->Insert("WESTFORD", "Wf", "E");

        MHO_ControlConditionEvaluator ev2;
        ev2.SetPassInformation("Gs-Wf", "1234+567", "X", "288-210210");

        REQUIRE(eval_cond(ev2, {"station", "Gs"}) == true);
        REQUIRE(eval_cond(ev2, {"baseline", "Gs-Wf"}) == true);
        REQUIRE(eval_cond(ev2, {"baseline", "?-Wf"}) == true);
        REQUIRE(eval_cond(ev2, {"station", "Ef"}) == false);
    }

    return 0;
}
