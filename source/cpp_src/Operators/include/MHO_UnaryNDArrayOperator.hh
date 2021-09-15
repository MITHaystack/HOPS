#ifndef MHO_UnaryNDArrayOperator_HH__
#define MHO_UnaryNDArrayOperator_HH__

#include "MHO_NDArrayWrapper.hh"
#include <cstring>

namespace hops{

//template parameters must inherit from MHO_NDArrayWrapper
//only operates on a single array, input = ouput
template<class XInputArrayType>
class MHO_UnaryNDArrayOperator
{
    public:

        MHO_UnaryNDArrayOperator():
            fInput(nullptr)
        {};

        virtual ~MHO_UnaryNDArrayOperator(){};

        virtual void SetInput(XInputArrayType* in){fInput = in;};
        virtual XInputArrayType* GetInput(){return fInput;};

        virtual bool Initialize() = 0;
        virtual bool ExecuteOperation() = 0;

    protected:

        XInputArrayType* fInput;

};


}//end of namespace

#endif /* __MHO_UnaryNDArrayOperator_HH__ */
