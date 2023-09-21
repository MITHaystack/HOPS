#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define EXTRA_DEBUG

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"


//fringe finding library helper functions
#include "MHO_BasicFringeFitter.hh"

//for command line parsing
#include "MHO_BasicFringeDataConfiguration.hh"

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

using namespace hops;

int main(int argc, char** argv)
{
    //TODO allow messaging keys to be set via command line arguments
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("ffit"));
    
    MHO_BasicFringeFitter ffit;
    int parse_status = MHO_BasicFringeDataConfiguration::parse_command_line(argc, argv, ffit.GetParameterStore() );
    if(parse_status != 0){msg_fatal("main", "could not parse command line options." << eom); std::exit(1);}
    
    #ifdef USE_PYBIND11
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive, need this or we segfault
    #endif
    
    ffit.Configure();
    
    ////////////////////////////////////////////////////////////////////////////
    //POST-CONFIGURE FOR COMPILE-TIME EXTENSIONS -- this should be reorganized with visitor pattern
    ////////////////////////////////////////////////////////////////////////////
    #ifdef USE_PYBIND11
    #pragma message("TODO FIXME -- formalize the means by which plugin dependent operator builders are added to the configuration")
    ffit.GetOperatorBuildManager()->AddBuilderType<MHO_PythonOperatorBuilder>("python_labelling", "python_labelling"); 
    ffit.GetOperatorBuildManager()->AddBuilderType<MHO_PythonOperatorBuilder>("python_flagging", "python_flagging"); 
    ffit.GetOperatorBuildManager()->AddBuilderType<MHO_PythonOperatorBuilder>("python_calibration", "python_calibration"); 
    #endif
    
    
    //initialize and perform run loop
    ffit.Initialize();
    while( !ffit.IsFinished() )
    {
        ffit.PreRun();
        ffit.Run();
        ffit.PostRun();
    }
    
    ffit.Finalize();
    
    ////////////////////////////////////////////////////////////////////////////
    //OUTPUT/PLOTTING -- this should be reorganized with visitor pattern
    ////////////////////////////////////////////////////////////////////////////
    
    mho_json plot_data = ffit.GetPlotData();
        //open and dump to file
    std::string output_file = ffit.GetParameterStore()->GetAs<std::string>("/cmdline/output_file");
    std::ofstream fdumpFile(output_file.c_str(), std::ofstream::out);
    fdumpFile << plot_data;
    fdumpFile.close();
    
    #ifdef USE_PYBIND11
    msg_debug("main", "python plot generation enabled." << eom );
    py::dict plot_obj = plot_data;
    
    //load our interface module
    auto vis_module = py::module::import("hops_visualization");
    auto plot_lib = vis_module.attr("fourfit_plot");
    //call a python function on the interface class instance
    plot_lib.attr("make_fourfit_plot")(plot_obj, "fplot.png");
    #endif //USE_PYBIND11

    return 0;
}
