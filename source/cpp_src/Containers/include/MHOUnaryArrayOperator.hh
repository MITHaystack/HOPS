#ifndef MHOUnaryArrayOperator_HH__
#define MHOUnaryArrayOperator_HH__

#include "MHOArrayOperator.hh"

namespace hops{

template<typename XValueType, std::size_t INPUT_RANK, std::size_t OUTPUT_RANK>
class MHOUnaryArrayOperator: public MHOArrayOperator<XValueType>
{
    public:
        MHOUnaryArrayOperator():fInput(NULL),fOutput(NULL){;};
        virtual ~MHOUnaryArrayOperator(){;};

        virtual void SetInput(MHOArrayWrapper<XValueType, INPUT_RANK>* in){fInput = in;};
        virtual void SetOutput(MHOArrayWrapper<XValueType, OUTPUT_RANK>* out){fOutput = out;};

        virtual MHOArrayWrapper<XValueType,INPUT_RANK>* GetInput(){return fInput;};
        virtual MHOArrayWrapper<XValueType,OUTPUT_RANK>* GetOutput(){return fOutput;};

        virtual bool Initialize(){;};

        virtual bool ExecuteOperation() = 0;

    protected:

        MHOArrayWrapper<XValueType, INPUT_RANK>* fInput;
        MHOArrayWrapper<XValueType, OUTPUT_RANK>* fOutput;
};

}//end of namespace


#endif /* __MHOUnaryArrayOperator_HH__ */
