#include "MHO_FringeFitterFactory.hh"
#include "MHO_BasicFringeFitter.hh"
#include "MHO_IonosphericFringeFitter.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include "pybind11_json/pybind11_json.hpp"
    #include <pybind11/embed.h>
    #include <pybind11/pybind11.h>
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;
    #include "MHO_PyConfigurePath.hh"
    #include "MHO_PythonOperatorBuilder.hh"
#endif

namespace hops
{

MHO_FringeFitterFactory::MHO_FringeFitterFactory(MHO_FringeData* data): fFringeData(data), fFringeFitter(nullptr)
{}

MHO_FringeFitterFactory::~MHO_FringeFitterFactory()
{
    //delete the fringe fitter
    delete fFringeFitter;
}

MHO_FringeFitter* MHO_FringeFitterFactory::ConstructFringeFitter()
{
    //if it has already been built, just return the existing one
    if(fFringeFitter != nullptr)
    {
        return fFringeFitter;
    }

    //currently we only have two fringe-fitting options (basic or with ionosphere fitting)
    //but if we add more types, we need to add logic to decide what type should be built
    bool do_ion = false;
    fFringeData->GetParameterStore()->Get("/config/do_ion", do_ion);

    if(do_ion)
    {
        msg_debug("fringe", "constructing an ionospheric fringe fitter" << eom);
        fFringeFitter = new MHO_IonosphericFringeFitter(fFringeData);
    }
    else
    {
        msg_debug("fringe", "constructing a basic fringe fitter" << eom);
        fFringeFitter = new MHO_BasicFringeFitter(fFringeData);
    }

    ////////////////////////////////////////////////////////////////////////////
    //POST-CONFIGURE FOR COMPILE-TIME EXTENSIONS
    ////////////////////////////////////////////////////////////////////////////

#ifdef USE_PYBIND11
    fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_labeling", "python_labeling");
    fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_flagging", "python_flagging");
    fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_calibration", "python_calibration");
    fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_prefit", "python_prefit");
    fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_postfit", "python_postfit");
    fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_finalize", "python_finalize");
#endif

    //configures data and builds operators
    fFringeFitter->Configure();

    return fFringeFitter;
}

} // namespace hops
