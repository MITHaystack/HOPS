#include "MHO_DefaultPythonPlotVisitor.hh"
#include "MHO_Message.hh"

#define PYBIND11_DETAILED_ERROR_MESSAGES

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include "pybind11_json/pybind11_json.hpp"
    #include <pybind11/embed.h>
    #include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
    #include <pybind11/pybind11.h>
    namespace py = pybind11;
    namespace nl = nlohmann;
    using namespace pybind11::literals;
    #include "MHO_PyConfigurePath.hh"
    #include "MHO_PythonOperatorBuilder.hh"
    #include "MHO_PyContainerStoreInterface.hh"
    #include "MHO_PyFringeDataInterface.hh"
    #include "MHO_PyParameterStoreInterface.hh"
    #include "MHO_PyScanStoreInterface.hh"
    #include "MHO_PyTableContainer.hh"
#endif


namespace hops 
{

void 
MHO_DefaultPythonPlotVisitor::Plot(MHO_FringeData* data)
{
    msg_debug("fringe", "attempting to plot data with the default python plotting utility" <<eom);
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

        MHO_PyFringeDataInterface data_wrapper(data);

        // // //QUICK HACK FOR PCPHASES UNTIL WE GET est_pc_maual working/////////////
        // try
        // {
        //     auto mod = py::module::import("mho_test3");
        //     mod.attr("dummy_wrapper")(*data_wrapper);
        // }
        // catch(py::error_already_set &excep)
        // {
        //     msg_error("python_bindings", "python exception when calling subroutine (" << "mho_test3"<< "," << "dummy_wrapper" << ")" << eom );
        //     msg_error("python_bindings", "python error message: "<< excep.what() << eom);
        //     PyErr_Clear(); //clear the error and attempt to continue
        // }

        ////////////////////////////////////////////////////////////////////////
        //load our interface module -- this is extremely slow!
        try 
        {
            //required modules pyMHO_Containers and pyMHO_Operators are imported by configure_pypath()
            auto vis_module = py::module::import("hops_visualization");
            auto plot_lib = vis_module.attr("fourfit_plot");
            plot_lib.attr("make_fourfit_plot_wrapper")(data_wrapper);
        }
        catch(py::error_already_set &excep)
        {
            if( std::string(excep.what()).find("SystemExit") != std::string::npos) 
            {
                msg_debug("python_bindings", "sys.exit() called from within python, exiting" << eom);
                std::exit(0);//exit
            }
            else 
            {
                msg_error("python_bindings", "python exception when calling subroutine (" << "fourfit_plot"<< "," << "make_fourfit_plot_wrapper" << ")" << eom );
                msg_error("python_bindings", "python error message: "<< excep.what() << eom);
                PyErr_Clear(); //clear the error and attempt to continue
            }
        }
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
