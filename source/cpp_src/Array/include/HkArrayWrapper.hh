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
            fDataIsOwned = false;
            fData = nullptr;
        }

        //copy constructor, understood that if fData in obj exists that we copy its contents
        HkArrayWrapper( const HkArrayWrapper& obj)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = obj.fDimensions[i];
            }
            fTotalArraySize = obj.fTotalArraySize;

            if(fTotalArraySize != 0)
            {
                fData = new XValueType[fTotalArraySize];

                fDataIsOwned = true; //data is internally managed
            }
            else
            {
                fData = nullptr;
                fDataIsOwned = false;
            }
        }

        //for the time being the data array is owned/managed externally,
        HkArrayWrapper(XValueType* data, const std::size_t* dim)
        {
            fData = data;
            fDataIsOwned = false; //data is externally managed
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);
        }

        //data is internally allocated, we may want to improve this with an allocator type parameter
        HkArrayWrapper(const std::size_t* dim)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = HkArrayMath::TotalArraySize<RANK>(fDimensions);

            if(fTotalArraySize != 0)
            {
                fData = new XValueType[fTotalArraySize];
                fDataIsOwned = true; //data is internally managed
            }
            else
            {
                fData = nullptr;
                fDataIsOwned = false;
                //FIXME with proper exception
                std::cout<<"Warning, total array size is zero, data ptr assumed to be externally managed!"<<std::endl;
            }
        }


        virtual ~HkArrayWrapper()
        {
            if(fDataIsOwned)
            {
                delete[] fData;
            }
        };

        void SetData(XValueType* ptr) //should we also take an argument specifying the size?
        {
            if(fDataIsOwned)
            {
                delete[] fData;
                fData = nullptr;
                fDataIsOwned = false;
            }
            fData = ptr;
        }

        void SetData(std::vector<XValueType>* ptr) //should we also take an argument specifying the size?
        {
            if(fDataIsOwned)
            {
                delete[] fData;
                fData = nullptr;
                fDataIsOwned = false;
            }
            fData = ptr;
        }



        //in some cases may need access to the underlying array
        XValueType* GetRawData(){return &(fData[0]);};
        const XValueType* GetawData() const {return &(fData[0]);};

        //pointer to underlying vector
        std::vector<XValueType>* GetData(){return fData;};
        const std::vector<XValueType>* GetData() const {return fData;};

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
        //TODO, implement == operator, and clone

    protected:

        bool fDataIsOwned;
        std::vector< XValueType > fData; //use a vector to simplfy storage of multidimensional array
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
