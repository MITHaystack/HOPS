#ifndef MHO_FringePlotVisitor_HH__
#define MHO_FringePlotVisitor_HH__

#include "MHO_FringeFitter.hh"

namespace hops 
{

class MHO_FringePlotVisitor: public MHO_FringeFitterVisitor
{
    public:
        MHO_FringePlotVisitor(){};
        virtual ~MHO_FringePlotVisitor(){};

        //default behavior
        virtual void Visit(MHO_FringeFitter* fitter) override
        {
            MHO_FringeData* data = fitter->GetFringeData();
            Plot(data);
        }
        
        //add specializations for specific fringe fitters if needed

    protected:

        virtual void Plot(MHO_FringeData* data) = 0;
};


}//end namespace

#endif /* end of include guard: MHO_FringePlotVisitor_HH__ */
