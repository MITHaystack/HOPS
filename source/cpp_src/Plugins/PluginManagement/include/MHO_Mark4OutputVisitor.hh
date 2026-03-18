#ifndef MHO_Mark4OutputVisitor_HH__
#define MHO_Mark4OutputVisitor_HH__

#include "MHO_FringeFitter.hh"

namespace hops 
{

class MHO_Mark4OutputVisitor: public MHO_FringeFitterVisitor
{
    public:
        MHO_Mark4OutputVisitor();
        virtual ~MHO_Mark4OutputVisitor();

        virtual void Visit(MHO_FringeFitter* fitter) override;

    protected:

};

}//end namespace

#endif /* end of include guard: MHO_Mark4OutputVisitor_HH__ */
