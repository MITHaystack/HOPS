#ifndef MHO_HopsOutputVisitor_HH__
#define MHO_HopsOutputVisitor_HH__

#include "MHO_FringeFitter.hh"

namespace hops 
{

class MHO_HopsOutputVisitor: public MHO_FringeFitterVisitor
{
    public:
        MHO_HopsOutputVisitor();
        virtual ~MHO_HopsOutputVisitor();

        virtual void Visit(MHO_FringeFitter* fitter) override;


    protected:

        int WriteDataObjects(MHO_FringeData* data, std::string filename);

};

}//end namespace

#endif /* end of include guard: MHO_HopsOutputVisitor_HH__ */
