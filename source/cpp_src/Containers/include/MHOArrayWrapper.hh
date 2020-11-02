#ifndef MHOArrayWrapper_HH__
#define MHOArrayWrapper_HH__

/*
*File: MHOArrayWrapper.hh
*Class: MHOArrayWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:38.395Z
*Description:
* Thu 13 Aug 2020 02:53:11 PM EDT - simplified so as to handle memory management interally
* externally managed arrays should be a different class
*/

#include <vector>
#include <array>
#include <stdexcept>

#include "MHOArrayMath.hh"

namespace hops
{

template< typename XValueType, std::size_t RANK>
class MHOArrayWrapper
{
    public:

        //empty constructor, to be configured later
        MHOArrayWrapper()
        {
            //dimensions not known at time of construction
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = 0;
            }
            fTotalArraySize = 0;
            fDataPtr = nullptr;
            fExternallyManaged = false;
        }

        //data is externally allocated - we take no responsiblity to 
        //delete the data pointed to by ptr upon destruction
        MHOArrayWrapper(XValueType* ptr, const std::size_t* dim)
        {
            //dimensions not known at time of construction
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = MHOArrayMath::TotalArraySize<RANK>(fDimensions);
            fDataPtr = ptr;
            fExternallyManaged = true;
        }

        //data is internally allocated
        MHOArrayWrapper(const std::size_t* dim)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = MHOArrayMath::TotalArraySize<RANK>(fDimensions);
            fData.resize(fTotalArraySize);
            fDataPtr = &(fData[0]);
            fExternallyManaged = false;
        }

        //copy constructor
        MHOArrayWrapper(const MHOArrayWrapper& obj)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = obj.fDimensions[i];
            }
            fTotalArraySize = MHOArrayMath::TotalArraySize<RANK>(fDimensions);

            if(obj.fExternallyManaged)
            {
                //cheap copy, just point to externally managed data
                fDataPtr = obj.fDataPtr;
                fExternallyManaged = true;
            }
            else 
            {
                //expensive copy, must allocate new space anc copy contents
                fExternallyManaged = false;
            
                fData.resize(fTotalArraySize);
                if(fTotalArraySize != 0)
                {
                    std::copy(obj.fData.begin(), obj.fData.end(), fData.begin() );
                }
            }
        }

        virtual ~MHOArrayWrapper(){};


        void Resize(const std::size_t* dim)
        {
            if(fExternallyManaged)
            {
                //we cannot re-size an externally managed array
                //so instead we issue a warning and reconfigure 
                //our state to use an internally managed array
                msg_warn("containers", "Resize operation called on a wrapper to 
                          an exernally managed array, replacing with internally 
                          managed memory. This may result in unexpected behavior." << eom);
            }

            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = MHOArrayMath::TotalArraySize<RANK>(fDimensions);
            fData.resize(fTotalArraySize);
            fDataPtr = &(fData[0]);
            fExternallyManaged = false;
        }

        //resize function so we can give each dim as an individual std::size_t (rather than array)
        template <typename ...XDimSizeTypeS >
        typename std::enable_if< (sizeof...(XDimSizeTypeS) == RANK), void >::type //compile-time check that the number of arguments is the same as the rank of the array
        Resize(XDimSizeTypeS...dim)
        {
            const std::array<std::size_t, RANK> dim_sizes = {{static_cast<size_t>(dim)...}}; //convert the arguments to an array
            Resize(&(dim_sizes[0]));
        }

        //in some cases we may need access 
        //to the underlying raw array pointer
        XValueType* GetData(){return fDataPtr;};
        const XValueType* GetData() const {return fDataPtr;};

        std::size_t GetSize() const {return fTotalArraySize;};

        void GetDimensions(std::size_t* array_dim) const
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                array_dim[i] = fDimensions[i];
            }
        }

        const std::size_t* GetDimensions() const
        {
            return fDimensions;
        }

        std::size_t GetDimension(std::size_t dim_index) const
        {
            return fDimensions[dim_index];
        }

        std::size_t GetOffsetForIndices(const std::size_t* index)
        {
            return MHOArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, index);
        }


        //access operator (,,...,) -- no bounds checking
        template <typename ...XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), XValueType& >::type //compile-time check that the number of arguments is the same as the rank of the array
        operator()(XIndexTypeS...idx)
        {
            const std::array<std::size_t, RANK> indices = {{static_cast<size_t>(idx)...}}; //convert the arguments to an array
            return fDataPtr[ MHOArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ]; //compute the offset into the array and return reference to the data
        }

        //const reference access operator()
        template <typename ...XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK),  const XValueType& >::type
        operator()(XIndexTypeS...idx) const
        {
            const std::array<std::size_t, RANK> indices = {{static_cast<size_t>(idx)...}};
            return fDataPtr[ MHOArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ];
        }

        //access via at(,,,,) -- same as operator() but with bounds checking
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), XValueType& >::type
        at(XIndexTypeS...idx)
        {
            const std::array<std::size_t, RANK> indices = {{static_cast<size_t>(idx)...}};
            if( MHOArrayMath::CheckIndexValidity<RANK>( fDimensions, &(indices[0]) ) ) //make sure the indices are valid for the given array dimensions
            {
                return fDataPtr[ MHOArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ];
            }
            else
            {
                throw std::out_of_range("MHOArrayWrapper::at() indices out of range.");
            }
        }

        //const at()
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), const XValueType& >::type
        at(XIndexTypeS...idx) const
        {
            const std::array<std::size_t, RANK> indices = {{static_cast<size_t>(idx)...}};
            if( MHOArrayMath::CheckIndexValidity<RANK>( fDimensions, &(indices[0]) ) )
            {
                return fDataPtr[  MHOArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ];
            }
            else
            {
                throw std::out_of_range("MHOArrayWrapper::at() indices out of range.");
            }
        }

        //fast access operator by 1-dim index (absolute-position) into the array
        XValueType& operator[](std::size_t i){return fDataPtr[i];}
        const XValueType& operator[](std::size_t i) const {return fDataPtr[i];}

        MHOArrayWrapper& operator=(const MHOArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                if(rhs.fExternallyManaged)
                {
                    //cheap copies for externally managed arrays 
                    //just copy dimensions and ptr

                    for(std::size_t i=0; i<RANK; i++)
                    {
                        fDimensions[i] = rhs.fDimensions[i];
                    }
                    fTotalArraySize = MHOArrayMath::TotalArraySize<RANK>(fDimensions);
                    fDataPtr = rhs.fDataPtr;
                    fExternallyManaged = true;

                    //effectively de-allocate anything we might have had  
                    std::vector< XValueType >().swap(fData)
                }
                else 
                {
                    Resize(rhs.fDimensions);
                    //understood that if fData in obj exists 
                    //then we also copy its contents
                    if(fTotalArraySize != 0)
                    {
                        std::copy(rhs.fData.begin(), rhs.fData.end(), fData.begin() );
                    }
                    fDataPtr = &(fData[0]);
                    fExternallyManaged = false;
                }
            }
        }


    protected:

        XValueType fDataPtr; 
        bool fExternallyManaged;
        std::vector< XValueType > fData; //used for internally managed data
        std::size_t fDimensions[RANK]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array

};

















