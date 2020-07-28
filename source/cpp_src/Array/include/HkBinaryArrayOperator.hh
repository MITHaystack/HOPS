#ifndef HkBinaryArrayOperator_HH__
#define HkBinaryArrayOperator_HH__

#include "HkArrayOperator.hh"

namespace hops{

template<typename XValueType, unsigned int NDIM>
class HkBinaryArrayOperator: public HkArrayOperator<XValueType,NDIM>
{
    public:
        HkBinaryArrayOperator():fFirstInput(NULL),fSecondInput(NULL),fOutput(NULL){;};
        virtual ~HkBinaryArrayOperator(){;};

        virtual void SetFirstInput(HkArrayWrapper<XValueType,NDIM>* in){fFirstInput = in;};
        virtual void SetSecondInput(HkArrayWrapper<XValueType,NDIM>* in){fSecondInput = in;};
        virtual void SetOutput(HkArrayWrapper<XValueType,NDIM>* out){fOutput = out;};

        virtual HkArrayWrapper<XValueType,NDIM>* GetFirstInput(){return fFirstInput;};
        virtual HkArrayWrapper<XValueType,NDIM>* GetSecondInput(){return fSecondInput;};
        virtual HkArrayWrapper<XValueType,NDIM>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        HkArrayWrapper<XValueType,NDIM>* fFirstInput;
        HkArrayWrapper<XValueType,NDIM>* fSecondInput;
        HkArrayWrapper<XValueType,NDIM>* fOutput;
};

}


#endif /* __HkBinaryArrayOperator_HH__ */
