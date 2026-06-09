#ifndef MHO_PythonSubprocessContract_HH__
#define MHO_PythonSubprocessContract_HH__

#include <string>

namespace hops
{
namespace python_subprocess
{

/*!
 *@file  MHO_PythonSubprocessContract.hh
 *@author J. Barrett - barrettj@mit.edu
 *@brief Versioned JSON contract shared by the C++ subprocess backends and the
 *       Python entry points they spawn. This is the stability surface that
 *       replaces the compiled-module ABI for the no-embed (subprocess) path:
 *       the C++ and Python halves agree on these field names and on a
 *       schema_version so they can evolve across the supported CPython minors.
 *
 *  Control evaluation (module: hops_control)
 *  -----------------------------------------
 *    request  : { "schema_version": <int>,
 *                 "pass_info":   <dict>,    // MHO_ControlEvaluatorSupport::BuildPassInfoDict
 *                 "config":      <dict>,    // canonical control-format dict
 *                 "script_path": <string> } // path to the user's .py control file
 *    response : { "schema_version": <int>,
 *                 "ok":         <bool>,
 *                 "statements": <array>,    // raw config.to_json() (pre condition-filter)
 *                 "error":      <string> }  // present iff ok == false
 *
 *  Plot rendering (module: hops_visualization.fourfit_plot)
 *  --------------------------------------------------------
 *    request  : { "schema_version": <int>,
 *                 "plot_dict": <dict>,      // MHO_FringeData::GetPlotData()
 *                 "show_plot": <bool>,      // optional (/cmdline/show_plot)
 *                 "disk_file": <string> }   // optional output filename (/cmdline/disk_file)
 *    response : { "schema_version": <int>,
 *                 "ok":          <bool>,
 *                 "output_file": <string>,  // path actually written (if any)
 *                 "error":       <string> } // present iff ok == false
 */

//bump when the request/response field layout changes incompatibly
static const int kSchemaVersion = 1;

//common field names
static const char* const kSchemaVersionKey = "schema_version";
static const char* const kOkKey = "ok";
static const char* const kErrorKey = "error";

//control request/response
namespace control
{
static const char* const kModule = "hops_control"; // python -m hops_control
static const char* const kPassInfoKey = "pass_info";
static const char* const kConfigKey = "config";
static const char* const kScriptPathKey = "script_path";
static const char* const kStatementsKey = "statements";
} // namespace control

//plot request/response
namespace plot
{
static const char* const kModule = "hops_visualization.fourfit_plot";
static const char* const kPlotDictKey = "plot_dict";
static const char* const kShowPlotKey = "show_plot";
static const char* const kDiskFileKey = "disk_file";
static const char* const kOutputFileKey = "output_file";
} // namespace plot

} // namespace python_subprocess

} // namespace hops

#endif /*! end of include guard: MHO_PythonSubprocessContract_HH__ */
