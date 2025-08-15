#include "MHO_FringePlotVisitorFactory.hh"

#ifdef USE_MATPLOTPP
    #include "MHO_BasicPlotVisitor.hh"
#endif

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include "MHO_DefaultPythonPlotVisitor.hh"
#endif

namespace hops
{

MHO_FringePlotVisitorFactory::MHO_FringePlotVisitorFactory(): fFringePlotter(nullptr)
{}

MHO_FringePlotVisitorFactory::~MHO_FringePlotVisitorFactory()
{
    //delete the fringe plotter
    if(fFringePlotter){delete fFringePlotter; fFringePlotter = nullptr;}
}

MHO_FringePlotVisitor* MHO_FringePlotVisitorFactory::ConstructPlotter()
{
    //if it has already been built, just return the existing one
    if(fFringePlotter != nullptr)
    {
        return fFringePlotter;
    }

    //currently we only have two fringe plotting options 
    //the first is based on matplot++ 
    //the second is based on python matplotlib 
    #ifdef USE_MATPLOTPP
        fFringePlotter = new MHO_BasicPlotVisitor();
        return fFringePlotter;
    #endif

    #ifdef USE_PYBIND11
        fFringePlotter = new MHO_DefaultPythonPlotVisitor();
        return fFringePlotter;
    #endif 

    //we don't have any plotting backend enabled, return nullptr
    return nullptr;
}

} // namespace hops
