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
        MHO_DefaultPythonPlotVisitor(){};
        virtual ~MHO_DefaultPythonPlotVisitor(){};

        //default visit behavior is fine
        //add specializations for specific fringe fitters if needed

    protected:
        /**
         * @brief Plots fringe data using default Python plotting utility.
         *
         * @param data Input MHO_FringeData for plotting.
         * @note This is a virtual function.
         */
        virtual void Plot(MHO_FringeData* data) override;
};

} // namespace hops

#endif /* end of include guard: MHO_DefaultPythonPlotVisitor_HH__ */
