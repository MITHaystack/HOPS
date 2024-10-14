#include "MHO_FringeFitterFactory.hh"
#include "MHO_BasicFringeFitter.hh"
#include "MHO_IonosphericFringeFitter.hh"


namespace hops 
{

MHO_FringeFitterFactory::MHO_FringeFitterFactory(MHO_FringeData* data):
    fFringeData(data),
    fFringeFitter(nullptr)
{}

MHO_FringeFitterFactory::~MHO_FringeFitterFactory()
{
    //delete the fringe fitter 
    delete fFringeFitter;
}

MHO_FringeFitter* 
MHO_FringeFitterFactory::ConstructFringeFitter()
{
    //if it has already been built, just return the existing one
    if(fFringeFitter != nullptr){return fFringeFitter;}

    //currently we only have two fringe-fitting options (basic or with ionosphere fitting)
    //but if we add more types, we need to add logic to decide what type should be built
    bool do_ion = false;
    fFringeData->GetParameterStore()->Get("/config/do_ion", do_ion);

    if(do_ion)
    {
        msg_debug("fringe", "constructing an ionospheric fringe fitter" << eom );
        fFringeFitter = new MHO_IonosphericFringeFitter(fFringeData);
    }
    else
    {
        msg_debug("fringe", "constructing a basic fringe fitter" << eom);
        fFringeFitter = new MHO_BasicFringeFitter(fFringeData);
    }
    fFringeFitter->Configure();
    return fFringeFitter;
}


}//end namespace