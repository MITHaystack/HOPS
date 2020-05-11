#ifndef HArrayWrapper_HH__
#define HArrayWrapper_HH__

#include "HArrayMath.hh"

namespace hops
{

/*
*
*@file HArrayWrapper.hh
*@class HArrayWrapper
*@brief
*@details
*
*<b>Revision History:<b>
*Date Name Brief Description
*Sat Aug 24 12:52:33 CEST 2013 J. Barrett (barrettj@mit.edu) First Version
*
*/


template< typename ArrayType, std::size_t NDIM>
class HArrayWrapper
{
    public:

        HArrayWrapper()
        {
            for(std::size_t i=0; i<NDIM; i++)
            {
                fBases[i] = 0;
                fDimensions[i] = 0;
            }
            fTotalArraySize = 0;
        }

        HArrayWrapper(ArrayType* data, const std::size_t* dim)
        {
            fData = data;

            for(std::size_t i=0; i<NDIM; i++)
            {
                fBases[i] = 0;
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HArrayMath::TotalArraySize<NDIM>(fDimensions);
        }

        virtual ~HArrayWrapper(){;};

        void SetData(ArrayType* ptr){fData = ptr;}
        ArrayType* GetData(){return fData;};
        const ArrayType* GetData() const {return fData;};

        std::size_t GetArraySize() const {return HArrayMath::TotalArraySize<NDIM>(fDimensions); };

        void SetArrayDimensions(const std::size_t* array_dim)
        {
            for(std::size_t i=0; i<NDIM; i++)
            {
                fDimensions[i] = array_dim[i];
            }

            fTotalArraySize = HArrayMath::TotalArraySize<NDIM>(fDimensions);

        }

        void GetArrayDimensions(std::size_t* array_dim) const
        {
            for(std::size_t i=0; i<NDIM; i++)
            {
                array_dim[i] = fDimensions[i];
            }
        }

        const std::size_t* GetArrayDimensions() const
        {
            return fDimensions;
        }

        std::size_t GetArrayDimension(std::size_t dim_index) const
        {
            return fDimensions[dim_index];
        }

        void SetArrayBases(const int* array_bases)
        {
            for(std::size_t i=0; i<NDIM; i++)
            {
                fBases[i] = array_bases[i];
            }
        }

        void GetArrayBases(int* array_bases) const
        {
            for(std::size_t i=0; i<NDIM; i++)
            {
                array_bases[i] = fBases[i];
            }
        }


        std::size_t GetOffsetForIndices(const int* index)
        {
            std::size_t index_proxy[NDIM];
            for(std::size_t i=0; i<NDIM; i++)
            {
                index_proxy[i] = (index[i] - fBases[i]);
            }
            return HArrayMath::OffsetFromRowMajorIndex<NDIM>(fDimensions, index_proxy);
        }


        ArrayType& operator[](const int* index);

        const ArrayType& operator[](const int* index) const;

        ArrayType& operator[](std::size_t index);

        const ArrayType& operator[](std::size_t index) const;


    private:

        ArrayType* fData; //raw pointer to multidimensional array
        std::size_t fDimensions[NDIM];
        int fBases[NDIM];

        std::size_t fTotalArraySize;

        

};

template<typename ArrayType, std::size_t NDIM>
ArrayType& HArrayWrapper<ArrayType,NDIM>::operator[](const int* index)
{
    std::size_t index_proxy[NDIM];
    for(std::size_t i=0; i<NDIM; i++)
    {
        index_proxy[i] = index[i] - fBases[i] ;
    }

//    #ifdef HArrayWrapper_DEBUG
//        std::size_t offset = HArrayMath::OffsetFromRowMajorIndex<NDIM>(fDimensions, index_proxy);
//        if( offset >= fTotalArraySize)
//        {
//            std::cout<<"HArrayWrapper[]: Warning index out of range!: "<<offset<<" > "<<fTotalArraySize<<std::endl;
//            for(std::size_t i=0; i<NDIM; i++)
//            {
//                std::cout<<"index_proxy["<<i<<"] = "<<index_proxy[i]<<std::endl;
//                std::cout<<"index["<<i<<"] = "<<index[i]<<std::endl;
//                std::cout<<"base["<<i<<"] = "<<fBases[i]<<std::endl;
//                std::cout<<"dimension size["<<i<<"] = "<<fDimensions[i]<<std::endl;
//            }
//            std::exit(1);
//        }
//    #endif

    return fData[ HArrayMath::OffsetFromRowMajorIndex<NDIM>(fDimensions, index_proxy) ];
}




template<typename ArrayType, std::size_t NDIM>
const ArrayType& HArrayWrapper<ArrayType,NDIM>::operator[](const int* index) const
{
    std::size_t index_proxy[NDIM];
    for(std::size_t i=0; i<NDIM; i++)
    {
        index_proxy[i] = index[i] - fBases[i] ;
    }
    return fData[ HArrayMath::OffsetFromRowMajorIndex<NDIM>(fDimensions, index_proxy) ];
}

template<typename ArrayType, std::size_t NDIM>
ArrayType& HArrayWrapper<ArrayType,NDIM>::operator[](std::size_t index)
{
    return fData[index];
}

template<typename ArrayType, std::size_t NDIM>
const ArrayType& HArrayWrapper<ArrayType,NDIM>::operator[](std::size_t index) const
{
    return fData[index];
}





}


#endif /* HArrayWrapper_H__ */
