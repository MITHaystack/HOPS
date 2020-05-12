#ifndef HUnaryArrayOperator_H__
#define HUnaryArrayOperator_H__

#include "HArrayOperator.hh"

namespace hops{

template<typename ArrayType, unsigned int NDIM>
class HUnaryArrayOperator: public HArrayOperator<ArrayType, NDIM>
{
    public:
        HUnaryArrayOperator():fInput(NULL),fOutput(NULL){;};
        virtual ~HUnaryArrayOperator(){;};

        virtual void SetInput(HArrayWrapper<ArrayType, NDIM>* in){fInput = in;};
        virtual void SetOutput(HArrayWrapper<ArrayType, NDIM>* out){fOutput = out;};

        virtual HArrayWrapper<ArrayType,NDIM>* GetInput(){return fInput;};
        virtual HArrayWrapper<ArrayType,NDIM>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        HArrayWrapper<ArrayType, NDIM>* fInput;
        HArrayWrapper<ArrayType, NDIM>* fOutput;
};

}//end of namespace


#endif /* __HUnaryArrayOperator_H__ */
