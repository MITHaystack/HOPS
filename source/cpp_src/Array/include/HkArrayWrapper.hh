#ifndef HkArrayWrapper_HH__
#define HkArrayWrapper_HH__

/*
*File: HkArrayWrapper.hh
*Class: HkArrayWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:38.395Z
*Description:
* Thu 13 Aug 2020 02:53:11 PM EDT - simplified so as to handle memory management interally
* externally managed arrays should be a different class
*/

#include <vector>
#include <array>
#include "HkArrayMath.hh"

namespace hops
{

template< typename XValueType, size_t RANK>
class HkArrayWrapper
{
    public:

        HkArrayWrapper()
        {
            //dimensions not known at time of construction
            for(size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = 0;
            }
            fTotalArraySize = 0;
        }

        //data is internally allocated
        //we may want to improve this with an allocator type parameter
        HkArrayWrapper(const size_t* dim)
        {
            for(size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);
            fData.resize(fTotalArraySize);
        }

        //copy constructor
        HkArrayWrapper(const HkArrayWrapper& obj)
        {
            for(size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = obj.fDimensions[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);

            //understood that if fData in obj exists that we copy its contents
            fData.resize(fTotalArraySize);
            if(fTotalArraySize != 0)
            {
                std::copy(obj.fData.begin(), obj.fData.end(), fData.begin() );
            }
        }

        virtual ~HkArrayWrapper(){};


        void Resize(const size_t* dim)
        {
            for(size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);
            fData.resize(fTotalArraySize);
        }

        //in some cases we may need access to the underlying raw array pointer
        XValueType* GetRawData(){return &(fData[0]);};
        const XValueType* GetawData() const {return &(fData[0]);};

        //pointer to data vector
        std::vector<XValueType>* GetData(){return fData;};
        const std::vector<XValueType>* GetData() const {return fData;};

        size_t GetSize() const {return fTotalArraySize;};

        void GetDimensions(size_t* array_dim) const
        {
            for(size_t i=0; i<RANK; i++)
            {
                array_dim[i] = fDimensions[i];
            }
        }

        const size_t* GetDimensions() const
        {
            return fDimensions;
        }

        size_t GetDimension(size_t dim_index) const
        {
            return fDimensions[dim_index];
        }

        size_t GetOffsetForIndices(const size_t* index)
        {
            return HkArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, index);
        }

        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), XValueType& >::type
        operator()(XIndexTypeS...idx)
        {
            const std::array<size_t, RANK> indices = {{idx...}};
            return fData[  HkArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ];
        }

        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), const XValueType& >::type
        operator()(XIndexTypeS...idx) const
        {
            const std::array<size_t, RANK> indices = {{idx...}};
            return fData[  HkArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ];
        }

        //access operator by 1-dim index (absolute-position) into the array
        XValueType& operator[](size_t i)
        {
            return fData[i];
        }

        const XValueType& operator[](size_t i) const
        {
            return fData[i];
        }

        HkArrayWrapper& operator=(const HkArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                for(size_t i=0; i<RANK; i++)
                {
                    fDimensions[i] = rhs.fDimensions[i];
                }
                fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);

                //understood that if fData in obj exists that we copy its contents
                fData.resize(fTotalArraySize);
                if(fTotalArraySize != 0)
                {
                    std::copy(rhs.fData.begin(), rhs.fData.end(), fData.begin() );
                }
            }
        }


    protected:

        //use a vector to simplfy storage of multidimensional array
        //TODO, evaluate whether or not we want to handle different types of allocators

        std::vector< XValueType > fData;
        size_t fDimensions[RANK]; //size of each dimension
        size_t fTotalArraySize; //total size of array

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
        XValueType* GetRawData(){return &fData;};
        const XValueType* GetRawData() const {return &fData;};

        size_t GetSize() const {return 1;};

    protected:

        XValueType fData; //single value
        size_t fTotalArraySize; //total size of array
};



}//end of hops namespace


#endif /* HkArrayWrapper_HH__ */
