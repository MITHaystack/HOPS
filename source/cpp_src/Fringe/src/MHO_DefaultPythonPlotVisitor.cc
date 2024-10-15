#include "MHO_DefaultPythonPlotVisitor.hh"
#include "MHO_Message.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include "pybind11_json/pybind11_json.hpp"
    #include <pybind11/embed.h>
    #include <pybind11/pybind11.h>
    namespace py = pybind11;
    namespace nl = nlohmann;
    using namespace pybind11::literals;
    #include "MHO_PyConfigurePath.hh"
    #include "MHO_PythonOperatorBuilder.hh"
    #include "MHO_PyFringeDataInterface.hh"
#endif


namespace hops 
{

void 
MHO_DefaultPythonPlotVisitor::Plot(MHO_FringeData* data)
{
    msg_info("fringe", "going to plot with the default python plotting utility" <<eom);
    ////////////////////////////////////////////////////////////////////////////
    //OUTPUT/PLOTTING -- this should be reorganized with visitor pattern
    ////////////////////////////////////////////////////////////////////////////
    bool test_mode = data->GetParameterStore()->GetAs< bool >("/cmdline/test_mode");
    bool show_plot = data->GetParameterStore()->GetAs< bool >("/cmdline/show_plot");
    bool is_skipped = data->GetParameterStore()->GetAs< bool >("/status/skipped");


#ifdef USE_PYBIND11
    if(show_plot && !is_skipped)
    {
        msg_debug("main", "python plot generation enabled." << eom);
        mho_json plot_data = data->GetPlotData();
        py::dict plot_obj = plot_data;

        MHO_PyFringeDataInterface pyDataInterface(data);

        // //QUICK HACK FOR PCPHASES UNTIL WE GET est_pc_maual working/////////////
        // try
        // {
        //     auto mod = py::module::import("mho_test3");
        //     mod.attr("generate_pcphases_wrapper")(pyDataInterface);
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
    if(show_plot && !is_skipped)
    {
        msg_warn("main",
                 "plot output requested, but not enabled since HOPS was built without pybind11 support, ignoring."
                     << eom);
    }
#endif


}

}//end namespace
