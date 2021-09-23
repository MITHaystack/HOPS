#ifndef MHO_BinaryNDArrayOperator_HH__
#define MHO_BinaryNDArrayOperator_HH__

#include "MHO_Operator.hh"
#include "MHO_NDArrayWrapper.hh"
#include <cstring>

namespace hops{

 //template parameters must inherit from MHO_NDArrayWrapper
template<class XInputArrayType1, class XInputArrayType2, class XOutputArrayType>
class MHO_BinaryNDArrayOperator: public MHO_Operator
{
    public:

        MHO_BinaryNDArrayOperator():
            fInput1(nullptr),
            fInput2(nullptr),
            fOutput(nullptr)
        {};

        virtual ~MHO_BinaryNDArrayOperator(){};

        virtual void SetFirstInput(XInputArrayType1* in){fInput1 = in;};
        virtual void SetSecondInput(XInputArrayType2* in){fInput2 = in;};
        virtual void SetOutput(XOutputArrayType* out){fOutput = out;};

        virtual XInputArrayType1* GetFirstInput(){return fInput1;};
        virtual XInputArrayType2* GetSecondInput(){return fInput2;};
        virtual XOutputArrayType* GetOutput(){return fOutput;};

        //inherts these from MHO_Operator
        // virtual bool Initialize() = 0;
        // virtual bool ExecuteOperation() = 0;

    protected:

        XInputArrayType1* fInput1;
        XInputArrayType2* fInput2;
        XOutputArrayType* fOutput;

};


}


#endif /* __MHO_BinaryNDArrayOperator_HH__ */
