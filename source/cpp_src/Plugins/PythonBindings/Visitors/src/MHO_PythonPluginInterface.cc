#include "MHO_PythonPluginInterface.hh"

namespace hops
{

MHO_PythonPluginInterface* MHO_PythonPluginInterface::fPythonPluginInterface = nullptr;

MHO_PythonPluginInterface::MHO_PythonPluginInterface(){}
MHO_PythonPluginInterface::~MHO_PythonPluginInterface() = default;

MHO_PythonPluginInterface* MHO_PythonPluginInterface::GetInstance()
{
    //singleton interface construction
    if(fPythonPluginInterface == nullptr)
    {
        fPythonPluginInterface = new MHO_PythonPluginInterface();
    }
    return fPythonPluginInterface;
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
