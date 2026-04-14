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
#include "MHO_BasicFringeUtilities.hh"

//for control
#include "MHO_ControlDefinitions.hh"
#include "MHO_FringeControlInitialization.hh"

//Python control-file support (only when pybind11 is available)
#ifdef USE_PYBIND11
    #include "MHO_PyControlEvaluator.hh"
    #include "MHO_PythonPluginInterface.hh"
#endif

//fringe finding
#include "MHO_FringeFitter.hh"
#include "MHO_FringeFitterFactory.hh"
#include "MHO_FringePlotVisitor.hh"

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

mho_json convert_profile_events(std::vector< MHO_ProfileEvent >& events)
{
    mho_json event_list;
    for(std::size_t i = 0; i < events.size(); i++)
    {
        mho_json obj;
        obj["event_id"] = i;
        obj["flag"] = events[i].fFlag;
        obj["line"] = events[i].fLineNumber;
        obj["thread_id"] = events[i].fThreadID;
        obj["filename"] = std::string(events[i].fFilename);
        obj["funcname"] = std::string(events[i].fFuncname);
        obj["time"] = events[i].fTime;
        event_list.push_back(obj);
    }
    return event_list;
}

void flush_profile_events(MHO_ParameterStore* paramStore)
{
    std::vector< MHO_ProfileEvent > events;
    MHO_Profiler::GetInstance().GetEvents(events);
    //convert and dump the events into the parameter store for now (will be empty unless enabled)
    mho_json event_list = convert_profile_events(events);
    paramStore->Set("/profile/events", event_list);
}






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

        //populate a few necessary parameters and  initialize the fringe/scan data
        MHO_FringeData fringeData;
        fringeData.GetParameterStore()->CopyFrom(cmdline_params); //copy in command line info

        //set the current pass info (directory, root_file, source, baseline, pol-product, frequency-group)
        mho_json pass = pass_vector[pass_index];
        pass["build_time"] = HOPS_BUILD_TIMESTAMP; //set the build time stamp in the pass info
        fringeData.GetParameterStore()->Set("/pass", pass);

        //initializes the scan data store, reads the ovex file and sets the value of '/pass/source'
        bool scan_dir_ok = MHO_FringeDataInitializer::initialize_scan_data(fringeData.GetParameterStore(),
                                                                                  fringeData.GetScanDataStore());
        if(!scan_dir_ok)
        {
            continue;
        }

        MHO_FringeDataInitializer::populate_initial_parameters(fringeData.GetParameterStore(),
                                                                      fringeData.GetScanDataStore());

        //parse the control file and form the control statements
        //detect whether the control file is a python script (.py extension)
        bool is_python_cf = false;
        std::string ctrl_file = fringeData.GetParameterStore()->GetAs< std::string >("/files/control_file");
        std::string ctrl_file_ext = MHO_DirectoryInterface::GetFileExtension(ctrl_file);
        if(ctrl_file_ext == "py"){is_python_cf = true;}

        if(is_python_cf)
        {
            #ifndef USE_PYBIND11
            if(is_python_cf) //bail out, pybind11 is not in use
            {
                msg_fatal("main", "a python control file was specified but HOPS was built without pybind11 support." << eom);
                std::exit(1);
            }
            #else

            fringeData.GetParameterStore()->Set("/status/is_finished", false);
            fringeData.GetParameterStore()->Set("/status/skipped", false);

            //ensure the Python interpreter is running before we call into it
            MHO_PythonPluginInterface::EnsureInitialized();

            //populate the control format (same setup as the DSL path)
            fringeData.GetControlFormat() = MHO_ControlDefinitions::GetControlFormat();
            MHO_FringeControlInitialization::add_default_operator_format_def(fringeData.GetControlFormat());

            //evaluate the Python control script -> control_statements
            bool py_ok = MHO_PyControlEvaluator::Evaluate(fringeData.GetParameterStore(),
                                                          fringeData.GetControlFormat(),
                                                          fringeData.GetControlStatements());
            if(!py_ok)
            {
                msg_error("main", "Python control file evaluation failed, skipping pass." << eom);
                fringeData.GetParameterStore()->Set("/status/skipped", true);
                fringeData.GetParameterStore()->Set("/status/is_finished", true);
                continue;
            }

            //consume the statements (add default operators, run ParameterManager, etc.)
            MHO_FringeControlInitialization::apply_control_statements(fringeData.GetParameterStore(),
                                                                      fringeData.GetControlFormat(),
                                                                      fringeData.GetControlStatements());

            //a python control file implicitly activates the Python plugin
            fringeData.GetParameterStore()->Set("/config/plugins/activate_python", true);

            #endif //USE_PYBIND11
        }
        else
        {
            //traditional control file processing
            MHO_FringeControlInitialization::process_control_file(fringeData.GetParameterStore(),
                                                                  fringeData.GetControlFormat(),
                                                                  fringeData.GetControlStatements());
        }

        //build the fringe fitter based on the input/control
        MHO_FringeFitterFactory ff_factory(&fringeData);
        MHO_FringeFitter* ffit = ff_factory.ConstructFringeFitter();

        //////////////////////////////////////////////////////////////
        // Plugin library (operator builder) registration (if any modules were built)
        plugin_factory.SetParameterStore(fringeData.GetParameterStore());
        plugin_factory.GetPluginVisitors(plugin_visitors);
        for(std::size_t np = 0; np < plugin_visitors.size(); np++)
        {
            ffit->Accept(plugin_visitors[np]);
        }

        //now (after plugin registration) we can configure the fringe fitter
        ffit->Configure();

        //initialize and perform run loop
        while(!ffit->IsFinished())
        {
            ffit->Initialize();
            ffit->PreRun();
            ffit->Run();
            ffit->PostRun();
        }
        ffit->Finalize();

        //determine if this pass was skipped or is in test-mode
        bool is_skipped = fringeData.GetParameterStore()->GetAs< bool >("/status/skipped");
        if(is_skipped)
        {
            continue;
        }

        //flush profile events
        profiler_stop();
        flush_profile_events(fringeData.GetParameterStore());

        //output visitors, write fringe data to file with variouis formats (as needed)
        bool test_mode = fringeData.GetParameterStore()->GetAs< bool >("/cmdline/test_mode");
        if(!test_mode)
        {
            plugin_factory.GetOutputVisitors(output_visitors);
            for(std::size_t np = 0; np < output_visitors.size(); np++)
            {
                ffit->Accept(output_visitors[np]);
            }
        }

        //use the plotter factory to construct one of the available plotting backends
        //whether or not the plot is displayed depends on the value of '/cmdline/show_plot'
        //but this logic is handled by the plot visitors themselves
        plugin_factory.GetPlotVisitors(plot_visitors);
        for(std::size_t np = 0; np < plot_visitors.size(); np++)
        {
            ffit->Accept(plot_visitors[np]);
        }
    } //end of pass loop

#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->GlobalBarrier();
    MHO_MPIInterface::GetInstance()->Finalize();
#endif

    return 0;
}
