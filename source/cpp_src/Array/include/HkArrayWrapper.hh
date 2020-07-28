#ifndef HkArrayWrapper_HH__
#define HkArrayWrapper_HH__

/*
*File: HkArrayWrapper.hh
*Class: HkArrayWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:38.395Z
*Description:
*/

#include "HkArrayMath.hh"

namespace hops
{

template< typename XValueType, std::size_t RANK>
class HkArrayWrapper
{
    public:

        HkArrayWrapper()
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = 0;
            }
            fTotalArraySize = 0;
        }

        //for the time being the data array is owned/managed externally,
        //we may want to improve this with an allocator parameter
        HkArrayWrapper(XValueType* data, const std::size_t* dim)
        {
            fData = data;
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);
        }

        virtual ~HkArrayWrapper(){;};

        void SetData(XValueType* ptr){fData = ptr;}

        XValueType* GetData(){return fData;};
        const XValueType* GetData() const {return fData;};

        std::size_t GetArraySize() const {return HkArrayMath::TotalArraySize<RANK>(fDimensions); };

        void SetArrayDimensions(const std::size_t* array_dim)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = array_dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);
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
            return HkArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, index);
        }

        //TODO, implement [] operator overload for multi-dim array
        //TODO, implement == operator, copy constructor, and clone

    protected:

        XValueType* fData; //raw pointer to multidimensional array
        std::size_t fDimensions[RANK]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array
};


//specialization for a RANK-0 (i.e. a scalar)
template< typename XValueType >
class HkArrayWrapper<XValueType, 0>
{
    public:

        HkArrayWrapper()
        {
            fTotalArraySize = 1;
        }

        HkArrayWrapper(const XValueType& data)
        {
            fData = data;
            fTotalArraySize = 1;
        }

        virtual ~HkArrayWrapper(){};

        void SetData(const XValueType& value){fData = value;}
        XValueType GetData(){return fData;};

        std::size_t GetArraySize() const {return 1;};

    protected:

        XValueType fData; //single value
        std::size_t fTotalArraySize; //total size of array
};

//specialization for RANK=1, (i.e. a vector)
template< typename XValueType >
class HkArrayWrapper<XValueType, 1>
{
    public:

        HkArrayWrapper()
        {
            fDimensions[0] = 0;
            fTotalArraySize = 0;
        }

        HkArrayWrapper(XValueType* data, const std::size_t* dim)
        {
            fData = data;
            fDimensions[0] = dim[0];
            fTotalArraySize = HkArrayMath::TotalArraySize<1>(fDimensions);
        }

        virtual ~HkArrayWrapper(){;};

        void SetData(XValueType* ptr){fData = ptr;}
        XValueType* GetData(){return fData;};
        const XValueType* GetData() const {return fData;};

        std::size_t GetArraySize() const {return HkArrayMath::TotalArraySize<1>(fDimensions); };

        void SetArrayDimensions(const std::size_t* array_dim)
        {
            fDimensions[0] = array_dim[0];
            fTotalArraySize = HkArrayMath::TotalArraySize<1>(fDimensions);
        }

        void GetArrayDimensions(std::size_t* array_dim) const
        {
            array_dim[0] = fDimensions[0];
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
            return HkArrayMath::OffsetFromRowMajorIndex<1>(fDimensions, index);
        }

    protected:

        XValueType* fData; //raw pointer to multidimensional array
        std::size_t fDimensions[1]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array
};


}//end of hops namespace


#endif /* HkArrayWrapper_HH__ */
