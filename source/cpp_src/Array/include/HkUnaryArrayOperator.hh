#ifndef HkUnaryArrayOperator_HH__
#define HkUnaryArrayOperator_HH__

#include "HkArrayOperator.hh"

namespace hops{

template<typename XValueType, std::size_t RANK>
class HkUnaryArrayOperator: public HkArrayOperator<XValueType, RANK>
{
    public:
        HkUnaryArrayOperator():fInput(NULL),fOutput(NULL){;};
        virtual ~HkUnaryArrayOperator(){;};

        virtual void SetInput(HkArrayWrapper<XValueType, RANK>* in){fInput = in;};
        virtual void SetOutput(HkArrayWrapper<XValueType, RANK>* out){fOutput = out;};

        virtual HkArrayWrapper<XValueType,RANK>* GetInput(){return fInput;};
        virtual HkArrayWrapper<XValueType,RANK>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        HkArrayWrapper<XValueType, RANK>* fInput;
        HkArrayWrapper<XValueType, RANK>* fOutput;
};

}//end of namespace


#endif /* __HkUnaryArrayOperator_HH__ */
