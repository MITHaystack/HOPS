#ifndef HInPlaceUnaryArrayOperator_HH__
#define HInPlaceUnaryArrayOperator_HH__

#include "HArrayOperator.hh"

namespace hops{

template< typename XValueType, unsigned int RANK>
class HInPlaceUnaryArrayOperator: public HArrayOperator<XValueType,RANK>
{
    public:
        HInPlaceUnaryArrayOperator():fOutput(NULL){};
        virtual ~HInPlaceUnaryArrayOperator(){};

        virtual void SetOutput(HArrayWrapper<XValueType,RANK>* out)
        {
            fOutput = out;
        };

        virtual HArrayWrapper<XValueType,RANK>* GetOutput(){return fOutput;};

        virtual void Initialize(){};

        virtual void ExecuteOperation() = 0;

    protected:

        HArrayWrapper<XValueType,RANK>* fOutput;

};

}//end of namespace


#endif /* __HInPlaceUnaryArrayOperator_HH__ */