//specialization for a RANK-0 (i.e. a scalar)
template< typename XValueType >
class MHOArrayWrapper<XValueType, 0>
{
    public:

        MHOArrayWrapper()
        {
            fTotalArraySize = 1;
        }

        //copy constructor
        MHOArrayWrapper(const MHOArrayWrapper& obj)
        {
            fTotalArraySize = 1;
            fData = obj.fData;
        }

        MHOArrayWrapper(const XValueType& data)
        {
            fData = data;
            fTotalArraySize = 1;
        }

        virtual ~MHOArrayWrapper(){};

        void SetData(const XValueType& value){fData = value;}
        XValueType GetData(){return fData;};

        std::size_t GetSize() const {return 1;};

        MHOArrayWrapper& operator=(const MHOArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                fData = rhs.fData;
            }
        }

    protected:

        XValueType fData; //single value
        std::size_t fTotalArraySize; //total size of array
};












































//specialization for a RANK-1 (i.e. a vector)
template< typename XValueType >
class MHOArrayWrapper<XValueType, 1>
{
    public:

        MHOArrayWrapper()
        {
            fDimensions[0] = 0;
            fTotalArraySize = 0;
            fData.resize(0);
        }

        //data is internally allocated
        //we may want to improve this with an allocator type parameter
        MHOArrayWrapper(std::size_t dim)
        {
            fDimensions[0] = dim;
            fTotalArraySize = dim;
            fData.resize(fTotalArraySize);
        }

