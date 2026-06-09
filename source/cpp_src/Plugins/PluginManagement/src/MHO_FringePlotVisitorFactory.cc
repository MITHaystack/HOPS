#include "MHO_FringePlotVisitorFactory.hh"

#ifdef USE_MATPLOTPP
    #include "MHO_BasicPlotVisitor.hh"
#endif

// Python plotting backend: in-process (embedded) or via the user's python3
// (subprocess). At most one of these is defined in a python-enabled build.
#if defined(USE_EMBEDDED_PYTHON)
    #include "MHO_DefaultPythonPlotVisitor.hh"
#elif defined(USE_PYTHON_SUBPROCESS)
    #include "MHO_SubprocessPythonPlotVisitor.hh"
#endif

namespace hops
{

namespace
{
//construct the python (matplotlib) plotter for the compiled backend, or nullptr
//if no python plotting backend is available in this build.
static MHO_FringePlotVisitor* make_python_plotter()
{
#if defined(USE_EMBEDDED_PYTHON)
    return new MHO_DefaultPythonPlotVisitor();
#elif defined(USE_PYTHON_SUBPROCESS)
    return new MHO_SubprocessPythonPlotVisitor();
#else
    return nullptr;
#endif
}
} // namespace

MHO_FringePlotVisitorFactory::MHO_FringePlotVisitorFactory(): fFringePlotter(nullptr)
{}

MHO_FringePlotVisitorFactory::~MHO_FringePlotVisitorFactory()
{
    //delete the fringe plotter
    if(fFringePlotter)
    {
        delete fFringePlotter;
        fFringePlotter = nullptr;
    }
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
        msg_debug("fringe", "plotting backend choice is: " << plot_backend << eom);
        fFringePlotter = new MHO_BasicPlotVisitor();
        return fFringePlotter;
#else
        msg_warn("fringe", "plotting backend choice: " << plot_backend << " is not available on this system " << eom);
#endif
    }
    else if(plot_backend == "matplotlib")
    {

#if defined(USE_EMBEDDED_PYTHON) || defined(USE_PYTHON_SUBPROCESS)
        msg_debug("fringe", "plotting backend choice is: " << plot_backend << eom);
        fFringePlotter = make_python_plotter();
        return fFringePlotter;
#else
        msg_warn("fringe", "plotting backend choice: " << plot_backend << " is not available on this system " << eom);
#endif
    }

//if plot_backend was unset, and we have 'gnuplot' available, then use that
#ifdef USE_MATPLOTPP
    msg_debug("fringe", "default plotting backend is: gnuplot " << eom);
    fFringePlotter = new MHO_BasicPlotVisitor();
    return fFringePlotter;
#endif

//made it here, so no plot_backend was set, and 'gnuplot' wasn't build, so fall back to python
#if defined(USE_EMBEDDED_PYTHON) || defined(USE_PYTHON_SUBPROCESS)
    msg_debug("fringe", "default plotting backend is: matplotlib " << eom);
    fFringePlotter = make_python_plotter();
    return fFringePlotter;
#endif

    //we don't have any plotting backend enabled, return nullptr
    return nullptr;
}

} // namespace hops
