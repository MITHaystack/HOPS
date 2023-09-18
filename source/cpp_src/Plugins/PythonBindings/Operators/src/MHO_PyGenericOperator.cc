#include "MHO_PyGenericOperator.hh"

namespace hops 
{

MHO_PyGenericOperator::MHO_PyGenericOperator():
    fInitialized(false),
    fParameterStore(nullptr),
    fContainerStore(nullptr),
    fParameterInterface(nullptr),
    fContainerInterface(nullptr)
{
    fModuleName = "";
    fFunctionName = "";
}

MHO_PyGenericOperator::~MHO_PyGenericOperator()
{
    //just delete the interface objects, the original param/container stores are managed exeternally
    delete fContainerInterface;
    delete fParameterInterface;
}

void MHO_PyGenericOperator::SetParameterStore(MHO_ParameterStore* pstore){fParameterStore = pstore;}
void MHO_PyGenericOperator::SetContainerStore(MHO_ContainerStore* cstore){fContainerStore = cstore;}

void MHO_PyGenericOperator::SetModuleName(std::string module_name){fModuleName = module_name;}
void MHO_PyGenericOperator::SetFunctionName(std::string function_name){fFunctionName = function_name;}

bool MHO_PyGenericOperator::Initialize()
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
}


bool MHO_PyGenericOperator::Execute()
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


}//end of namespace