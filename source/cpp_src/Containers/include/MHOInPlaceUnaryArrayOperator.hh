#ifndef MHOInPlaceUnaryArrayOperator_HH__
#define MHOInPlaceUnaryArrayOperator_HH__

#include "MHOArrayOperator.hh"

namespace hops{

template< typename XValueType, std::size_t RANK>
class MHOInPlaceUnaryArrayOperator: public MHOArrayOperator<XValueType>
{
    public:
        MHOInPlaceUnaryArrayOperator():fOutput(NULL){};
        virtual ~MHOInPlaceUnaryArrayOperator(){};

        virtual void SetOutput(MHOArrayWrapper<XValueType,RANK>* out)
        {
            fOutput = out;
        };

        virtual MHOArrayWrapper<XValueType,RANK>* GetOutput(){return fOutput;};

        virtual bool Initialize(){return true;};

        virtual bool ExecuteOperation() = 0;

    protected:

        MHOArrayWrapper<XValueType,RANK>* fOutput;

};

}//end of namespace


#endif /* __MHOInPlaceUnaryArrayOperator_HH__ */
