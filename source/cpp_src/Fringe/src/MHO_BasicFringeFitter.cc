#include "MHO_BasicFringeFitter.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"
//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ElementTypeCaster.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include <pybind11/pybind11.h>
    #include <pybind11/embed.h>
    #include "pybind11_json/pybind11_json.hpp"
    namespace py = pybind11;
    namespace nl = nlohmann;
    using namespace pybind11::literals;
#endif


namespace hops 
{


MHO_BasicFringeFitter::MHO_BasicFringeFitter():MHO_FringeFitter(){};
MHO_BasicFringeFitter::~MHO_BasicFringeFitter(){};

//basic run scheme: configure, init, then while(!IsFinished() ){ pre-run, run, post-run }
void MHO_BasicFringeFitter::Configure()
{
    
}

void MHO_BasicFringeFitter::Initialize()
{
    
}

void MHO_BasicFringeFitter::PreRun()
{
    
}

void MHO_BasicFringeFitter::Run()
{
    
}

void MHO_BasicFringeFitter::PostRun()
{
    
}

bool MHO_BasicFringeFitter::IsFinished()
{
    
}


}//end namespace
