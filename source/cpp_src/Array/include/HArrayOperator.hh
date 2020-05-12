#ifndef HArrayOperator_H__
#define HArrayOperator_H__

#include "HArrayWrapper.hh"
#include <cstring>

namespace hops{

template<typename T, size_t RANK>
class HArrayOperator
{
    public:
        HArrayOperator(){};
        virtual ~HArrayOperator(){};

        virtual void Initialize(){};
        virtual void ExecuteOperation() = 0;

        //utilities
        static bool
        HaveSameNumberOfElements(const HArrayWrapper<T,RANK>* arr1, const HArrayWrapper<T,RANK>* arr2)
        {
            return ( arr1->GetArraySize() == arr2->GetArraySize() );
        }

        static bool
        HaveSameDimensions(const HArrayWrapper<T,RANK>* arr1, const HArrayWrapper<T,RANK>* arr2)
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
        ResetArray(HArrayWrapper<T,RANK>* arr, const T& obj)
        {
            T* ptr = arr->GetData();
            size_t n_elem = arr->GetArraySize();
            for(size_t i=0; i < n_elem; i++)
            {
                ptr[i] = obj;
            }
        }

        //set all of the elements in an array to be equal to zero
        static void
        ZeroArray(HArrayWrapper<T,RANK>* arr)
        {
            T* ptr = arr->GetData();
            size_t n_bytes = (arr->GetArraySize() )*( sizeof(T) );
            std::memset(ptr, 0, n_bytes);
        }


};


}//end of namespace

#endif /* __HArrayOperator_H__ */
