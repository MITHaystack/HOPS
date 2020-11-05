#ifndef MHOArrayOperator_HH__
#define MHOArrayOperator_HH__

#include "MHOArrayWrapper.hh"
#include <cstring>

namespace hops{

 //template parameters must inherit from MHOArrayWrapper
template<typename XInputArrayType, XOutputArrayType>
class MHOArrayOperator
{
    public:

        MHOArrayOperator():
            fInput(nullptr),
            fOutput(nullptr)
        {};

        virtual ~MHOArrayOperator(){};

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

#endif /* __MHOArrayOperator_HH__ */
