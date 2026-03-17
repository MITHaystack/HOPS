#include "MHO_FringeFitterFactory.hh"
#include "MHO_BasicFringeFitter.hh"
#include "MHO_IonosphericFringeFitter.hh"
// #include "MHO_IonosphericFringeFitterOpenMP.hh"
#include "MHO_SpectralLineFringeFitter.hh"

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

    //determine which fringe fitter to construct
    bool do_spectral_line = false;
    if(fFringeData->GetParameterStore()->IsPresent("/control/fit/spectral_line") )
    {
        fFringeData->GetParameterStore()->Get("/control/fit/spectral_line", do_spectral_line);
    }
    
    bool do_ion = false;
    fFringeData->GetParameterStore()->Get("/config/do_ion", do_ion);

    if(do_spectral_line)
    {
        msg_debug("fringe", "constructing a spectral line fringe fitter" << eom);
        fFringeFitter = new MHO_SpectralLineFringeFitter(fFringeData);
    }
    else if(do_ion)
    {
        msg_debug("fringe", "constructing an ionospheric fringe fitter" << eom);
        // #ifdef _OPENMP
        // fFringeFitter = new MHO_IonosphericFringeFitterOpenMP(fFringeData);
        // #else
        fFringeFitter = new MHO_IonosphericFringeFitter(fFringeData);
        // #endif
    }
    else
    {
        msg_debug("fringe", "constructing a basic fringe fitter" << eom);
        fFringeFitter = new MHO_BasicFringeFitter(fFringeData);
    }

    return fFringeFitter;
}

} // namespace hops
