#ifndef HUnaryArrayOperator_HH__
#define HUnaryArrayOperator_HH__

#include "HArrayOperator.hh"

namespace hops{

template<typename XValueType, unsigned int RANK>
class HUnaryArrayOperator: public HArrayOperator<XValueType, RANK>
{
    public:
        HUnaryArrayOperator():fInput(NULL),fOutput(NULL){;};
        virtual ~HUnaryArrayOperator(){;};

        virtual void SetInput(HArrayWrapper<XValueType, RANK>* in){fInput = in;};
        virtual void SetOutput(HArrayWrapper<XValueType, RANK>* out){fOutput = out;};

        virtual HArrayWrapper<XValueType,RANK>* GetInput(){return fInput;};
        virtual HArrayWrapper<XValueType,RANK>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        HArrayWrapper<XValueType, RANK>* fInput;
        HArrayWrapper<XValueType, RANK>* fOutput;
};

}//end of namespace


#endif /* __HUnaryArrayOperator_HH__ */
