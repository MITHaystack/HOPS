#ifndef HInPlaceUnaryArrayOperator_H__
#define HInPlaceUnaryArrayOperator_H__

#include "HArrayOperator.hh"

namespace hops{

template< typename T, unsigned int RANK>
class HInPlaceUnaryArrayOperator: public HArrayOperator<T,RANK>
{
    public:
        HInPlaceUnaryArrayOperator():fOutput(NULL){};
        virtual ~HInPlaceUnaryArrayOperator(){};

        virtual void SetOutput(HArrayWrapper<T,RANK>* out)
        {
            fOutput = out;
        };

        virtual HArrayWrapper<T,RANK>* GetOutput(){return fOutput;};

        virtual void Initialize(){};

        virtual void ExecuteOperation() = 0;

    protected:

        HArrayWrapper<T,RANK>* fOutput;

};

}//end of namespace


#endif /* __HInPlaceUnaryArrayOperator_H__ */
