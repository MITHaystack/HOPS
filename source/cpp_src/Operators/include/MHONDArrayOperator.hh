#ifndef MHONDArrayOperator_HH__
#define MHONDArrayOperator_HH__

#include "MHONDArrayWrapper.hh"
#include <cstring>

namespace hops{

 //template parameters must inherit from MHONDArrayWrapper
template<class XInputArrayType, class XOutputArrayType>
class MHONDArrayOperator
{
    public:

        MHONDArrayOperator():
            fInput(nullptr),
            fOutput(nullptr)
        {};

        virtual ~MHONDArrayOperator(){};

        virtual void SetInput(XInputArrayType* in){fInput = in;};
        virtual void SetOutput(XOutputArrayType* out){fOutput = out;};
        virtual XInputArrayType* GetInput(){return fInput;};
        virtual XOutputArrayType* GetOutput(){return fOutput;};

        virtual bool Initialize() = 0;
        virtual bool ExecuteOperation() = 0;

    protected:

        XInputArrayType* fInput;
        XOutputArrayType* fOutput;

};


}//end of namespace

#endif /* __MHONDArrayOperator_HH__ */
