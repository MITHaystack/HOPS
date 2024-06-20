#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <getopt.h>

//option parsing and help text library
#include "CLI11.hpp"

#define EXTRA_DEBUG

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"
#include "MHO_Timer.hh"

//fringe finding library helper functions
#include "MHO_BasicFringeFitter.hh"
#include "MHO_IonosphericFringeFitter.hh"

//for control intialization
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_FringeControlInitialization.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include <pybind11/pybind11.h>
    #include <pybind11/embed.h>
    #include "pybind11_json/pybind11_json.hpp"
    namespace py = pybind11;
    namespace nl = nlohmann;
    using namespace pybind11::literals;
    #include "MHO_PythonOperatorBuilder.hh"
    #include "MHO_PyConfigurePath.hh"
#endif

//needed to export to mark4 fringe files
#include "MHO_MK4FringeExport.hh"


#include "MHO_MPIInterfaceWrapper.hh"


using namespace hops;


int main(int argc, char** argv)
{
    int process_id = 0;
    int local_id = 0;
    int n_processes = 1;

    #ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->Initialize(&argc, &argv, true); //true -> run with no local even/odd split
    process_id = MHO_MPIInterface::GetInstance()->GetGlobalProcessID();
    local_id = MHO_MPIInterface::GetInstance()->GetLocalProcessID();
    n_processes = MHO_MPIInterface::GetInstance()->GetNProcesses();
    #endif

    #ifdef USE_PYBIND11
    //start the interpreter and keep it alive, need this or we segfault
    //each process has its own interpreter
    py::scoped_interpreter guard{};
    configure_pypath();
    #endif

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("fourfit"));

    MHO_ParameterStore cmdline_params;
    int parse_status = MHO_BasicFringeDataConfiguration::parse_fourfit_command_line(argc, argv, &cmdline_params );
    if(parse_status != 0){msg_fatal("main", "could not parse command line options." << eom); std::exit(1);}

    //flattened pass-info parameters (these are flattened into a single string primarily for MPI)
    std::string concat_delim = ",";
    std::string cscans;
    std::string croots;
    std::string cbaselines;
    std::string cfgroups;
    std::string cpolprods;

    MPI_SINGLE_PROCESS
    {
        MHO_BasicFringeDataConfiguration::determine_passes(&cmdline_params, cscans, croots, cbaselines, cfgroups, cpolprods);
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
    MPI_SINGLE_PROCESS{ msg_info("main", "fourfit will fringe "<<n_pass<<" passes of data" << eom); }

    //this loop could be trivially parallelized (with the exception of plotting)
    for(std::size_t pass_index=0; pass_index < n_pass; pass_index++)
    {
        if(pass_index % n_processes == local_id)
        {
            profiler_start();
            //grab this pass info
            mho_json pass = pass_vector[pass_index];
            std::string scan_dir = pass["directory"];
            std::string root_file = pass["root_file"];
            std::string baseline = pass["baseline"];
            std::string polprod = pass["polprod"];
            std::string fgroup = pass["frequency_group"];

            //populate a few necessary parameters and  initialize the fringe/scan data
            MHO_FringeData fringeData;
            fringeData.GetParameterStore()->CopyFrom(cmdline_params); //copy in command line info
            //set the current pass info (directory, root_file, source, baseline, pol-product, frequency-group)
            fringeData.GetParameterStore()->Set("/pass/directory", scan_dir);
            fringeData.GetParameterStore()->Set("/pass/root_file", root_file);
            fringeData.GetParameterStore()->Set("/pass/baseline", baseline);
            fringeData.GetParameterStore()->Set("/pass/polprod", polprod);
            fringeData.GetParameterStore()->Set("/pass/frequency_group", fgroup);

            //initializes the scan data store, reads the ovex file and sets the value of '/pass/source'
            bool scan_dir_ok = MHO_BasicFringeDataConfiguration::initialize_scan_data(fringeData.GetParameterStore(), fringeData.GetScanDataStore());
            MHO_BasicFringeDataConfiguration::populate_initial_parameters(fringeData.GetParameterStore(), fringeData.GetScanDataStore());

            //parse the control file and form the control statements
            MHO_FringeControlInitialization::process_control_file(fringeData.GetParameterStore(), fringeData.GetControlFormat(), fringeData.GetControlStatements() );

            bool do_ion = false;
            fringeData.GetParameterStore()->Get("/config/do_ion", do_ion);

            MHO_FringeFitter* ffit;
            //TODO FIXME...replace this logic with a factory method based on the
            //contents of the control and parameter store
            //but for the time being we only have two choices
            if(do_ion){ ffit = new MHO_IonosphericFringeFitter(&fringeData);}
            else{ ffit = new MHO_BasicFringeFitter(&fringeData);}
            ffit->Configure();

            ////////////////////////////////////////////////////////////////////////////
            //POST-CONFIGURE FOR COMPILE-TIME EXTENSIONS -- this should be reorganized with visitor pattern
            ////////////////////////////////////////////////////////////////////////////
            #ifdef USE_PYBIND11
            #pragma message("TODO FIXME -- formalize the means by which plugin dependent operator builders are added to the configuration")
            ffit->GetOperatorBuildManager()->AddBuilderType<MHO_PythonOperatorBuilder>("python_labeling", "python_labeling");
            ffit->GetOperatorBuildManager()->AddBuilderType<MHO_PythonOperatorBuilder>("python_flagging", "python_flagging");
            ffit->GetOperatorBuildManager()->AddBuilderType<MHO_PythonOperatorBuilder>("python_calibration", "python_calibration");
            #endif

            //initialize and perform run loop
            ffit->Initialize();
            while( !ffit->IsFinished() )
            {
                ffit->PreRun();
                ffit->Run();
                ffit->PostRun();
            }
            ffit->Finalize();

            ////////////////////////////////////////////////////////////////////////////
            //OUTPUT/PLOTTING -- this should be reorganized with visitor pattern
            ////////////////////////////////////////////////////////////////////////////
            bool test_mode = fringeData.GetParameterStore()->GetAs<bool>("/cmdline/test_mode");
            bool show_plot = fringeData.GetParameterStore()->GetAs<bool>("/cmdline/show_plot");
            mho_json plot_data = fringeData.GetPlotData();

            profiler_stop();
            std::vector< MHO_ProfileEvent > events;
            MHO_Profiler::GetInstance().GetEvents(events);
            //convert and dump the events into the parameter store for now (will be empty unless enabled)
            mho_json event_list = MHO_BasicFringeDataConfiguration::ConvertProfileEvents(events);
            fringeData.GetParameterStore()->Set("/profile/events", event_list);

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
                    fexporter.SetPlotData(plot_data);
                    fexporter.SetContainerStore(fringeData.GetContainerStore());
                    fexporter.ExportFringeFile();
                }
            }

            #ifdef USE_PYBIND11
            if(show_plot)
            {
                msg_debug("main", "python plot generation enabled." << eom );
                py::dict plot_obj = plot_data;

                // //QUICK HACK FOR PCPHASES UNTIL WE GET est_pc_maual working/////////////
                // try
                // {
                //     auto mod = py::module::import("mho_test3");
                //     mod.attr("generate_pcphases")(plot_obj);
                // }
                // catch(py::error_already_set &excep)
                // {
                //     msg_error("python_bindings", "python exception when calling subroutine (" << "mho_test3"<< "," << "generate_pcphases" << ")" << eom );
                //     msg_error("python_bindings", "python error message: "<< excep.what() << eom);
                //     PyErr_Clear(); //clear the error and attempt to continue
                // }

                ////////////////////////////////////////////////////////////////////////
                //load our interface module -- this is extremely slow!
                auto vis_module = py::module::import("hops_visualization");
                auto plot_lib = vis_module.attr("fourfit_plot");
                //call a python function on the interface class instance
                //TODO, pass filename to save plot if needed
                plot_lib.attr("make_fourfit_plot")(plot_obj, true, "");
            }
            #else //USE_PYBIND11
            if(show_plot)
            {
                msg_warn("main", "plot output requested, but not enabled since HOPS was built without pybind11 support, ignoring." << eom);
            }
            #endif

            //clean up
            delete ffit;

        }
    } //end of pass loop


    #ifdef HOPS_USE_MPI
        MHO_MPIInterface::GetInstance()->GlobalBarrier();
        MHO_MPIInterface::GetInstance()->Finalize();
    #endif

    return 0;
}
