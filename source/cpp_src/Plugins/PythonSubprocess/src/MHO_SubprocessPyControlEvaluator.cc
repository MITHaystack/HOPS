#include "MHO_SubprocessPyControlEvaluator.hh"

#include <fstream>

#include "MHO_ControlEvaluatorSupport.hh"
#include "MHO_Message.hh"
#include "MHO_PythonSubprocessContract.hh"
#include "MHO_PythonSubprocessRunner.hh"

namespace hops
{

bool MHO_SubprocessPyControlEvaluator::Evaluate(MHO_ParameterStore* paramStore, const mho_json& control_format,
                                                mho_json& control_statements)
{
    namespace pc = python_subprocess;

    std::string script_path = paramStore->GetAs< std::string >("/files/control_file");

    //fail early (and clearly) if the control file is unreadable, rather than
    //deep inside the python child
    {
        std::ifstream ifs(script_path.c_str());
        if(!ifs.is_open())
        {
            msg_error("python_control", "Cannot open Python control file: " << script_path << eom);
            return false;
        }
    }

    //build the request: pass-info dict is the SAME one the embedded path builds
    mho_json request;
    request[pc::kSchemaVersionKey] = pc::kSchemaVersion;
    request[pc::control::kPassInfoKey] = MHO_ControlEvaluatorSupport::BuildPassInfoDict(paramStore);
    request[pc::control::kConfigKey] = control_format;
    request[pc::control::kScriptPathKey] = script_path;

    MHO_PythonSubprocessRunner::Result result = MHO_PythonSubprocessRunner::RunModule(pc::control::kModule, request.dump());

    if(!result.spawned)
    {
        msg_error("python_control",
                  "Could not launch python subprocess to evaluate control file '" << script_path << "'." << eom);
        return false;
    }

    //parse the JSON response off the child's stdout
    mho_json response;
    try
    {
        response = mho_json::parse(result.out);
    }
    catch(const std::exception& e)
    {
        msg_error("python_control", "Failed to parse JSON response from control subprocess: " << e.what() << eom);
        if(!result.err.empty())
        {
            msg_error("python_control", "python stderr: " << result.err << eom);
        }
        return false;
    }

    bool ok = response.value(pc::kOkKey, false);
    if(!ok || !result.Success())
    {
        std::string err = response.value(pc::kErrorKey, std::string("unknown error"));
        msg_error("python_control", "Python control evaluation failed: " << err << eom);
        if(!result.err.empty())
        {
            msg_error("python_control", "python stderr: " << result.err << eom);
        }
        return false;
    }

    if(!response.contains(pc::control::kStatementsKey))
    {
        msg_error("python_control", "Control subprocess response is missing 'statements'." << eom);
        return false;
    }
    control_statements = response[pc::control::kStatementsKey];

    //condition filtering + command-line set-string overrides: SAME shared logic
    MHO_ControlEvaluatorSupport::ApplyConditionFilterAndSetString(paramStore, control_statements);
    return true;
}

} // namespace hops
