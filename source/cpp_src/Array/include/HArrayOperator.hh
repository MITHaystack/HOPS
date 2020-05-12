#ifndef HArrayOperator_HH__
#define HArrayOperator_HH__

#include "HArrayWrapper.hh"
#include <cstring>

namespace hops{

template<typename XValueType, size_t RANK>
class HArrayOperator
{
    public:
        HArrayOperator(){};
        virtual ~HArrayOperator(){};

        virtual void Initialize(){};
        virtual void ExecuteOperation() = 0;

        //utilities
        static bool
        HaveSameNumberOfElements(const HArrayWrapper<XValueType,RANK>* arr1, const HArrayWrapper<XValueType,RANK>* arr2)
        {
            return ( arr1->GetArraySize() == arr2->GetArraySize() );
        }

        static bool
        HaveSameDimensions(const HArrayWrapper<XValueType,RANK>* arr1, const HArrayWrapper<XValueType,RANK>* arr2)
        {
            size_t shape1[RANK];
            size_t shape2[RANK];

            arr1->GetArrayDimensions(shape1);
            arr2->GetArrayDimensions(shape2);

            for(size_t i=0; i<RANK; i++)
            {
                if(shape1[i] != shape2[i]){return false;}
            }

            return true;
        }

        //set all of the elements in an array to be equal to the object obj
        static void
        ResetArray(HArrayWrapper<XValueType,RANK>* arr, const XValueType& obj)
        {
            XValueType* ptr = arr->GetData();
            size_t n_elem = arr->GetArraySize();
            for(size_t i=0; i < n_elem; i++)
            {
                ptr[i] = obj;
            }
        }

        //set all of the elements in an array to be equal to zero
        static void
        ZeroArray(HArrayWrapper<XValueType,RANK>* arr)
        {
            XValueType* ptr = arr->GetData();
            size_t n_bytes = (arr->GetArraySize() )*( sizeof(XValueType) );
            std::memset(ptr, 0, n_bytes);
        }


};


}//end of namespace

#endif /* __HArrayOperator_HH__ */
