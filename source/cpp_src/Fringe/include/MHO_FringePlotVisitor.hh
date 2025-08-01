#ifndef MHO_FringePlotVisitor_HH__
#define MHO_FringePlotVisitor_HH__

#include "MHO_FringeFitter.hh"

namespace hops
{

/**
 * @brief Class MHO_FringePlotVisitor
 */
class MHO_FringePlotVisitor: public MHO_FringeFitterVisitor
{
    public:
        MHO_FringePlotVisitor(){};
        virtual ~MHO_FringePlotVisitor(){};

        //default behavior
        /**
         * @brief Visits a fringe fitter and plots its data.
         *
         * @param fitter The MHO_FringeFitter to visit and plot.
         * @note This is a virtual function.
         */
        virtual void Visit(MHO_FringeFitter* fitter) override
        {
            MHO_FringeData* data = fitter->GetFringeData();
            Plot(data);
        }

        //add specializations for specific fringe fitters if needed

    protected:
        /**
         * @brief Function Plot
         *
         * @param data (MHO_FringeData*)
         * @note This is a virtual function.
         */
        virtual void Plot(MHO_FringeData* data) = 0;
};

} // namespace hops

#endif /* end of include guard: MHO_FringePlotVisitor_HH__ */
