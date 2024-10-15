#ifndef MHO_DefaultPythonPlotVisitor_HH__
#define MHO_DefaultPythonPlotVisitor_HH__

#include "MHO_FringePlotVisitor.hh"

namespace hops 
{

class MHO_DefaultPythonPlotVisitor: public MHO_FringePlotVisitor
{
    public:
        MHO_DefaultPythonPlotVisitor(){};
        virtual ~MHO_DefaultPythonPlotVisitor(){};

        //default visit behavior is fine
        //add specializations for specific fringe fitters if needed

    protected:

        virtual void Plot(MHO_FringeData* data) = 0;
};

}//end of namespace

#endif /* end of include guard: MHO_DefaultPythonPlotVisitor_HH__ */
