#ifndef MHOUnaryArrayOperator_HH__
#define MHOUnaryArrayOperator_HH__

#include "MHOArrayOperator.hh"

namespace hops{

template<typename XValueType, std::size_t RANK>
class MHOUnaryArrayOperator: public MHOArrayOperator<XValueType, RANK>
{
    public:
        MHOUnaryArrayOperator():fInput(NULL),fOutput(NULL){;};
        virtual ~MHOUnaryArrayOperator(){;};

        virtual void SetInput(MHOArrayWrapper<XValueType, RANK>* in){fInput = in;};
        virtual void SetOutput(MHOArrayWrapper<XValueType, RANK>* out){fOutput = out;};

        virtual MHOArrayWrapper<XValueType,RANK>* GetInput(){return fInput;};
        virtual MHOArrayWrapper<XValueType,RANK>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        MHOArrayWrapper<XValueType, RANK>* fInput;
        MHOArrayWrapper<XValueType, RANK>* fOutput;
};

}//end of namespace


#endif /* __MHOUnaryArrayOperator_HH__ */
