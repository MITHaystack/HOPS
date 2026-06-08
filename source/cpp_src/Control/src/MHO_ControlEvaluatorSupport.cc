#include "MHO_ControlEvaluatorSupport.hh"

#include "MHO_ControlFileParser.hh"

namespace hops
{

mho_json MHO_ControlEvaluatorSupport::BuildPassInfoDict(MHO_ParameterStore* paramStore)
{
    mho_json d;

    std::string baseline = paramStore->GetAs< std::string >("/config/baseline");
    d["baseline"] = baseline;

    //derive single-char Mk4 IDs from the baseline string
    if(baseline.size() == 2)
    {
        d["ref_mk4id"] = std::string(1, baseline[0]);
        d["rem_mk4id"] = std::string(1, baseline[1]);
        d["ref_code"] = std::string(1, baseline[0]);
        d["rem_code"] = std::string(1, baseline[1]);
    }
    else if(baseline.find('-') != std::string::npos)
    {
        //extract 2-char station codes from baseline string if passed like "Gs-Wf"
        std::size_t delim = baseline.find('-');
        d["ref_mk4id"] = baseline.substr(0, delim);
        d["rem_mk4id"] = baseline.substr(delim + 1);
        d["ref_code"] = baseline.substr(0, delim);
        d["rem_code"] = baseline.substr(delim + 1);
    }
    else
    {
        //wildcard
        d["ref_mk4id"] = "?";
        d["rem_mk4id"] = "?";
        d["ref_code"] = "??";
        d["rem_code"] = "??";
    }

    std::string source = "?";
    paramStore->Get("/vex/scan/source/name", source);
    d["source"] = source;

    std::string fgroup = "?";
    paramStore->Get("/config/fgroup", fgroup);
    d["fgroup"] = fgroup;

    std::string scan_name = "?";
    paramStore->Get("/vex/scan/name", scan_name);
    d["scan_name"] = scan_name;

    std::string polprod = "??";
    paramStore->Get("/config/polprod", polprod);
    d["polprod"] = polprod;

    return d;
}

void MHO_ControlEvaluatorSupport::ApplyConditionFilterAndSetString(MHO_ParameterStore* paramStore,
                                                                   mho_json& control_statements)
{
    std::string baseline = paramStore->GetAs< std::string >("/config/baseline");
    std::string source = "?";
    paramStore->Get("/vex/scan/source/name", source);
    std::string fgroup = "?";
    paramStore->Get("/config/fgroup", fgroup);
    std::string scan_name = "?";
    paramStore->Get("/vex/scan/name", scan_name);

    //GetApplicableStatements expects {"conditions": [...]} not a bare array
    mho_json wrapped;
    wrapped["conditions"] = control_statements;

    MHO_ControlConditionEvaluator condition_eval;
    condition_eval.SetPassInformation(baseline, source, fgroup, scan_name);
    control_statements = condition_eval.GetApplicableStatements(wrapped);

    //append any command-line 'set' overrides; placed last so they override Python-generated statements
    std::string set_string = paramStore->GetAs< std::string >("/cmdline/set_string");
    if(set_string != "")
    {
        msg_info("python_control", "Applying command-line 'set' overrides to Python control file output." << eom);
        MHO_ControlFileParser set_parser;
        set_parser.SetControlFile("/dev/null");
        set_parser.PassSetString(set_string);
        auto set_contents = set_parser.ParseControl();

        MHO_ControlConditionEvaluator set_eval;
        set_eval.SetPassInformation(baseline, source, fgroup, scan_name);
        mho_json set_statements = set_eval.GetApplicableStatements(set_contents);

        for(auto& block : set_statements)
        {
            control_statements.push_back(block);
        }
    }
}

} // namespace hops
