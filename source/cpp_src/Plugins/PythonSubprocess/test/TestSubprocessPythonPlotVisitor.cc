#include <cstdio>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "MHO_FringeData.hh"
#include "MHO_Message.hh"
#include "MHO_SubprocessPythonPlotVisitor.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// Behavior test for the no-embed (subprocess) default plotter. We do not have a
// captured plot_dict fixture (one requires running a full fringe fit), so this
// validates the DECIDED failure behavior + the C++ plumbing rather than a
// rendered image: on a render failure (here, an empty/garbage plot_dict, or a
// missing python/hops package) the visitor must WARN and SKIP -- it must not
// throw and must not leave a spurious output file. A skipped pass must be an
// immediate no-op. This holds in any environment (python present or not), so
// the test needs no SKIP path.

static bool file_exists(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eWarning);

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

    std::cout << "TestSubprocessPythonPlotVisitor: all cases passed." << std::endl;
    return 0;
}
