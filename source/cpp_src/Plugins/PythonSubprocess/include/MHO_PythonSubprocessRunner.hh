#ifndef MHO_PythonSubprocessRunner_HH__
#define MHO_PythonSubprocessRunner_HH__

#include <string>

namespace hops
{

/*!
 *@file  MHO_PythonSubprocessRunner.hh
 *@class MHO_PythonSubprocessRunner
 *@author J. Barrett - barrettj@mit.edu
 *@brief Pure-C++ (no pybind, no libpython) helper that runs the user's python3
 *       and exchanges JSON with it. This is the interprocess comms primitive used by the
 *       no-embed subprocess backends (control-file evaluation and default
 *       plotting). One process is spawned per action (no persistent worker!),
 *       which is slow and a lot of overhead, but this is ok for a fallback.
 *
 * Implementation note: we use popen() (a unidirectional pipe) and pass two temp
 * file paths on the child's command line -- the JSON request (which we write)
 * and an initially-empty response file the child writes its JSON answer into.
 * Reading the response from its own file rather than the shared stdout keeps
 * stray output from user control/plot code from corrupting the JSON contract.
 * The child's stdout is still drained (so a chatty child cannot block on a full
 * pipe) but is treated as diagnostics only. This also sidesteps the
 * bidirectional-pipe deadlock concerns of a hand-rolled fork/exec.
 *
 * Interpreter discovery: $HOPS_PYTHON if set, else "python3" on PATH.
 * Package discovery: the shipped pure-python hops_* packages are located via
 *   MHO_DirectoryInterface::GetHopsInstallPrefix() + the install-relative site
 *   subdir (baked in as HOPS_PYTHON_SITE_SUBDIR, matching hops_pypath.sh.in),
 *   and prepended to the child's PYTHONPATH so the default control/plot entry
 *   points are self-contained (no user pip install required).
 */

class MHO_PythonSubprocessRunner
{
    public:
        struct Result
        {
                bool spawned = false; //!< did popen() launch a shell/interpreter at all
                bool exited = false;  //!< did the child exit normally (vs. signalled)
                int exit_code = -1;   //!< child exit status when exited == true
                std::string out;      //!< the JSON response (read from the child's response file)
                std::string err;      //!< captured stderr (python traceback, if any)

                //!< convenience: launched, exited normally, with status 0
                bool Success() const { return spawned && exited && exit_code == 0; }
        };

        /**
         * @brief Resolve the python interpreter to use: $HOPS_PYTHON, else
         * "python3" (left for PATH resolution by the shell).
         */
        static std::string ResolveInterpreter();

        /**
         * @brief Install-prefix-relative directory holding the shipped hops_*
         * python packages, resolved at runtime against GetHopsInstallPrefix()
         * (or $HOPS_INSTALL). Empty if it cannot be determined.
         */
        static std::string ResolvePackageDir();

        /**
         * @brief Run `python -m <module> <request_file> <response_file>`, feeding
         * the JSON @p request_json via a temp file and reading the JSON response
         * back from a second temp file the child writes (stderr is captured too).
         * The child's PYTHONPATH is prepended with ResolvePackageDir() so the
         * shipped hops packages import cleanly.
         *
         * @param module       python module to run with -m (e.g. "hops_control").
         * @param request_json the JSON request payload written to the temp file.
         * @return Result with the response in .out + exit status. spawned==false
         *         means the interpreter could not be launched at all.
         */
        static Result RunModule(const std::string& module, const std::string& request_json);
};

} // namespace hops

#endif /*! end of include guard: MHO_PythonSubprocessRunner_HH__ */
