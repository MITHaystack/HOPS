#ifndef MHO_PyGenericOperator_HH__
#define MHO_PyGenericOperator_HH__

#include "MHO_Operator.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_FringeData.hh"
#include "MHO_PyContainerStoreInterface.hh"
#include "MHO_PyFringeDataInterface.hh"
#include "MHO_PyParameterStoreInterface.hh"

#include "MHO_PyTableContainer.hh"

#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace hops
{

/*!
 *@file  MHO_PyGenericOperator.hh
 *@class  MHO_PyGenericOperator
 *@author  J. Barrett - barrettj@mit.edu
 *@date Mon Sep 18 13:51:51 2023 -0400
 *@brief  this class allows a user to inject a python function of the form:
 * func(fringe_data_interface)
 * into the control flow of the fringe fitter. It is basically allowed full access to
 * any data or parameters encompassed by the fringe_data object (container store, parameter store, plot data).
 * The only exception is that re-sizing of arrays is not allowed.
 */

class MHO_PyGenericOperator: public MHO_Operator
{
    public:
        MHO_PyGenericOperator(): fInitialized(false), fFringeData(nullptr), fFringeDataInterface(nullptr)
        {
            fModuleName = "";
            fFunctionName = "";
        };

        virtual ~MHO_PyGenericOperator() { delete fFringeDataInterface; };

        void SetFringeData(MHO_FringeData* fdata) { fFringeData = fdata; }

        // void SetParameterStore(MHO_ParameterStore* pstore) { fParameterStore = pstore; };
        //
        // void SetContainerStore(MHO_ContainerStore* cstore) { fContainerStore = cstore; };

        void SetModuleName(std::string module_name) { fModuleName = module_name; }

        void SetFunctionName(std::string function_name) { fFunctionName = function_name; }

        virtual bool Initialize() override
        {
            fInitialized = false;
            if(fModuleName == "")
            {
                return false;
            }
            if(fFunctionName == "")
            {
                return false;
            }

            //construct the python interface exposing the parameter and container store
            if(fFringeData != nullptr)
            {
                if(fFringeDataInterface == nullptr)
                {
                    fFringeDataInterface = new MHO_PyFringeDataInterface(fFringeData);
                }
                fInitialized = true;
            }

            // //construct the python interface exposing the parameter and container store
            // if(fContainerStore != nullptr && fParameterStore != nullptr)
            // {
            //     if(fContainerInterface == nullptr){fContainerInterface = new MHO_PyContainerStoreInterface(fContainerStore); }
            //     if(fParameterInterface == nullptr){fParameterInterface = new MHO_PyParameterStoreInterface(fParameterStore); }
            //     fInitialized = true;
            // }
            return fInitialized;
        };

        virtual bool Execute() override
        {
            if(fInitialized)
            {
                bool success = false;
                if(Py_IsInitialized() == 0)
                {
                    //the internal python interpreter has not been started, bail out
                    msg_error("python_bindings", "python interpreter not running/initialized, "
                                                     << "cannot call python subroutine (" << fModuleName << "," << fFunctionName
                                                     << ")." << eom);
                    return success;
                }

                //calling user code, so use try-catch in case there are errors
                try
                {
                    auto mod = py::module::import(fModuleName.c_str());
                    // mod.attr(fFunctionName.c_str())(*fContainerInterface, *fParameterInterface);
                    mod.attr(fFunctionName.c_str())(*fFringeDataInterface);
                    success = true;
                }
                catch(py::error_already_set& excep)
                {
                    success = false;
                    msg_error("python_bindings", "python exception when calling subroutine (" << fModuleName << ","
                                                                                              << fFunctionName << ")" << eom);
                    msg_error("python_bindings", "python error message: " << excep.what() << eom);
                    msg_warn("python_bindings", "attempting to continue, but in-memory data may be in unknown state." << eom);
                    PyErr_Clear(); //clear the error and attempt to continue
                }

                return success;
            }
            return false;
        }

    private:
        bool fInitialized;
        std::string fModuleName;
        std::string fFunctionName;

        MHO_FringeData* fFringeData;
        MHO_PyFringeDataInterface* fFringeDataInterface;

        // MHO_ContainerStore* fContainerStore;
        // MHO_ParameterStore* fParameterStore;
        // MHO_PyParameterStoreInterface* fParameterInterface;
        // MHO_PyContainerStoreInterface* fContainerInterface;
};

} // namespace hops

#endif /*! end of include guard: MHO_PyGenericOperator */
