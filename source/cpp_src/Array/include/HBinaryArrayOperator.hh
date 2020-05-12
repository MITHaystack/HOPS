#ifndef HBinaryArrayOperator_H__
#define HBinaryArrayOperator_H__

#include "HArrayOperator.hh"

namespace hops{

template<typename T, unsigned int NDIM>
class HBinaryArrayOperator: public HArrayOperator<T,NDIM>
{
    public:
        HBinaryArrayOperator():fFirstInput(NULL),fSecondInput(NULL),fOutput(NULL){;};
        virtual ~HBinaryArrayOperator(){;};

        virtual void SetFirstInput(HArrayWrapper<T,NDIM>* in){fFirstInput = in;};
        virtual void SetSecondInput(HArrayWrapper<T,NDIM>* in){fSecondInput = in;};
        virtual void SetOutput(HArrayWrapper<T,NDIM>* out){fOutput = out;};

        virtual HArrayWrapper<T,NDIM>* GetFirstInput(){return fFirstInput;};
        virtual HArrayWrapper<T,NDIM>* GetSecondInput(){return fSecondInput;};
        virtual HArrayWrapper<T,NDIM>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        HArrayWrapper<T,NDIM>* fFirstInput;
        HArrayWrapper<T,NDIM>* fSecondInput;
        HArrayWrapper<T,NDIM>* fOutput;
};

}


#endif /* __HBinaryArrayOperator_H__ */
