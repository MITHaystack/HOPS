#include "MHO_DefaultPythonPlotVisitor.hh"
#include "MHO_Message.hh"

#define PYBIND11_DETAILED_ERROR_MESSAGES

//pybind11 stuff to interface with python
#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/embed.h>
#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
#include <pybind11/pybind11.h>
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;
#include "MHO_PyConfigurePath.hh"
#include "MHO_PyContainerStoreInterface.hh"
#include "MHO_PyFringeDataInterface.hh"
#include "MHO_PyParameterStoreInterface.hh"
#include "MHO_PyScanStoreInterface.hh"
#include "MHO_PyTableContainer.hh"
#include "MHO_PythonOperatorBuilder.hh"


namespace hops
{

void MHO_DefaultPythonPlotVisitor::Plot(MHO_FringeData* data)
{
    msg_debug("fringe", "attempting to plot data with the python (matplotlib) plotting utility" << eom);
    
    bool is_skipped = data->GetParameterStore()->GetAs< bool >("/status/skipped");
    if(!is_skipped)
    {
        msg_debug("main", "python plot generation enabled." << eom);
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
        catch(py::error_already_set& excep)
        {
            if(std::string(excep.what()).find("SystemExit") != std::string::npos)
            {
                msg_debug("python_bindings", "sys.exit() called from within python, exiting" << eom);
                std::exit(0); //ok to exit program entirely
            }
            else
            {
                msg_error("python_bindings", "python exception when calling subroutine ("
                                                 << "fourfit_plot"
                                                 << ","
                                                 << "make_fourfit_plot_wrapper"
                                                 << ")" << eom);
                msg_error("python_bindings", "python error message: " << excep.what() << eom);
                PyErr_Clear(); //clear the error and attempt to continue
            }
        }
    }
}

} // namespace hops
