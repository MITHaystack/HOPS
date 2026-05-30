// Unit tests for the protected helpers in MHO_OperatorBuilder (the base class
// shared by every operator builder): station-identifier extraction from control
// "if station X" conditions, matching those identifiers against the current
// baseline's ref/rem stations, and the default IsConfigurationOk() validation.

#include <string>
#include <vector>

#include "MHO_OperatorBuilder.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// minimal concrete builder that exposes the protected helpers for testing
class ExposedBuilder: public MHO_OperatorBuilder
{
    public:
        ExposedBuilder(): MHO_OperatorBuilder(nullptr) {}

        bool Build() override { return true; }

        std::vector< std::string > ExtractAll() const { return ExtractAllStationIdentifiers(); }
        bool MatchRole(const std::string& id, const std::string& role) const { return StationMatchesRole(id, role); }
        bool MatchBaseline(const std::string& id) const { return StationMatchesCurrentBaseline(id); }
        std::vector< std::string > Matching() const { return GetMatchingStationIdentifiers(); }
        bool ConfigOk() { return IsConfigurationOk(); }
};

// a baseline parameter store: ref = Gs (mk4 G), rem = Ef (mk4 E)
static void build_baseline_pstore(MHO_ParameterStore& p)
{
    p.Set("/ref_station/mk4id", std::string("G"));
    p.Set("/ref_station/site_id", std::string("Gs"));
    p.Set("/rem_station/mk4id", std::string("E"));
    p.Set("/rem_station/site_id", std::string("Ef"));
}

static mho_json tokens(std::vector< std::string > toks)
{
    mho_json c;
    c["value"] = toks;
    return c;
}

static bool vec_eq(const std::vector< std::string >& a, const std::vector< std::string >& b)
{
    if(a.size() != b.size())
    {
        return false;
    }
    for(std::size_t i = 0; i < a.size(); i++)
    {
        if(a[i] != b[i])
        {
            return false;
        }
    }
    return true;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    MHO_ParameterStore pstore;
    build_baseline_pstore(pstore);

    // Case 1: ExtractAllStationIdentifiers
    {
        ExposedBuilder b;
        // no conditions set -> wildcard
        REQUIRE(vec_eq(b.ExtractAll(), {"??"}));

        b.SetConditions(tokens({"station", "Gs"}));
        REQUIRE(vec_eq(b.ExtractAll(), {"Gs"}));

        b.SetConditions(tokens({"if", "station", "Gs", "or", "station", "Ef"}));
        REQUIRE(vec_eq(b.ExtractAll(), {"Gs", "Ef"}));

        // no "station" keyword present -> wildcard fallback
        b.SetConditions(tokens({"baseline", "GE"}));
        REQUIRE(vec_eq(b.ExtractAll(), {"??"}));
    }

    // Case 2: StationMatchesRole (mk4 id vs 2-char code vs wildcard)
    {
        ExposedBuilder b;
        b.SetParameterStore(&pstore);

        REQUIRE(b.MatchRole("G", "ref") == true);    // mk4 id matches ref
        REQUIRE(b.MatchRole("E", "ref") == false);   // mk4 id is the remote's
        REQUIRE(b.MatchRole("Gs", "ref") == true);   // 2-char code matches ref
        REQUIRE(b.MatchRole("Ef", "ref") == false);  // 2-char code is the remote's
        REQUIRE(b.MatchRole("E", "rem") == true);    // mk4 id matches rem
        REQUIRE(b.MatchRole("Ef", "rem") == true);   // 2-char code matches rem
        REQUIRE(b.MatchRole("??", "ref") == true);   // wildcards always match
        REQUIRE(b.MatchRole("?", "rem") == true);
    }

    // Case 3: StationMatchesRole with no parameter store -> always true (permissive)
    {
        ExposedBuilder b; // no pstore set
        REQUIRE(b.MatchRole("Zz", "ref") == true);
    }

    // Case 4: StationMatchesCurrentBaseline (ref OR rem)
    {
        ExposedBuilder b;
        b.SetParameterStore(&pstore);
        REQUIRE(b.MatchBaseline("G") == true);   // ref mk4
        REQUIRE(b.MatchBaseline("E") == true);   // rem mk4
        REQUIRE(b.MatchBaseline("Gs") == true);  // ref code
        REQUIRE(b.MatchBaseline("Ef") == true);  // rem code
        REQUIRE(b.MatchBaseline("X") == false);  // unknown mk4
        REQUIRE(b.MatchBaseline("Zz") == false); // unknown code
        REQUIRE(b.MatchBaseline("??") == true);  // wildcard
    }

    // Case 5: GetMatchingStationIdentifiers filters to stations on this baseline
    {
        ExposedBuilder b;
        b.SetParameterStore(&pstore);

        // single on-baseline station passes through
        b.SetConditions(tokens({"station", "Gs"}));
        REQUIRE(vec_eq(b.Matching(), {"Gs"}));

        // "Gs or Zz" -> Zz is not on this baseline, so it is filtered out
        b.SetConditions(tokens({"station", "Gs", "or", "station", "Zz"}));
        REQUIRE(vec_eq(b.Matching(), {"Gs"}));

        // neither station on this baseline -> wildcard fallback
        b.SetConditions(tokens({"station", "Zz", "or", "station", "Yy"}));
        REQUIRE(vec_eq(b.Matching(), {"??"}));

        // no station keyword -> wildcard (applies to all)
        b.SetConditions(tokens({"baseline", "GE"}));
        REQUIRE(vec_eq(b.Matching(), {"??"}));
    }

    // Case 6: IsConfigurationOk
    {
        ExposedBuilder b;

        // no format set -> default-ok
        REQUIRE(b.ConfigOk() == true);

        // a non-operator statement -> default-ok regardless of attributes
        mho_json param_fmt;
        param_fmt["statement_type"] = "parameter";
        b.SetFormat(param_fmt);
        REQUIRE(b.ConfigOk() == true);

        // compound operator with all required fields present -> ok
        mho_json op_fmt;
        op_fmt["statement_type"] = "operator";
        op_fmt["type"] = "compound";
        op_fmt["name"] = "foo";
        op_fmt["fields"] = std::vector< std::string >{"a", "b"};
        b.SetFormat(op_fmt);

        mho_json attr_ok;
        attr_ok["value"]["a"] = 1;
        attr_ok["value"]["b"] = 2;
        b.SetAttributes(attr_ok);
        REQUIRE(b.ConfigOk() == true);

        // a required field missing from the attributes -> not ok
        mho_json attr_bad;
        attr_bad["value"]["a"] = 1; // missing "b"
        b.SetAttributes(attr_bad);
        REQUIRE(b.ConfigOk() == false);
    }

    return 0;
}
