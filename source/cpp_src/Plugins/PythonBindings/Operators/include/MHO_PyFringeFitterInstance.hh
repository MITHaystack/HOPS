#ifndef MHO_PyFringeFitterInstance_HH__
#define MHO_PyFringeFitterInstance_HH__

#include "MHO_Operator.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_PyContainerStoreInterface.hh"
#include "MHO_PyParameterStoreInterface.hh"
#include "MHO_PyTableContainer.hh"

#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace hops
{

/*
 *@file  MHO_PyFringeFitterInstance.hh
 *@class  MHO_PyFringeFitterInstance
 *@author  J. Barrett - barrettj@mit.edu
 *@date Tue Sep 19 16:26:35 2023 -0400
 *@brief  Container class which provides an interface to a fringe fitter instance
 */

class MHO_PyFringeFitterInstance
{
    public:
        MHO_PyFringeFitterInstance(){};
        virtual ~MHO_PyFringeFitterInstance(){};

        // void SetParameterStore(MHO_ParameterStore* pstore){fParameterStore = pstore;};
        // void SetContainerStore(MHO_ContainerStore* cstore){fContainerStore = cstore;};
        //
        // void SetModuleName(std::string module_name){fModuleName = module_name;}
        // void SetFunctionName(std::string function_name){fFunctionName = function_name;}

    private:
        // bool fInitialized;
        // std::string fModuleName;
        // std::string fFunctionName;
        //
        // MHO_ContainerStore* fContainerStore;
        // MHO_ParameterStore* fParameterStore;
        //
        // MHO_PyParameterStoreInterface* fParameterInterface;
        // MHO_PyContainerStoreInterface* fContainerInterface;
};

} // namespace hops

#endif /*! end of include guard: MHO_PyFringeFitterInstance */
