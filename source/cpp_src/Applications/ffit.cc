#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define EXTRA_DEBUG

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"
#include "MHO_Timer.hh"

//fringe finding library helper functions
#include "MHO_BasicFringeFitter.hh"
#include "MHO_IonosphericFringeFitter.hh"

//for command line parsing
#include "MHO_BasicFringeDataConfiguration.hh"
//for control intialization
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
#endif

#define EXPORT_MK4

#ifdef EXPORT_MK4
#include "MHO_MK4FringeExport.hh"
#endif

using namespace hops;

//TODO move this some where else
mho_json ConvertProfileEvents(std::vector< MHO_ProfileEvent >& events)
{
    mho_json event_list;
    for(std::size_t i=0; i<events.size(); i++)
    {
        mho_json obj;
        obj["event_id"] = i;
        obj["flag"] = events[i].fFlag;
        obj["line"] = events[i].fLineNumber;
        obj["thread_id"] = events[i].fThreadID;
        obj["filename"] = std::string( events[i].fFilename );
        obj["funcname"] = std::string( events[i].fFuncname );
        obj["time"] = events[i].fTime;
        event_list.push_back(obj);
    }
    return event_list;
}


int main(int argc, char** argv)
{
    profiler_start();

    //TODO allow messaging keys to be set via command line arguments
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("ffit"));

    MHO_FringeData fringeData;

    int parse_status = MHO_BasicFringeDataConfiguration::parse_command_line(argc, argv, fringeData.GetParameterStore() );
    if(parse_status != 0){msg_fatal("main", "could not parse command line options." << eom); std::exit(1);}

    //populate a few necessary parameters and  initialize the scan data store
    MHO_BasicFringeDataConfiguration::populate_initial_parameters(fringeData.GetParameterStore(), fringeData.GetScanDataStore());

    //parse the control file and form the control statements
    MHO_FringeControlInitialization::process_control_file(fringeData.GetParameterStore(), fringeData.GetControlFormat(), fringeData.GetControlStatements() );

    bool do_ion = false;
    fringeData.GetParameterStore()->Get("/config/do_ion", do_ion);

    MHO_FringeFitter* ffit;
    //TODO FIXME...replace this logic with a factory method based on the parameter store
    //but for the time being we only have two choices
    if(do_ion){ ffit = new MHO_IonosphericFringeFitter(&fringeData);}
    else{ ffit = new MHO_BasicFringeFitter(&fringeData);}

    #ifdef USE_PYBIND11
    // start the interpreter and keep it alive, need this or we segfault
    py::scoped_interpreter guard{};
    #endif

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
    mho_json event_list = ConvertProfileEvents(events);
    //dump the events into the parameter store for now (will be empty unless enabled)
    fringeData.GetParameterStore()->Set("/profile/events", event_list);

    //open and dump to file -- should we profile this as well?
    if(!test_mode)
    {
        fringeData.WriteOutput();

        #ifdef EXPORT_MK4
        MHO_MK4FringeExport fexporter;
        fexporter.SetParameterStore(fringeData.GetParameterStore());
        fexporter.SetPlotData(plot_data);
        fexporter.SetContainerStore(fringeData.GetContainerStore());
        fexporter.ExportFringeFile();
        #endif
    }



    #ifdef USE_PYBIND11
    if(show_plot)
    {
        // MHO_Timer timer;
        // double current_time;
        // timer.Start();

        msg_debug("main", "python plot generation enabled." << eom );
        py::dict plot_obj = plot_data;

        //load our interface module -- this is extremely slow!
        auto vis_module = py::module::import("hops_visualization");
        auto plot_lib = vis_module.attr("fourfit_plot");
        //call a python function on the interface class instance
        //TODO, pass filename to save plot if needed
        plot_lib.attr("make_fourfit_plot")(plot_obj, true, "");

        // current_time = timer.GetTimeSinceStart();
        // std::cout<<"time to plot = "<<current_time<<std::endl;

    }
    #else //USE_PYBIND11
    if(show_plot)
    {
        msg_warn("main", "plot output requested, but not enabled since HOPS was built without pybind11 support, ignoring." << eom);
    }
    #endif



    return 0;
}
