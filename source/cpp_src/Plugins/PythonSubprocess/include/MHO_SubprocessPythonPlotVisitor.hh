#ifndef MHO_SubprocessPythonPlotVisitor_HH__
#define MHO_SubprocessPythonPlotVisitor_HH__

#include "MHO_FringePlotVisitor.hh"

namespace hops
{

/*!
 *@file  MHO_SubprocessPythonPlotVisitor.hh
 *@class MHO_SubprocessPythonPlotVisitor
 *@author J. Barrett - barrettj@mit.edu
 *@brief No-embed (subprocess) default fringe plotter. Implements the same
 *       MHO_FringePlotVisitor interface as the embedded
 *       MHO_DefaultPythonPlotVisitor, but renders by shelling out to the user's
 *       python3 (`python -m hops_visualization.fourfit_plot <request.json>`),
 *       passing the plot data as JSON. No libpython is linked.
 *
 * The C++ side already holds the plot data as mho_json (MHO_FringeData::
 * GetPlotData()), and the Python renderer (make_fourfit_plot) is entirely
 * dict-driven, so the subprocess path produces the same plot as the embedded
 * path for the DEFAULT plotter.
 *
 * Scope / behavior (decided):
 *  - DEFAULT plot only. Custom Python plots (/control/config/python_custom_plot)
 *    are NOT supported here; if requested we warn and render the default plot.
 *  - On any failure (no python3, import error, render error) we WARN and SKIP
 *    the plot, then continue processing. No hard error, no matplot++ fallback.
 */

class MHO_SubprocessPythonPlotVisitor: public MHO_FringePlotVisitor
{
    public:
        MHO_SubprocessPythonPlotVisitor(){};
        virtual ~MHO_SubprocessPythonPlotVisitor(){};

        /**
         * @brief Render the fringe plot via the python3 subprocess.
         * @param data Input MHO_FringeData (provides plot_dict + cmdline params).
         * @note This is a virtual function.
         */
        virtual void Plot(MHO_FringeData* data) override;
};

} // namespace hops

#endif /* end of include guard: MHO_SubprocessPythonPlotVisitor_HH__ */
