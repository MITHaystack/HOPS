#ifndef MHO_PyGenericOperator_HH__
#define MHO_PyGenericOperator_HH__

/*
*@file: MHO_PyGenericOperator.hh
*@class: MHO_PyGenericOperator
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date: Mon 18 Sep 2023 01:26:22 PM EDT
*@brief:
*/

#include "MHO_Operator.hh"

#include "MHO_PyTableContainer.hh"
#include "MHO_PyParameterStoreInterface.hh"
#include "MHO_PyContainerStoreInterface.hh"
#include "MHO_ContainerDefinitions.hh"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
namespace py = pybind11;

namespace hops
{


class MHO_PyGenericOperator: public MHO_Operator
{
    public:

        MHO_PyGenericOperator():
            fInitialized(false),
            fParameterStore(nullptr),
            fContainerStore(nullptr),
            fParameterInterface(nullptr),
            fContainerInterface(nullptr)
        {
            fModuleName = "";
            fFunctionName = "";
        };

        virtual ~MHO_PyGenericOperator()
        {
            delete fContainerInterface;
            delete fParameterInterface;
        };

        void SetParameterStore(MHO_ParameterStore* pstore){fParameterStore = pstore;};
        void SetContainerStore(MHO_ContainerStore* cstore){fContainerStore = cstore;};

        void SetModuleName(std::string module_name){fModuleName = module_name;}
        void SetFunctionName(std::string function_name){fFunctionName = function_name;}

        virtual bool Initialize() override
        {
            fInitialized = false;
            if(fModuleName == ""){return false;}
            if(fFunctionName == ""){return false;}
            //construct the python interface exposing the parameter and container store
            if(fContainerStore != nullptr && fParameterStore != nullptr)
            {
                fContainerInterface = new MHO_PyContainerStoreInterface(fContainerStore);
                fParameterInterface = new MHO_PyParameterStoreInterface(fParameterStore);
                fInitialized = true;
            }
            return fInitialized;
        };


        virtual bool Execute() override
        {
            if(fInitialized)
            {    
                //assume the python interpreter is already running (should we use try/catch?)
                auto mod = py::module::import(fModuleName.c_str());
                mod.attr(fFunctionName.c_str())(*fContainerInterface, *fParameterInterface);
                return true;
            }
            return false;
        }

    private:

        bool fInitialized;
        std::string fModuleName;
        std::string fFunctionName;

        MHO_ContainerStore* fContainerStore;
        MHO_ParameterStore* fParameterStore;

        MHO_PyParameterStoreInterface* fParameterInterface;
        MHO_PyContainerStoreInterface* fContainerInterface;

};

}//end of namespace


#endif /* end of include guard: MHO_PyGenericOperator */
