#include "MHO_PyControlEvaluator.hh"

#include <fstream>
#include <iterator>
#include <sstream>

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

    //execute the script and extract the configure() callable
    py::object configure_fn;
    try
    {
        py::dict script_ns;
        py::exec(script_code, py::globals(), script_ns);

        if(!script_ns.contains("configure"))
        {
            msg_error("python_control",
                      "Python control file '" << script_path << "' does not define a 'configure' function." << eom);
            return false;
        }
        configure_fn = script_ns["configure"];
    }
    catch(py::error_already_set& exc)
    {
        msg_error("python_control", "Python exception while loading control file '" << script_path << "':" << eom);
        msg_error("python_control", exc.what() << eom);
        PyErr_Clear();
        return false;
    }

    return EvaluateCallable(configure_fn, paramStore, control_format, control_statements);
}

bool MHO_PyControlEvaluator::EvaluateCallable(py::object fn, MHO_ParameterStore* paramStore, const mho_json& control_format,
                                              mho_json& control_statements)
{
    if(Py_IsInitialized() == 0)
    {
        msg_error("python_control", "Python interpreter is not running." << eom);
        return false;
    }

    try
    {
        py::gil_scoped_acquire gil;

        //import hops_control so PassInfo and Config are available
        py::module hops_ctrl = py::module::import("hops_control");

        py::object PassInfoClass = hops_ctrl.attr("PassInfo");
        py::object ConfigClass = hops_ctrl.attr("Config");

        mho_json pass_dict = MHO_ControlEvaluatorSupport::BuildPassInfoDict(paramStore);
        py::object pass_info = PassInfoClass(pass_dict);
        py::object config = ConfigClass(control_format);

        fn(pass_info, config);

        py::object result = config.attr("to_json")();
        control_statements = result.cast< mho_json >();
    }
    catch(py::error_already_set& exc)
    {
        msg_error("python_control", "Python exception in configure callable:" << eom);
        msg_error("python_control", exc.what() << eom);
        PyErr_Clear();
        return false;
    }

    MHO_ControlEvaluatorSupport::ApplyConditionFilterAndSetString(paramStore, control_statements);
    return true;
}

} // namespace hops
