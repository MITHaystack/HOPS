#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "MHO_FringeData.hh"
#include "MHO_Message.hh"
#include "MHO_PythonSubprocessContract.hh"
#include "MHO_PythonSubprocessRunner.hh"
#include "MHO_SubprocessPythonPlotVisitor.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Unit test for the no-embed (subprocess) default fringe plotter.
//
// Two flavors of test live here:
//
//  * Dependency-free behavior cases (always run): on a render failure (empty /
//    garbage plot_dict, or missing python/hops package) the visitor must WARN
//    and SKIP -- it must not throw and must not leave a spurious output file. A
//    skipped pass must be an immediate no-op. These hold in any environment, so
//    they need no SKIP path.
//
//  * Positive render case (runs only when a fixture path is given as argv[1]):
//    feed a real captured plot_dict (the fdump.json that
//    chk_simplefringesearch4.sh dumps under test_data/vt9105) through the full
//    subprocess path and assert a real image file is written. Because rendering
//    needs a working python3 + numpy/matplotlib + the shipped hops packages,
//    this case SKIPs (77) when those are unavailable; it only hard-fails on a
//    genuine contract/render error. The chk_subprocess_plot.sh wrapper generates
//    the fixture and passes its path here.

static bool file_exists(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0;
}

static bool file_nonempty(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && st.st_size > 0;
}

// stderr signatures that mean "this environment cannot render" (python/deps
// missing) rather than "the code is wrong" -> SKIP instead of FAIL.
static bool looks_like_missing_deps(const std::string& err)
{
    return err.find("ModuleNotFoundError") != std::string::npos || err.find("No module named") != std::string::npos ||
           err.find("ImportError") != std::string::npos;
}

// Returns 0 on success (positive case validated), 77 to SKIP, 1 on hard failure.
static int run_positive_case(const std::string& fixture_path)
{
    namespace pc = python_subprocess;

    if(!file_exists(fixture_path))
    {
        std::cout << "[SKIP] fixture not found: " << fixture_path << " (run chk_simplefringesearch4 first to dump fdump.json)"
                  << std::endl;
        return 77;
    }

    std::ifstream ifs(fixture_path.c_str(), std::ios::binary);
    std::string contents((std::istreambuf_iterator< char >(ifs)), std::istreambuf_iterator< char >());
    mho_json plot_dict;
    try
    {
        plot_dict = mho_json::parse(contents);
    }
    catch(const std::exception& e)
    {
        msg_error("test", "could not parse plot fixture " << fixture_path << ": " << e.what() << eom);
        return 1;
    }

    // First: drive the subprocess module directly so we can read the actual
    // ok/error/stderr and decide SKIP-vs-FAIL (the visitor deliberately swallows
    // failures). This also validates the JSON contract + that a real PNG lands.
    std::string probe_path = "/tmp/test_subproc_plot_probe_" + std::to_string(::getpid()) + ".png";
    std::remove(probe_path.c_str());

    mho_json request;
    request[pc::kSchemaVersionKey] = pc::kSchemaVersion;
    request[pc::plot::kPlotDictKey] = plot_dict;
    request[pc::plot::kShowPlotKey] = false;
    request[pc::plot::kDiskFileKey] = probe_path;

    MHO_PythonSubprocessRunner::Result result = MHO_PythonSubprocessRunner::RunModule(pc::plot::kModule, request.dump());

    if(!result.spawned)
    {
        std::cout << "[SKIP] could not launch python3 for plotting; skipping positive render case." << std::endl;
        return 77;
    }

    bool ok = false;
    std::string err_msg;
    try
    {
        mho_json response = mho_json::parse(result.out);
        ok = response.value(pc::kOkKey, false);
        if(!ok)
        {
            err_msg = response.value(pc::kErrorKey, std::string("unknown error"));
        }
    }
    catch(const std::exception& e)
    {
        err_msg = std::string("could not parse plot response: ") + e.what();
    }

    if(!ok || !result.Success())
    {
        // missing python deps (numpy/matplotlib/hops package) -> not a code bug
        if(looks_like_missing_deps(result.err) || looks_like_missing_deps(err_msg))
        {
            std::cout << "[SKIP] python plotting dependencies unavailable; skipping positive render case." << std::endl;
            std::remove(probe_path.c_str());
            return 77;
        }
        msg_error("test", "plot subprocess failed: " << err_msg << eom);
        if(!result.err.empty())
        {
            msg_error("test", "python stderr: " << result.err << eom);
        }
        std::remove(probe_path.c_str());
        return 1;
    }

    // the contract reported success -> a real, non-empty image must exist
    REQUIRE(file_nonempty(probe_path));
    std::remove(probe_path.c_str());

    // Now exercise the unit under test (the visitor) end-to-end with the same
    // fixture; the environment is known-capable, so it MUST produce its file.
    std::string out_path = "/tmp/test_subproc_plot_visitor_" + std::to_string(::getpid()) + ".png";
    std::remove(out_path.c_str());

    MHO_FringeData data;
    data.GetPlotData() = plot_dict;
    data.GetParameterStore()->Set("/status/skipped", false);
    data.GetParameterStore()->Set("/cmdline/show_plot", false);
    data.GetParameterStore()->Set("/cmdline/disk_file", out_path);

    MHO_SubprocessPythonPlotVisitor plotter;
    plotter.Plot(&data); // must return normally

    REQUIRE(file_nonempty(out_path));
    std::remove(out_path.c_str());

    std::cout << "TestSubprocessPythonPlotVisitor: positive render case passed." << std::endl;
    return 0;
}

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eWarning);

    std::cout << "TestSubprocessPythonPlotVisitor: CASE 1 & 2 are negative tests; a "
                 "'python plot rendering failed' WARNING below is EXPECTED."
              << std::endl;

    // CASE 1: invalid plot_dict -> python renderer errors -> warn + skip, no file
    {
        std::string out_path = "/tmp/test_subproc_plot_" + std::to_string(::getpid()) + ".png";
        std::remove(out_path.c_str());

        MHO_FringeData data;
        data.GetPlotData() = mho_json::object(); // missing all expected keys -> render fails
        data.GetParameterStore()->Set("/status/skipped", false);
        data.GetParameterStore()->Set("/cmdline/disk_file", out_path);

        MHO_SubprocessPythonPlotVisitor plotter;
        plotter.Plot(&data); // must return normally (no throw)

        REQUIRE(!file_exists(out_path)); // failed render must not produce an output file
        std::remove(out_path.c_str());
    }

    // CASE 2: a skipped pass is an immediate no-op (no spawn, no file)
    {
        std::string out_path = "/tmp/test_subproc_plot_skip_" + std::to_string(::getpid()) + ".png";
        std::remove(out_path.c_str());

        MHO_FringeData data;
        data.GetPlotData() = mho_json::object();
        data.GetParameterStore()->Set("/status/skipped", true);
        data.GetParameterStore()->Set("/cmdline/disk_file", out_path);

        MHO_SubprocessPythonPlotVisitor plotter;
        plotter.Plot(&data);

        REQUIRE(!file_exists(out_path));
        std::remove(out_path.c_str());
    }

    std::cout << "TestSubprocessPythonPlotVisitor: behavior cases passed." << std::endl;

    // CASE 3 (positive render): only when a captured plot_dict fixture is supplied.
    if(argc > 1)
    {
        return run_positive_case(std::string(argv[1]));
    }

    return 0;
}
