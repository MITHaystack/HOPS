#ifndef MHOArrayOperator_HH__
#define MHOArrayOperator_HH__

#include "MHOArrayWrapper.hh"
#include <cstring>

namespace hops{

template<typename XValueType, std::size_t RANK>
class MHOArrayOperator
{
    public:
        MHOArrayOperator(){};
        virtual ~MHOArrayOperator(){};

        virtual void Initialize(){};
        virtual void ExecuteOperation() = 0;

        //utilities
        static bool
        HaveSameNumberOfElements(const MHOArrayWrapper<XValueType,RANK>* arr1, const MHOArrayWrapper<XValueType,RANK>* arr2)
        {
            return ( arr1->GetSize() == arr2->GetSize() );
        }

        static bool
        HaveSameDimensions(const MHOArrayWrapper<XValueType,RANK>* arr1, const MHOArrayWrapper<XValueType,RANK>* arr2)
        {
            std::size_t shape1[RANK];
            std::size_t shape2[RANK];

            arr1->GetDimensions(shape1);
            arr2->GetDimensions(shape2);

            for(std::size_t i=0; i<RANK; i++)
            {
                if(shape1[i] != shape2[i]){return false;}
            }

            return true;
        }

        //set all of the elements in an array to be equal to the object obj
        static void
        ResetArray(MHOArrayWrapper<XValueType,RANK>* arr, const XValueType& obj)
        {
            XValueType* ptr = arr->GetData();
            std::size_t n_elem = arr->GetSize();
            for(std::size_t i=0; i < n_elem; i++)
            {
                ptr[i] = obj;
            }
        }

        //set all of the elements in an array to be equal to zero
        static void
        ZeroArray(MHOArrayWrapper<XValueType,RANK>* arr)
        {
            XValueType* ptr = arr->GetData();
            std::size_t n_bytes = (arr->GetSize() )*( sizeof(XValueType) );
            std::memset(ptr, 0, n_bytes);
        }


};


}//end of namespace

#endif /* __MHOArrayOperator_HH__ */
