#include "MHO_PyControlEvaluator.hh"

#include <fstream>
#include <iterator>
#include <sstream>

#include "MHO_ControlFileParser.hh"

//pybind11 + JSON bridge
#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;
using namespace pybind11::literals;

namespace hops
{

bool MHO_PyControlEvaluator::Evaluate(MHO_ParameterStore* paramStore, const mho_json& control_format,
                                      mho_json& control_statements)
{
    if(Py_IsInitialized() == 0)
    {
        msg_error("python_control", "Python interpreter is not running. "
                                        << "Cannot evaluate Python control file." << eom);
        return false;
    }

    std::string script_path = paramStore->GetAs< std::string >("/files/control_file");

    //read the script text from disk
    std::ifstream ifs(script_path);
    if(!ifs.is_open())
    {
        msg_error("python_control", "Cannot open Python control file: " << script_path << eom);
        return false;
    }
    std::string script_code((std::istreambuf_iterator< char >(ifs)), std::istreambuf_iterator< char >());

    try
    {
        //import hops_control so PassInfo and Config are available
        py::module hops_ctrl = py::module::import("hops_control");

        //build python objects for pass info and config
        py::object PassInfoClass = hops_ctrl.attr("PassInfo");
        py::object ConfigClass   = hops_ctrl.attr("Config");

        mho_json pass_dict = BuildPassInfoDict(paramStore);
        py::object pass_info = PassInfoClass(pass_dict); //PassInfo(pass_dict)
        py::object config    = ConfigClass(control_format); //Config(format_dict)

        //execute the user's script in a fresh namespace that shares builtins
        py::dict script_ns;
        py::exec(script_code, py::globals(), script_ns);

        //retrieve and call configure(pass_info, config)
        if(!script_ns.contains("configure"))
        {
            msg_error("python_control",
                      "Python control file '" << script_path << "' does not define a 'configure' function." << eom);
            return false;
        }
        script_ns["configure"](pass_info, config);

        //extract the accumulated statements as mho_json
        py::object result = config.attr("to_json")();
        control_statements = result.cast< mho_json >();
    }
    catch(py::error_already_set& exc)
    {
        msg_error("python_control", "Python exception while evaluating control file '" << script_path << "':" << eom);
        msg_error("python_control", exc.what() << eom);
        PyErr_Clear();
        return false;
    }

    //filter blocks to only those applicable to the current pass,
    //mirroring what MHO_ControlFileParser does on the DSL path
    std::string baseline = paramStore->GetAs< std::string >("/config/baseline");
    std::string source   = "?";
    paramStore->Get("/vex/scan/source/name", source);
    std::string fgroup   = "?";
    paramStore->Get("/config/fgroup", fgroup);
    std::string scan_name = "?";
    paramStore->Get("/vex/scan/name", scan_name);

    //GetApplicableStatements expects {"conditions": [...]} not a bare array
    mho_json wrapped;
    wrapped["conditions"] = control_statements;

    MHO_ControlConditionEvaluator condition_eval;
    condition_eval.SetPassInformation(baseline, source, fgroup, scan_name);
    control_statements = condition_eval.GetApplicableStatements(wrapped);

    //append any command-line 'set' overrides, mirroring what process_control_file does on the DSL path.
    //the set-string blocks are placed after the Python-generated ones so they take effect last (i.e. they override).
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

    return true;
}

mho_json MHO_PyControlEvaluator::BuildPassInfoDict(MHO_ParameterStore* paramStore)
{
    mho_json d;

    std::string baseline = paramStore->GetAs< std::string >("/config/baseline");
    d["baseline"] = baseline;

    //derive single-char Mk4 IDs from the baseline string
    if(baseline.size() == 2)
    {
        d["ref_mk4id"] = std::string(1, baseline[0]);
        d["rem_mk4id"] = std::string(1, baseline[1]);
        d["ref_name"] = std::string(1, baseline[0]);
        d["rem_name"] = std::string(1, baseline[1]);
    }
    else if(baseline.find('-') != std::string::npos)
    {
        std::size_t delim = baseline.find('-');
        d["ref_mk4id"] = baseline.substr(0, delim);
        d["rem_mk4id"] = baseline.substr(delim + 1);
        d["ref_name"] = baseline.substr(0, delim);
        d["rem_name"] = baseline.substr(delim + 1);
    }
    else
    {
        d["ref_mk4id"]  = "?";
        d["rem_mk4id"]  = "?";
        d["ref_name"] = "?";
        d["rem_name"] = "?";
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

} // namespace hops
