#ifndef HkInPlaceUnaryArrayOperator_HH__
#define HkInPlaceUnaryArrayOperator_HH__

#include "HkArrayOperator.hh"

namespace hops{

template< typename XValueType, std::size_t RANK>
class HkInPlaceUnaryArrayOperator: public HkArrayOperator<XValueType,RANK>
{
    public:
        HkInPlaceUnaryArrayOperator():fOutput(NULL){};
        virtual ~HkInPlaceUnaryArrayOperator(){};

        virtual void SetOutput(HkArrayWrapper<XValueType,RANK>* out)
        {
            fOutput = out;
        };

        virtual HkArrayWrapper<XValueType,RANK>* GetOutput(){return fOutput;};

        virtual void Initialize(){};

        virtual void ExecuteOperation() = 0;

    protected:

        HkArrayWrapper<XValueType,RANK>* fOutput;

};

}//end of namespace


#endif /* __HkInPlaceUnaryArrayOperator_HH__ */
