#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "MHO_ControlDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_SubprocessPyControlEvaluator.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// End-to-end test of the no-embed (subprocess) Python control evaluator. It
// drives the full path: BuildPassInfoDict -> JSON request -> `python -m
// hops_control` -> JSON response -> ApplyConditionFilterAndSetString. Because
// that path shares BuildPassInfoDict / ApplyConditionFilterAndSetString and the
// Python PassInfo/Config/configure machinery with the embedded evaluator, a
// correct result here also demonstrates parity with the embedded backend (only
// the transport differs).
//
// Requires an installed tree (the control-format JSON + the hops_control python
// package are resolved relative to $HOPS_INSTALL); the test SKIPs (exit 77)
// when the control format cannot be loaded so an un-installed build tree does
// not report a hard failure.

static std::string write_temp_py(const std::string& body, int tag)
{
    std::string path = "/tmp/test_subproc_ctrl_" + std::to_string(::getpid()) + "_" + std::to_string(tag) + ".py";
    std::ofstream(path) << body;
    return path;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eWarning);

    mho_json control_format = MHO_ControlDefinitions::GetControlFormat();
    if(control_format.empty())
    {
        std::cout << "[SKIP] control format unavailable; run 'make install' and set HOPS_INSTALL to run this test."
                  << std::endl;
        return 77; // ctest SKIP_RETURN_CODE
    }

    // CASE 1: a valid control script produces applicable statements
    {
        const std::string script = "from hops_control import PassInfo, Config\n"
                                   "def configure(p, cfg):\n"
                                   "    cfg.ref_freq(215000.0)\n";
        std::string path = write_temp_py(script, 1);

        MHO_ParameterStore ps;
        ps.Set("/files/control_file", path);
        ps.Set("/config/baseline", std::string("GE"));
        ps.Set("/config/fgroup", std::string("X"));
        ps.Set("/vex/scan/source/name", std::string("3C279"));
        ps.Set("/vex/scan/name", std::string("105-1800"));
        ps.Set("/config/polprod", std::string("XX"));
        ps.Set("/cmdline/set_string", std::string(""));

        mho_json statements;
        bool ok = MHO_SubprocessPyControlEvaluator::Evaluate(&ps, control_format, statements);

        REQUIRE(ok);
        REQUIRE(statements.is_array());
        REQUIRE(!statements.empty());
        // the unconditional ref_freq must survive condition filtering for this pass
        REQUIRE(statements.dump().find("ref_freq") != std::string::npos);

        std::remove(path.c_str());
    }

    // CASE 2: a script without a configure() function is reported as failure
    {
        const std::string script = "x = 1\n";
        std::string path = write_temp_py(script, 2);

        MHO_ParameterStore ps;
        ps.Set("/files/control_file", path);
        ps.Set("/config/baseline", std::string("GE"));
        ps.Set("/cmdline/set_string", std::string(""));

        mho_json statements;
        bool ok = MHO_SubprocessPyControlEvaluator::Evaluate(&ps, control_format, statements);
        REQUIRE(!ok);

        std::remove(path.c_str());
    }

    // CASE 3: a missing control file is reported as failure (before spawning)
    {
        MHO_ParameterStore ps;
        ps.Set("/files/control_file", std::string("/nonexistent/does_not_exist_subproc_ctrl.py"));
        ps.Set("/config/baseline", std::string("GE"));
        ps.Set("/cmdline/set_string", std::string(""));

        mho_json statements;
        bool ok = MHO_SubprocessPyControlEvaluator::Evaluate(&ps, control_format, statements);
        REQUIRE(!ok);
    }

    std::cout << "TestSubprocessPyControlEvaluator: all cases passed." << std::endl;
    return 0;
}
