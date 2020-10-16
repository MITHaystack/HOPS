#ifndef MHOBinaryArrayOperator_HH__
#define MHOBinaryArrayOperator_HH__

#include "MHOArrayOperator.hh"

namespace hops{

template<typename XValueType, std::size_t RANK>
class MHOBinaryArrayOperator: public MHOArrayOperator<XValueType,RANK>
{
    public:
        MHOBinaryArrayOperator():fFirstInput(NULL),fSecondInput(NULL),fOutput(NULL){;};
        virtual ~MHOBinaryArrayOperator(){;};

        virtual void SetFirstInput(MHOArrayWrapper<XValueType,RANK>* in){fFirstInput = in;};
        virtual void SetSecondInput(MHOArrayWrapper<XValueType,RANK>* in){fSecondInput = in;};
        virtual void SetOutput(MHOArrayWrapper<XValueType,RANK>* out){fOutput = out;};

        virtual MHOArrayWrapper<XValueType,RANK>* GetFirstInput(){return fFirstInput;};
        virtual MHOArrayWrapper<XValueType,RANK>* GetSecondInput(){return fSecondInput;};
        virtual MHOArrayWrapper<XValueType,RANK>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        MHOArrayWrapper<XValueType,RANK>* fFirstInput;
        MHOArrayWrapper<XValueType,RANK>* fSecondInput;
        MHOArrayWrapper<XValueType,RANK>* fOutput;
};

}


#endif /* __MHOBinaryArrayOperator_HH__ */
