#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

//option parsing and help text library
#include "CLI11.hpp"

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"
#include "MHO_Timer.hh"

//fringe finding
#include "MHO_FringeFitter.hh"
#include "MHO_FringePlotVisitor.hh"
#include "MHO_FringeFitterFactory.hh"
#include "MHO_FringePlotVisitorFactory.hh"

//for control intialization
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_FringeControlInitialization.hh"
#include "MHO_LockFileHandler.hh"

//interface with plugin libraries
#include "MHO_PluginVisitorFactory.hh"

//needed to export to mark4 fringe files
#include "MHO_MK4FringeExport.hh"

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
    (void) MHO_LockFileHandler::GetInstance();

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
    int parse_status = MHO_BasicFringeDataConfiguration::parse_fourfit_command_line(argc, argv, &cmdline_params);
    if(parse_status != 0)
    {
        msg_fatal("main", "could not parse command line options." << eom);
        std::exit(1);
    }

    //flattened pass-info parameters (these are flattened into a single string primarily for MPI)
    std::string cscans, croots, cbaselines, cfgroups, cpolprods;

    MPI_SINGLE_PROCESS
    {
        msg_debug("main", "determining the data passes" << eom);
        MHO_BasicFringeDataConfiguration::determine_passes(&cmdline_params, cscans, croots, cbaselines, cfgroups, cpolprods);
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
    MHO_BasicFringeDataConfiguration::split_passes(pass_vector, cscans, croots, cbaselines, cfgroups, cpolprods);

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

    //this loop could be trivially parallelized (with the exception of plotting)
    for(std::size_t pass_index = 0; pass_index < n_pass; pass_index++)
    {
        if(pass_index % n_processes == process_id)
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
            bool scan_dir_ok = MHO_BasicFringeDataConfiguration::initialize_scan_data(fringeData.GetParameterStore(), fringeData.GetScanDataStore());

            MHO_BasicFringeDataConfiguration::populate_initial_parameters(fringeData.GetParameterStore(), fringeData.GetScanDataStore());

            //parse the control file and form the control statements
            MHO_FringeControlInitialization::process_control_file(fringeData.GetParameterStore(), fringeData.GetControlFormat(), fringeData.GetControlStatements());

            //build the fringe fitter based on the input/control
            MHO_FringeFitterFactory ff_factory(&fringeData);
            MHO_FringeFitter* ffit = ff_factory.ConstructFringeFitter();

            //////////////////////////////////////////////////////////////
            // Plugin library (operator builder) registration (if any modules were built)
            plugin_factory.SetParameterStore(fringeData.GetParameterStore());
            plugin_factory.GetPluginVisitors(plugin_visitors);
            for(std::size_t np=0; np<plugin_visitors.size(); np++)
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

            //flush profile events
            profiler_stop();
            std::vector< MHO_ProfileEvent > events;
            MHO_Profiler::GetInstance().GetEvents(events);

            //convert and dump the events into the parameter store for now (will be empty unless enabled)
            mho_json event_list = MHO_BasicFringeDataConfiguration::ConvertProfileEvents(events);
            fringeData.GetParameterStore()->Set("/profile/events", event_list);

            //determine if this pass was skipped or is in test-mode
            bool is_skipped = fringeData.GetParameterStore()->GetAs< bool >("/status/skipped");
            bool test_mode = fringeData.GetParameterStore()->GetAs< bool >("/cmdline/test_mode");

            if(!is_skipped)
            {
                // //output visitors 
                // if(!test_mode)
                // {
                //     plugin_factory.GetOutputVisitors(output_visitors);
                //     for(std::size_t np=0; np<output_visitors.size(); np++)
                //     {
                //         ffit->Accept(output_visitors[np]);
                //     }
                // }

                //OUTPUT
                //open and dump to file -- should we profile this as well?
                if(!test_mode)
                {
                    bool use_mk4_output = false;
                    fringeData.GetParameterStore()->Get("/cmdline/mk4format_output", use_mk4_output);
                
                    if(!use_mk4_output)
                    {
                        fringeData.WriteOutput();
                    }
                    else
                    {
                        MHO_MK4FringeExport fexporter;
                        fexporter.SetParameterStore(fringeData.GetParameterStore());
                        fexporter.SetPlotData(fringeData.GetPlotData());
                        fexporter.SetContainerStore(fringeData.GetContainerStore());
                        fexporter.ExportFringeFile();
                    }
                }

                //use the plotter factory to construct one of the available plotting backends
                //whether or not the plot is displayed depends on the value of '/cmdline/show_plot' 
                //but this logic is handled by the plot visitors themselves
                plugin_factory.GetPlotVisitors(plot_visitors);
                for(std::size_t np=0; np<plot_visitors.size(); np++)
                {
                    ffit->Accept(plot_visitors[np]);
                }
            }
        }
    } //end of pass loop

#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->GlobalBarrier();
    MHO_MPIInterface::GetInstance()->Finalize();
#endif

    // MHO_Profiler::GetInstance().DumpEvents();

    return 0;
}
