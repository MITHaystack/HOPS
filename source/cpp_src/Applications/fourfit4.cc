#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

//option parsing and help text library
#include "CLI11.hpp"

//basic infrastructure and messaging
#include "MHO_LockFileHandler.hh"
#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"
#include "MHO_Timer.hh"

//for data discovery/intialization
#include "MHO_FringeCommandLineParser.hh"
#include "MHO_FringeDataDiscovery.hh"
#include "MHO_FringeDataInitializer.hh"
#include "MHO_Mk4InputConverter.hh"

//Python control-file support (only when pybind11 is available)
#ifdef USE_PYBIND11
    #include "MHO_PyControlEvaluator.hh"
    #include "MHO_PythonPluginInterface.hh"
#endif

//single-pass encapsulation
#include "MHO_FringePass.hh"

//interface with plugin libraries, plot and output visitors
#include "MHO_PluginVisitorFactory.hh"

//wraps the MPI interface (in case it is not enabled)
#include "MHO_MPIInterfaceWrapper.hh"

//set build timestamp, for fourfit plots (legacy behavior)
#ifdef HOPS_BUILD_TIME
    #define HOPS_BUILD_TIMESTAMP STRING(HOPS_BUILD_TIME)
#else
    //no build time defined...default
    #define HOPS_BUILD_TIMESTAMP "2000-01-01T00:00:00.0Z"
#endif

using namespace hops;


int main(int argc, char** argv)
{
    //initialize the lock-file/signal-handler
    (void)MHO_LockFileHandler::GetInstance();

    int process_id = 0;
    int local_id = 0;
    int n_processes = 1;

#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->Initialize(&argc, &argv, true); //true -> run with no local even/odd split
    process_id = MHO_MPIInterface::GetInstance()->GetGlobalProcessID();
    local_id = MHO_MPIInterface::GetInstance()->GetLocalProcessID();
    n_processes = MHO_MPIInterface::GetInstance()->GetNProcesses();
#endif

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("fourfit"));

    MHO_ParameterStore cmdline_params;
    int parse_status = MHO_FringeCommandLineParser::parse_fourfit_command_line(argc, argv, &cmdline_params);
    if(parse_status != 0)
    {
        msg_fatal("main", "could not parse command line options." << eom);
        std::exit(1);
    }

    //flattened pass-info parameters (these are flattened into a single string primarily for MPI)
    std::string cscans, croots, cbaselines, cfgroups, cpolprods;

    static MK4TempDirGuard mk4_guard;
    MPI_SINGLE_PROCESS
    {
        bool mk4_input = cmdline_params.GetAs< bool >("/cmdline/mk4format_input");
        if(mk4_input)
        {
            //this is going to be slow, especially if we are trying to run on a whole experiment
            //the obvious solution/alternative is for the user to first run mark42hops
            //and then work with the hops4 files, not to convert mark4's on-the-fly in a temp directory
            //but we provide this ability for convenience
            mk4_guard.path = MHO_Mk4InputConverter::convert_mk4_input(&cmdline_params);
            if(mk4_guard.path.empty())
            {
                msg_fatal("main", "mark4 input conversion failed, exiting." << eom);
                std::exit(1);
            }
        }

        msg_debug("main", "determining the data passes" << eom);
        MHO_FringeDataDiscovery::determine_passes(&cmdline_params, cscans, croots, cbaselines, cfgroups, cpolprods);
        msg_debug("main", "done determining the data passes" << eom);
    }

//use MPI bcast to send all of the pass information to the worker processes
#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->BroadcastString(cscans);
    MHO_MPIInterface::GetInstance()->BroadcastString(croots);
    MHO_MPIInterface::GetInstance()->BroadcastString(cbaselines);
    MHO_MPIInterface::GetInstance()->BroadcastString(cfgroups);
    MHO_MPIInterface::GetInstance()->BroadcastString(cpolprods);
#endif

    std::vector< mho_json > pass_vector;
    MHO_FringeDataDiscovery::split_passes(pass_vector, cscans, croots, cbaselines, cfgroups, cpolprods);

    std::size_t n_pass = pass_vector.size();
    MPI_SINGLE_PROCESS
    {
        msg_info("main", "fourfit will fringe " << n_pass << " passes of data" << eom);
    }

    ////////////////////////////////////////
    //plugin/plot/output interface (manages plugin RAII and visitor creation)
    MHO_PluginVisitorFactory plugin_factory; //must remain persistent until end of main to call plugin 'finalize' routines
    std::vector< MHO_FringeFitterVisitor* > plugin_visitors;
    std::vector< MHO_FringePlotVisitor* > plot_visitors;
    std::vector< MHO_FringeFitterVisitor* > output_visitors;
    ///////////////////////////////////////

    //loop over passes belonging to this process, ensures (pass_index % n_processes == process_id)
    for(std::size_t pass_index = process_id; pass_index < n_pass; pass_index += n_processes)
    {
        profiler_start();

        mho_json pass_spec = pass_vector[pass_index];

        MHO_FringePass fpass;
        fpass.CopyCommandLineParams(cmdline_params);
        fpass.SetScanDirectory(pass_spec["input_directory"]);
        fpass.SetBaseline(pass_spec["baseline"]);
        fpass.SetPolProduct(pass_spec["polprod"]);
        fpass.SetFrequencyGroup(pass_spec["frequency_group"]);
        fpass.SetScanName(pass_spec["scan"]);
        fpass.SetRootFile(pass_spec["root_file"]);
        fpass.SetBuildTimestamp(HOPS_BUILD_TIMESTAMP);

        if(!fpass.Initialize())
        {
            continue;
        }

        // Inject the Python control evaluator before Configure() so that .py
        // control files are supported when pybind11 is available.
#ifdef USE_PYBIND11
        MHO_PythonPluginInterface::EnsureInitialized();
        fpass.SetPythonControlEvaluator(MHO_PyControlEvaluator::Evaluate);
#endif

        if(!fpass.Configure())
        {
            continue;
        }

        // Collect visitors: plugin visitors are registered on the fitter before
        // Configure(); output and plot visitors are dispatched after Finalize().
        plugin_factory.SetParameterStore(fpass.GetFringeData()->GetParameterStore());
        plugin_factory.GetPluginVisitors(plugin_visitors);

        bool test_mode = fpass.GetFringeData()->GetParameterStore()->GetAs< bool >("/cmdline/test_mode");
        if(!test_mode)
        {
            plugin_factory.GetOutputVisitors(output_visitors);
        }
        plugin_factory.GetPlotVisitors(plot_visitors);

        fpass.Run(plugin_visitors, output_visitors, plot_visitors);

        if(fpass.IsSkipped())
        {
            continue;
        }

        //flush profile events
        profiler_stop();
        fpass.FlushProfileEvents();
    } //end of pass loop

#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->GlobalBarrier();
    MHO_MPIInterface::GetInstance()->Finalize();
#endif

    return 0;
}
