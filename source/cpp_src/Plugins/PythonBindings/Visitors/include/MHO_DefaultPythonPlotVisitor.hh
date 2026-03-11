#ifndef MHO_DefaultPythonPlotVisitor_HH__
#define MHO_DefaultPythonPlotVisitor_HH__

#include "MHO_FringePlotVisitor.hh"

namespace hops
{

/**
 * @brief Class MHO_DefaultPythonPlotVisitor
 */
class MHO_DefaultPythonPlotVisitor: public MHO_FringePlotVisitor
{

    public:
        MHO_DefaultPythonPlotVisitor();
        virtual ~MHO_DefaultPythonPlotVisitor(){};

        //default visit behavior is fine
        //add specializations for specific fringe fitters if needed

        /**
         * @brief Plots fringe data using default Python plotting utility.
         *
         * @param data Input MHO_FringeData for plotting.
         * @note This is a virtual function.
         */
        virtual void Plot(MHO_FringeData* data) override;

    protected:

        std::string fModulePath; //python module to import (. syntax works, e.g. hops_visualization.fourfit_plot)
        std::string fFunctionName; //python function to call (must be a free function that accepts MHO_PyFringeDataInterface)
        
        virtual void ConstructPlot(MHO_FringeData* data);



};

} // namespace hops

#endif /* end of include guard: MHO_DefaultPythonPlotVisitor_HH__ */
