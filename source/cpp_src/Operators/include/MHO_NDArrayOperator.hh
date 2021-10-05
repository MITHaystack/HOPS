#ifndef MHO_NDArrayOperator_HH__
#define MHO_NDArrayOperator_HH__

#include "MHO_Operator.hh"
#include "MHO_NDArrayWrapper.hh"
#include <cstring>

namespace hops{

 //template parameters must inherit from MHO_NDArrayWrapper
template<class XInputArrayType, class XOutputArrayType>
class MHO_NDArrayOperator: public MHO_Operator
{
    public:

        MHO_NDArrayOperator():
            fInput(nullptr),
            fOutput(nullptr)
        {};

        virtual ~MHO_NDArrayOperator(){};

        virtual void SetInput(XInputArrayType* in){fInput = in;};
        virtual void SetOutput(XOutputArrayType* out){fOutput = out;};
        virtual XInputArrayType* GetInput(){return fInput;};
        virtual XOutputArrayType* GetOutput(){return fOutput;};

        //inherts these from MHO_Operator
        // virtual bool Initialize() = 0;
        // virtual bool Execute() = 0;

    protected:

        XInputArrayType* fInput;
        XOutputArrayType* fOutput;

};


}//end of namespace

#endif /* __MHO_NDArrayOperator_HH__ */
