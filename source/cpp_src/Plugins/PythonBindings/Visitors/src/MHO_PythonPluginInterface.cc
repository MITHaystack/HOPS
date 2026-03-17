#include "MHO_PythonPluginInterface.hh"

namespace hops
{

bool MHO_PythonPluginInterface::fInitialized = false;

MHO_PythonPluginInterface::MHO_PythonPluginInterface()
{
    Initialize();
}

MHO_PythonPluginInterface::~MHO_PythonPluginInterface()
{
    Finalize();
}

void MHO_PythonPluginInterface::Initialize()
{
    if(!fInitialized)
    {
        msg_debug("python_bindings", "initializing python plugin interface" << eom);
        py::initialize_interpreter();
        configure_pypath();
        fInitialized = true;
    }
}


void MHO_PythonPluginInterface::Finalize()
{
    if(fInitialized)
    {
        py::finalize_interpreter();
        fInitialized = false;
    }
}


void 
MHO_PythonPluginInterface::Visit(MHO_FringeFitter* fitter)
{
    auto build_man = fitter->GetOperatorBuildManager();
    build_man->AddBuilderType< MHO_PythonOperatorBuilder >("python_labeling", "python_labeling");
    build_man->AddBuilderType< MHO_PythonOperatorBuilder >("python_flagging", "python_flagging");
    build_man->AddBuilderType< MHO_PythonOperatorBuilder >("python_calibration", "python_calibration");
    build_man->AddBuilderType< MHO_PythonOperatorBuilder >("python_prefit", "python_prefit");
    build_man->AddBuilderType< MHO_PythonOperatorBuilder >("python_postfit", "python_postfit");
    build_man->AddBuilderType< MHO_PythonOperatorBuilder >("python_finalize", "python_finalize");
}


}
