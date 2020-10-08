#ifndef HkBinaryArrayOperator_HH__
#define HkBinaryArrayOperator_HH__

#include "HkArrayOperator.hh"

namespace hops{

template<typename XValueType, std::size_t RANK>
class HkBinaryArrayOperator: public HkArrayOperator<XValueType,RANK>
{
    public:
        HkBinaryArrayOperator():fFirstInput(NULL),fSecondInput(NULL),fOutput(NULL){;};
        virtual ~HkBinaryArrayOperator(){;};

        virtual void SetFirstInput(HkArrayWrapper<XValueType,RANK>* in){fFirstInput = in;};
        virtual void SetSecondInput(HkArrayWrapper<XValueType,RANK>* in){fSecondInput = in;};
        virtual void SetOutput(HkArrayWrapper<XValueType,RANK>* out){fOutput = out;};

        virtual HkArrayWrapper<XValueType,RANK>* GetFirstInput(){return fFirstInput;};
        virtual HkArrayWrapper<XValueType,RANK>* GetSecondInput(){return fSecondInput;};
        virtual HkArrayWrapper<XValueType,RANK>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        HkArrayWrapper<XValueType,RANK>* fFirstInput;
        HkArrayWrapper<XValueType,RANK>* fSecondInput;
        HkArrayWrapper<XValueType,RANK>* fOutput;
};

}


#endif /* __HkBinaryArrayOperator_HH__ */
