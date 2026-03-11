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
    
MHO_DefaultPythonPlotVisitor::MHO_DefaultPythonPlotVisitor():
    fModulePath("hops_visualization.fourfit_plot"), //note dot syntax
    fFunctionName("make_fourfit_plot_wrapper")
{
    //default: module/library/attr names point to the broadband fringe plot
};


void MHO_DefaultPythonPlotVisitor::Plot(MHO_FringeData* data)
{
    msg_debug("python_bindings", "attempting to plot data with the python (matplotlib) plotting utility" << eom);
    
    bool is_skipped = data->GetParameterStore()->GetAs< bool >("/status/skipped");
    if(!is_skipped)
    {
        msg_debug("python_bindings", "python plot generation enabled." << eom);

        //determine if we have been passed a special/custom plotting function
        if( data->GetParameterStore()->IsPresent("/control/config/python_custom_plot") )
        {
            bool ok = false;
            ok = data->GetParameterStore()->Get("/control/config/python_custom_plot/module_path", fModulePath);
            ok &= data->GetParameterStore()->Get("/control/config/python_custom_plot/function_name", fFunctionName);
            if(!ok)
            {
                msg_error("python_bindings", "error retrieving python custom plot information" << eom);
            }
        }

        ConstructPlot(data);

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
    }
}

void MHO_DefaultPythonPlotVisitor::ConstructPlot(MHO_FringeData* data)
{
    MHO_PyFringeDataInterface data_wrapper(data);
    ////////////////////////////////////////////////////////////////////////
    //load our interface module -- this is extremely slow!
    try
    {
        //the modules pyMHO_Containers and pyMHO_Operators (required) are imported by configure_pypath()
        auto mod = py::module::import(fModulePath.c_str());
        mod.attr(fFunctionName.c_str())(data_wrapper);
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
                                             << fModulePath
                                             << ":"
                                             << fFunctionName
                                             << ")" << eom);
            msg_error("python_bindings", "python error message: " << excep.what() << eom);
            PyErr_Clear(); //clear the error and attempt to continue
        }
    }
}

} // namespace hops
