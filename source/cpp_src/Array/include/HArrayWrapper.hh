#ifndef HArrayWrapper_HH__
#define HArrayWrapper_HH__

#include "HArrayMath.hh"

namespace hops
{

template< typename ArrayType, std::size_t RANK>
class HArrayWrapper
{
    public:

        HArrayWrapper()
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = 0;
            }
            fTotalArraySize = 0;
        }

        HArrayWrapper(ArrayType* data, const std::size_t* dim)
        {
            fData = data;
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HArrayMath::TotalArraySize<RANK>(fDimensions);
        }

        virtual ~HArrayWrapper(){;};

        void SetData(ArrayType* ptr){fData = ptr;}
        ArrayType* GetData(){return fData;};
        const ArrayType* GetData() const {return fData;};

        std::size_t GetArraySize() const {return HArrayMath::TotalArraySize<RANK>(fDimensions); };

        void SetArrayDimensions(const std::size_t* array_dim)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = array_dim[i];
            }
            fTotalArraySize = HArrayMath::TotalArraySize<RANK>(fDimensions);
        }

        void GetArrayDimensions(std::size_t* array_dim) const
        {
            for(std::size_t i=0; i<RANK; i++)
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

        std::size_t GetOffsetForIndices(const size_t* index)
        {
            return HArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, index);
        }

        //TODO, implement [] operator overload for multi-dim array
        //TODO, implement == operator, copy constructor, and clone

    private:

        ArrayType* fData; //raw pointer to multidimensional array
        std::size_t fDimensions[RANK]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array
};





}


#endif /* HArrayWrapper_H__ */
