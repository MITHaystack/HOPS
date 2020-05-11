#ifndef HArrayOperator_H__
#define HArrayOperator_H__

#include "HArrayWrapper.hh"
#include <cstring>

namespace hops{


/**
*
*@file HArrayOperator.hh
*@class HArrayOperator
*@brief base class for operating on boost multi_array types with utilities
*@details
*
*<b>Revision History:<b>
*Date Name Brief Description
*Wed Sep 26 22:54:30 EDT 2012 J. Barrett (barrettj@mit.edu) First Version
*
*/



template<typename T, unsigned int NDIM>
class HArrayOperator
{
    public:
        HArrayOperator(){;};
        virtual ~HArrayOperator(){;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

        //utilities
        static bool
        HaveSameNumberOfElements(const HArrayWrapper<T,NDIM>* arr1, const HArrayWrapper<T,NDIM>* arr2)
        {
            return ( arr1->GetArraySize() == arr2->GetArraySize() );
        }

        static bool
        HaveSameDimensions(const HArrayWrapper<T,NDIM>* arr1, const HArrayWrapper<T,NDIM>* arr2)
        {
            unsigned int shape1[NDIM];
            unsigned int shape2[NDIM];

            arr1->GetArrayDimensions(shape1);
            arr2->GetArrayDimensions(shape2);

            for(unsigned int i=0; i<NDIM; i++)
            {
                if(shape1[i] != shape2[i]){return false;}
            }

            return true;
        }

        //set all of the elements in an array to be equal to the object obj
        static void
        ResetArray(HArrayWrapper<T,NDIM>* arr, const T& obj)
        {
            T* ptr = arr->GetData();
            unsigned int n_elem = arr->GetArraySize();
            for(unsigned int i=0; i < n_elem; i++)
            {
                ptr[i] = obj;
            }
        }

        //set all of the elements in an array to be equal to zero
        static void
        ZeroArray(HArrayWrapper<T,NDIM>* arr)
        {
            T* ptr = arr->GetData();
            unsigned int n_bytes = (arr->GetArraySize() )*( sizeof(T) );
            std::memset(ptr, 0, n_bytes);
        }


        static bool
        IsBoundedDomainSubsetOfArray(HArrayWrapper<T,NDIM>* arr,
        const int fLowerBound[NDIM], const int fUpperBound[NDIM])
        {
            unsigned int shape[NDIM];
            int bases[NDIM];
            arr->GetArrayDimensions(shape);
            arr->GetArrayBases(bases);

            //std::cout<<"____________"<<std::endl;

            //check that the output array's 'base values' are
            //less than or equal to the lower limits over which we will scan
            for(unsigned int i=0; i<NDIM; i++)
            {
                //std::cout<<"lower_bound = "<<fLowerBound[i]<<", base = "<<bases[i]<<std::endl;
                if(fLowerBound[i] < bases[i])
                {
                    return false;
                }
            }

            //check that the upper limits are less than or equal to
            //the base_values + the size of the array in that dimension
            for(unsigned int i=0; i<NDIM; i++)
            {
                //std::cout<<"upper_bound = "<<fUpperBound[i]<<", highest index = "<<(bases[i] + int(shape[i]) )<<std::endl;
                if( (bases[i] + int(shape[i]) ) < fUpperBound[i])
                {
                    return false;
                }
            }

            //checks passed
            return true;

        }


    protected:

};


}//end of namespace

#endif /* __HArrayOperator_H__ */
