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

#include "HkArrayMath.hh"

namespace hops
{

template< typename XValueType, std::size_t RANK>
class HkArrayWrapper
{
    public:

        HkArrayWrapper()
        {
            //dimensions not known at time of construction
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = 0;
            }
            fTotalArraySize = 0;
        }

        //data is internally allocated
        //we may want to improve this with an allocator type parameter
        HkArrayWrapper(const std::size_t* dim)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);
            fData.resize(fTotalArraySize);
        }

        //copy constructor
        HkArrayWrapper(const HkArrayWrapper& obj)
        {
            for(std::size_t i=0; i<RANK; i++)
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


        void Resize(const std::size_t* dim)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);
            fData.resize(fTotalArraySize);
        }

        //in some cases may need access to the underlying raw array pointer
        XValueType* GetRawData(){return &(fData[0]);};
        const XValueType* GetawData() const {return &(fData[0]);};

        //pointer to data vector
        std::vector<XValueType>* GetData(){return fData;};
        const std::vector<XValueType>* GetData() const {return fData;};

        std::size_t GetArraySize() const {return fTotalArraySize;};

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



        //access operator() using axis-positions, no checks on size/dim
        template <typename ...indices >
        XValueType& operator()(size_t first, size_t... other)
        {
            size_t arr[] = {first, other...};
            return fData[  HkArrayMath::OffsetFromRowMajorIndex<>(fDimensions, arr) ];
        }

        template <typename ...indices >
        const XValueType& operator()(size_t first, size_t... other) const
        {
            size_t arr[] = {first, other...};
            return fData[  HkArrayMath::OffsetFromRowMajorIndex<>(fDimensions, arr) ];
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
                for(std::size_t i=0; i<RANK; i++)
                {
                    fDimensions[i] = rhs.fDimensions[i];
                }
                `fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);

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
        XValueType* GetRawData(){return &fData;};
        const XValueType* GetRawData() const {return &fData;};

        std::size_t GetArraySize() const {return 1;};

    protected:

        XValueType fData; //single value
        std::size_t fTotalArraySize; //total size of array
};


}//end of hops namespace


#endif /* HkArrayWrapper_HH__ */