        //copy constructor
        MHOArrayWrapper(const MHOArrayWrapper& obj)
        {
            fDimensions[0] = obj.fDimensions[0];
            fTotalArraySize = obj.fTotalArraySize;
            if(fTotalArraySize != 0)
            {
                std::copy(obj.fData.begin(), obj.fData.end(), fData.begin() );
            }
        }

        virtual ~MHOArrayWrapper(){};

        void Resize(const std::size_t* dim)
        {
            fDimensions[0] = dim[0];
            fTotalArraySize = dim[0];
            fData.resize(fTotalArraySize);
        }

        void Resize(std::size_t dim)
        {
            fDimensions[0] = dim;
            fTotalArraySize = dim;
            fData.resize(fTotalArraySize);
        }

        //in some cases we may need access to the underlying raw array pointer
        XValueType* GetRawData(){return &(fData[0]);};
        const XValueType* GetRawData() const {return &(fData[0]);};

        //pointer to data vector
        std::vector<XValueType>* GetData(){return fData;};
        const std::vector<XValueType>* GetData() const {return fData;};

        std::size_t GetSize() const {return fTotalArraySize;};

        void GetDimensions(std::size_t* array_dim) const
        {
            for(std::size_t i=0; i<1; i++)
            {
                array_dim[i] = fDimensions[i];
            }
        }

        const std::size_t* GetDimensions() const
        {
            return fDimensions;
        }

        std::size_t GetDimension(std::size_t dim_index) const
        {
            return fDimensions[dim_index];
        }

        std::size_t GetOffsetForIndices(const std::size_t* index)
        {
            return MHOArrayMath::OffsetFromRowMajorIndex<1>(fDimensions, index);
        }

        //TODO fix narrowing warning on conversion of XIndexTypeS to std::size_t
        //access operator (,,,) -- no bounds checking
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == 1), XValueType& >::type
        operator()(XIndexTypeS...idx)
        {
            const std::array<std::size_t, 1> indices = {{static_cast<size_t>(idx)...}};
            return fData[ indices[0] ];
        }

        //const operator()
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == 1), const XValueType& >::type
        operator()(XIndexTypeS...idx) const
        {
            const std::array<std::size_t, 1> indices = {{static_cast<size_t>(idx)...}};
            return fData[ indices[0] ];
        }

        //access via at(,,,,) -- TODO, make this include bounds checking on each dimension!
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == 1), XValueType& >::type
        at(XIndexTypeS...idx)
        {
            const std::array<std::size_t, 1> indices = {{static_cast<size_t>(idx)...}};
            if( MHOArrayMath::CheckIndexValidity<1>( fDimensions, &(indices[0]) ) )
            {
                return fData[ indices[0] ];
            }
            else
            {
                throw std::out_of_range("MHOArrayWrapper::at() indices out of range.");
            }
        }

        //const at()
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == 1), const XValueType& >::type
        at(XIndexTypeS...idx) const
        {
            const std::array<std::size_t, 1> indices = {{static_cast<size_t>(idx)...}};
            if( MHOArrayMath::CheckIndexValidity<1>( fDimensions, &(indices[0]) ) )
            {
                return fData[ indices[0] ];
            }
            else
            {
                throw std::out_of_range("MHOArrayWrapper::at() indices out of range.");
            }
        }

        //access operator by 1-dim index (absolute-position) into the array
        XValueType& operator[](std::size_t i)
        {
            return fDataPtr[i];
        }

        const XValueType& operator[](std::size_t i) const
        {
            return fDataPtr[i];
        }

        MHOArrayWrapper& operator=(const MHOArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                fDimensions[0] = rhs.fDimensions[0];
                fTotalArraySize = rhs.fTotalArraySize;
                fData.resize(fTotalArraySize);
                if(fTotalArraySize != 0)
                {
                    std::copy(rhs.fData.begin(), rhs.fData.end(), fData.begin() );
                }
            }
        }

    protected:

        XValueType fDataPtr; 
        bool fExternallyManaged;
        std::vector< XValueType > fData; //used for internally managed data
        std::size_t fDimensions[1]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array
};


}//end of hops namespace


#endif /* MHOArrayWrapper_HH__ */
