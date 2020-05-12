#ifndef HBinaryArrayOperator_HH__
#define HBinaryArrayOperator_HH__

#include "HArrayOperator.hh"

namespace hops{

template<typename XValueType, unsigned int NDIM>
class HBinaryArrayOperator: public HArrayOperator<XValueType,NDIM>
{
    public:
        HBinaryArrayOperator():fFirstInput(NULL),fSecondInput(NULL),fOutput(NULL){;};
        virtual ~HBinaryArrayOperator(){;};

        virtual void SetFirstInput(HArrayWrapper<XValueType,NDIM>* in){fFirstInput = in;};
        virtual void SetSecondInput(HArrayWrapper<XValueType,NDIM>* in){fSecondInput = in;};
        virtual void SetOutput(HArrayWrapper<XValueType,NDIM>* out){fOutput = out;};

        virtual HArrayWrapper<XValueType,NDIM>* GetFirstInput(){return fFirstInput;};
        virtual HArrayWrapper<XValueType,NDIM>* GetSecondInput(){return fSecondInput;};
        virtual HArrayWrapper<XValueType,NDIM>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        HArrayWrapper<XValueType,NDIM>* fFirstInput;
        HArrayWrapper<XValueType,NDIM>* fSecondInput;
        HArrayWrapper<XValueType,NDIM>* fOutput;
};

}


#endif /* __HBinaryArrayOperator_HH__ */
