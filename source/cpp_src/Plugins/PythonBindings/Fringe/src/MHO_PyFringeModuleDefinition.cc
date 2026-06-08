#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "MHO_ControlEvaluatorSupport.hh"
#include "MHO_FringePass.hh"
#include "MHO_PyFringeDataInterface.hh"

using namespace hops;

#define PYBIND11_DETAILED_ERROR_MESSAGES

// ---------------------------------------------------------------------------
// Module-local helper: wrap a Python callable as a ControlEvaluatorFn.
//
// This mirrors MHO_PyControlEvaluator::EvaluateCallable but lives here so
// pyMHO_Fringe does not need to link against MHO_pyVisitors (which would
// create a fragile runtime dependency on an installed shared library). The
// pybind-free pass-info construction and condition/set-string post-processing
// are shared with the embedded and subprocess evaluators via
// MHO_ControlEvaluatorSupport.
// ---------------------------------------------------------------------------

static MHO_FringePass::ControlEvaluatorFn make_py_evaluator(py::object fn)
{
    return [fn](MHO_ParameterStore* ps, const mho_json& fmt, mho_json& stmts) -> bool {
        if(Py_IsInitialized() == 0)
        {
            return false;
        }
        try
        {
            py::gil_scoped_acquire gil;
            py::module hops_ctrl = py::module::import("hops_control");
            py::object PassInfo = hops_ctrl.attr("PassInfo");
            py::object Config = hops_ctrl.attr("Config");

            mho_json pd = MHO_ControlEvaluatorSupport::BuildPassInfoDict(ps);

            py::object pass_info = PassInfo(pd);
            py::object config = Config(fmt);

            fn(pass_info, config);

            stmts = config.attr("to_json")().cast< mho_json >();
        }
        catch(py::error_already_set& exc)
        {
            PyErr_Clear();
            return false;
        }

        MHO_ControlEvaluatorSupport::ApplyConditionFilterAndSetString(ps, stmts);
        return true;
    };
}

// ---------------------------------------------------------------------------
// Module definition
// ---------------------------------------------------------------------------

PYBIND11_MODULE(pyMHO_Fringe, m)
{
    m.doc() = "Python bindings for MHO_FringePass: drive fringe fitting entirely from Python";

    // Ensure pyMHO_Containers is loaded so MHO_PyFringeDataInterface is already registered
    py::module_::import("pyMHO_Containers");

    py::class_< MHO_FringePass >(m, "FringePass",
                                 R"doc(
        Encapsulates a single-baseline, single-pol-product fringe-fitting pass.

        Typical usage::

            import pyMHO_Fringe

            fp = pyMHO_Fringe.FringePass()
            fp.set_scan_directory("./1111/105-1800")
            fp.set_baseline("GE")
            fp.set_pol_product("XX")
            fp.set_frequency_group("X")
            fp.set_scan_name("105-1800")
            fp.set_configure(my_configure_fn)   # callable or control-file path

            if fp.initialize() and fp.configure():
                fp.run()

            data   = fp.get_fringe_data()
            plot   = data.get_plot_data()
            params = data.get_parameter_store()
        )doc")

        // --- input setters ---
        .def(py::init<>())
        .def("set_scan_directory", &MHO_FringePass::SetScanDirectory, py::arg("path"),
             "Path to the scan directory (contains .cor, .sta, etc.)")
        .def("set_baseline", &MHO_FringePass::SetBaseline, py::arg("baseline"), "Two-character baseline code, e.g. 'GE'")
        .def("set_pol_product", &MHO_FringePass::SetPolProduct, py::arg("polprod"),
             "Polarization product string, e.g. 'XX', 'RR', 'I'")
        .def("set_frequency_group", &MHO_FringePass::SetFrequencyGroup, py::arg("fgroup"),
             "Single-character frequency group code, e.g. 'X'")
        .def("set_scan_name", &MHO_FringePass::SetScanName, py::arg("scan"), "Scan name string, e.g. '105-1800'")
        .def("set_root_file", &MHO_FringePass::SetRootFile, py::arg("path"),
             "Root file basename (optional; discovered automatically if omitted)")
        .def("set_build_timestamp", &MHO_FringePass::SetBuildTimestamp, py::arg("ts"),
             "Build timestamp string written into plot output")
        .def("set_control_file", &MHO_FringePass::SetControlFile, py::arg("path"),
             "Override the control file path (DSL .cf or Python .py)")

        // --- configure: accepts either a callable or a control-file path ---
        .def(
            "set_configure",
            [](MHO_FringePass& self, py::object fn) {
                if(py::isinstance< py::str >(fn))
                {
                    // treat as a control-file path (DSL or .py); MHO_FringePass::Configure()
                    // handles both via extension detection
                    self.SetControlFile(fn.cast< std::string >());
                }
                else if(py::hasattr(fn, "__call__"))
                {
                    // wrap the callable and inject it as the control evaluator
                    self.SetPythonControlEvaluator(make_py_evaluator(fn));
                }
                else
                {
                    throw py::type_error("set_configure() requires a control-file path string or a callable configure(p, cfg)");
                }
            },
            py::arg("configure"),
            "Set the control configurator: a DSL/Python file path string, or a callable configure(p: PassInfo, cfg: Config)")

        // --- convenience setters for command-line-equivalent parameters ---
        .def(
            "set_output_directory",
            [](MHO_FringePass& self, const std::string& path) {
                self.GetFringeData()->GetParameterStore()->Set("/cmdline/output_directory", path);
            },
            py::arg("path"), "Directory for output .frng files (default: same as scan directory)")
        .def(
            "set_set_string",
            [](MHO_FringePass& self, const std::string& s) {
                self.GetFringeData()->GetParameterStore()->Set("/cmdline/set_string", s);
            },
            py::arg("s"), "Command-line 'set' override string (applied after control statements)")
        .def(
            "set_test_mode",
            [](MHO_FringePass& self, bool v) { self.GetFringeData()->GetParameterStore()->Set("/cmdline/test_mode", v); },
            py::arg("enable") = true, "When true, suppress writing .frng output files")

        // --- lifecycle ---
        .def("initialize", &MHO_FringePass::Initialize,
             "Open scan directory, load vex data, populate parameter store. "
             "Returns False if the scan directory or requested baseline is invalid.")
        .def("configure", &MHO_FringePass::Configure,
             "Evaluate the control file/callable and apply statements. "
             "Returns False on evaluation failure or if the pass is marked skipped.")
        .def(
            "run", [](MHO_FringePass& self) -> bool { return self.Run({}, {}, {}); },
            "Run the fringe fitter with no plugin/output/plot visitors. "
            "Access results via get_fringe_data() afterwards.")

        // --- status and results ---
        .def("is_skipped", &MHO_FringePass::IsSkipped, "True if a 'skip' control statement suppressed this pass.")
        .def(
            "get_fringe_data",
            [](MHO_FringePass& self) -> MHO_PyFringeDataInterface* {
                return new MHO_PyFringeDataInterface(self.GetFringeData());
            },
            py::return_value_policy::take_ownership, py::keep_alive< 1, 0 >(),
            "Return a view of the fringe data (parameter store, container store, plot data). "
            "Valid only for the lifetime of this FringePass.");
}
