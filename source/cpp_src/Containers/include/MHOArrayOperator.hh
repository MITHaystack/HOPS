#ifndef MHOArrayOperator_HH__
#define MHOArrayOperator_HH__

#include "MHOArrayWrapper.hh"
#include <cstring>

namespace hops{

template<typename XValueType>
class MHOArrayOperator
{
    public:
        MHOArrayOperator(){};
        virtual ~MHOArrayOperator(){};

        virtual bool Initialize(){};
        virtual bool ExecuteOperation() = 0;

        //utilities
        template<std::size_t INPUT_RANK, std::size_t OUTPUT_RANK>
        static bool
        HaveSameNumberOfElements(const MHOArrayWrapper<XValueType,INPUT_RANK>* arr1, const MHOArrayWrapper<XValueType,OUTPUT_RANK>* arr2)
        {
            return ( arr1->GetSize() == arr2->GetSize() );
        }

        template<std::size_t INPUT_RANK, std::size_t OUTPUT_RANK>
        static bool
        HaveSameDimensions(const MHOArrayWrapper<XValueType,INPUT_RANK>* arr1, const MHOArrayWrapper<XValueType,OUTPUT_RANK>* arr2)
        {
            std::size_t shape1[INPUT_RANK];
            std::size_t shape2[OUTPUT_RANK];

            if(INPUT_RANK == OUTPUT_RANK)
            {
                arr1->GetDimensions(shape1);
                arr2->GetDimensions(shape2);

                for(std::size_t i=0; i<RANK; i++)
                {
                    if(shape1[i] != shape2[i]){return false;}
                }

                return true;
            }
            return false;
        }

        //set all of the elements in an array to be equal to the object obj
        template<std::size_t ARRAY_RANK>
        static void
        ResetArray(MHOArrayWrapper<XValueType,ARRAY_RANK>* arr, const XValueType& obj)
        {
            XValueType* ptr = arr->GetData();
            std::size_t n_elem = arr->GetSize();
            for(std::size_t i=0; i < n_elem; i++)
            {
                ptr[i] = obj;
            }
        }

        //set all of the elements in an array to be equal to zero
        template<std::size_t ARRAY_RANK>
        static void
        ZeroArray(MHOArrayWrapper<XValueType,ARRAY_RANK>* arr)
        {
            XValueType* ptr = arr->GetData();
            std::size_t n_bytes = (arr->GetSize() )*( sizeof(XValueType) );
            std::memset(ptr, 0, n_bytes);
        }


};


}//end of namespace

#endif /* __MHOArrayOperator_HH__ */
