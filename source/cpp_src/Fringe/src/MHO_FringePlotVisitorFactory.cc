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

MHO_FringePlotVisitor* MHO_FringePlotVisitorFactory::ConstructPlotter(std::string plot_backend)
{
    //if it has already been built, just return the existing one
    if(fFringePlotter != nullptr)
    {
        return fFringePlotter;
    }

    if(plot_backend == "gnuplot")
    {

        #ifdef USE_MATPLOTPP
            msg_debug("fringe", "plotting backend choice is: "<< plot_backend << eom);
            fFringePlotter = new MHO_BasicPlotVisitor();
            return fFringePlotter;
        #else 
            msg_warn("fringe", "plotting backend choice: "<< plot_backend <<" is not available on this system " << eom);
        #endif
    }
    else if(plot_backend == "matplotlib")
    {

        #ifdef USE_PYBIND11
            msg_debug("fringe", "plotting backend choice is: "<< plot_backend << eom);
            fFringePlotter = new MHO_DefaultPythonPlotVisitor();
            return fFringePlotter;
        #else 
            msg_warn("fringe", "plotting backend choice: "<< plot_backend <<" is not available on this system " << eom);
        #endif
    }

    //if plot_backend was unset, and we have 'gnuplot' available, then use that
    #ifdef USE_MATPLOTPP
        msg_debug("fringe", "default plotting backend is: gnuplot "<< eom);
        fFringePlotter = new MHO_BasicPlotVisitor();
        return fFringePlotter;
    #endif

    //made it here, so no plot_backend was set, and 'gnuplot' wasn't build, so fall back to python
    #ifdef USE_PYBIND11
        msg_debug("fringe", "default plotting backend is: matplotlib "<< eom);
        fFringePlotter = new MHO_DefaultPythonPlotVisitor();
        return fFringePlotter;
    #endif 

    //we don't have any plotting backend enabled, return nullptr
    return nullptr;
}

} // namespace hops
