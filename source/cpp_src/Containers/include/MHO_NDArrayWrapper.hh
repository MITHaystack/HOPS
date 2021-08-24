#ifndef MHO_NDArrayWrapper_HH__
#define MHO_NDArrayWrapper_HH__

/*
*File: MHO_NDArrayWrapper.hh
*Class: MHO_NDArrayWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:38.395Z
*Description:
* Thu 13 Aug 2020 02:53:11 PM EDT 
*/

#include <cstring> //for memset

#include <vector>
#include <array>
#include <stdexcept>
#include <iterator>
#include <cstdlib>
#include <cmath>
#include <cinttypes>

#include "MHO_NDArrayMath.hh"
#include "MHO_Message.hh"

namespace hops
{

template< typename XValueType, std::size_t RANK>
class MHO_NDArrayWrapper
{
    public:

        using value_type = XValueType;
        typedef std::integral_constant< std::size_t, RANK > rank;

        //empty constructor, to be configured later
        MHO_NDArrayWrapper()
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
        MHO_NDArrayWrapper(XValueType* ptr, const std::size_t* dim)
        {
            //dimensions not known at time of construction
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = MHO_NDArrayMath::TotalArraySize<RANK>(fDimensions);
            fDataPtr = ptr;
            fExternallyManaged = true;
        }

        //data is internally allocated
        MHO_NDArrayWrapper(const std::size_t* dim)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = MHO_NDArrayMath::TotalArraySize<RANK>(fDimensions);
            fData.resize(fTotalArraySize);
            fDataPtr = &(fData[0]);
            fExternallyManaged = false;
        }

        //copy constructor
        MHO_NDArrayWrapper(const MHO_NDArrayWrapper& obj)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = obj.fDimensions[i];
            }
            fTotalArraySize = MHO_NDArrayMath::TotalArraySize<RANK>(fDimensions);

            if(obj.fExternallyManaged)
            {
                //cheap copy, just point to externally managed data
                fDataPtr = obj.fDataPtr;
                fExternallyManaged = true;
            }
            else
            {
                //expensive copy, must allocate new space and copy contents
                fData.resize(fTotalArraySize);
                fDataPtr = &(fData[0]);
                fExternallyManaged = false;
                if(fTotalArraySize != 0)
                {
                    std::copy(obj.fData.begin(), obj.fData.end(), fData.begin() );
                }
            }
        }

        virtual ~MHO_NDArrayWrapper()
        {
            //all internal memory management is handled by std::vector
            //external arrays are not managed by this class at all
        };

        virtual void Resize(const std::size_t* dim)
        {
            if(fExternallyManaged)
            {
                //we cannot re-size an externally managed array
                //so instead we issue a warning and reconfigure
                //our state to use an internally managed array
                msg_warn("containers", "Resize operation called on a wrapper pointing to " <<
                          "an exernally managed array will replace it with internally " <<
                          "managed memory. This may result in unexpected behavior." << eom);
            }

            for(std::size_t i=0; i<RANK; i++)
            {
                msg_debug("containers", "resizing dimension: "<<i<<" to: "<<dim[i] << eom );
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = MHO_NDArrayMath::TotalArraySize<RANK>(fDimensions);
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

        //set pointer to externally managed array with associated dimensions
        void SetExternalData(XValueType* ptr, const std::size_t* dim)
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                fDimensions[i] = dim[i];
            }
            fTotalArraySize = MHO_NDArrayMath::TotalArraySize<RANK>(fDimensions);

            if(!fExternallyManaged) //not currently already externally managed
            {
                //effectively de-allocate anything we might have had before
                std::vector< XValueType >().swap(fData);
            }
            fDataPtr = ptr;
            fExternallyManaged = true;
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

        std::size_t GetStride(std::size_t dim_index) const
        {
            //stride for elements of this dimension 
            std::size_t stride = 1;
            std::size_t i = RANK-1;
            while(i > dim_index)
            {
                stride *= fDimensions[i];
                i--;
            }
            return stride;
        }

        void GetStrides(std::size_t* array_stride) const
        {
            for(std::size_t i=0; i<RANK; i++){array_stride[i] = 0;}
            std::size_t stride = 1;
            std::size_t i = RANK-1;
            while(i > 0)
            {
                array_stride[i] = stride;
                stride *= fDimensions[i];
                i--;
            }
        }

        std::size_t GetOffsetForIndices(const std::size_t* index)
        {
            return MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, index);
        }

        //access operator (,,...,) -- no bounds checking
        template <typename ...XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), XValueType& >::type //compile-time check that the number of arguments is the same as the rank of the array
        operator()(XIndexTypeS...idx)
        {
            const std::array<std::size_t, RANK> indices = {{static_cast<size_t>(idx)...}}; //convert the arguments to an array
            return fDataPtr[ MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ]; //compute the offset into the array and return reference to the data
        }

        //const reference access operator()
        template <typename ...XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK),  const XValueType& >::type
        operator()(XIndexTypeS...idx) const
        {
            const std::array<std::size_t, RANK> indices = {{static_cast<size_t>(idx)...}};
            return fDataPtr[ MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ];
        }

        //access via at(,,,,) -- same as operator() but with bounds checking
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), XValueType& >::type
        at(XIndexTypeS...idx)
        {
            const std::array<std::size_t, RANK> indices = {{static_cast<size_t>(idx)...}};
            if( MHO_NDArrayMath::CheckIndexValidity<RANK>( fDimensions, &(indices[0]) ) ) //make sure the indices are valid for the given array dimensions
            {
                return fDataPtr[ MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ];
            }
            else
            {
                throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");
            }
        }

        //const at()
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), const XValueType& >::type
        at(XIndexTypeS...idx) const
        {
            const std::array<std::size_t, RANK> indices = {{static_cast<size_t>(idx)...}};
            if( MHO_NDArrayMath::CheckIndexValidity<RANK>( fDimensions, &(indices[0]) ) )
            {
                return fDataPtr[  MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) ) ];
            }
            else
            {
                throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");
            }
        }

        //fast access operator by 1-dim index (absolute-position) into the array
        XValueType& operator[](std::size_t i){return fDataPtr[i];}
        const XValueType& operator[](std::size_t i) const {return fDataPtr[i];}

        MHO_NDArrayWrapper& operator=(const MHO_NDArrayWrapper& rhs)
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
                    fTotalArraySize = MHO_NDArrayMath::TotalArraySize<RANK>(fDimensions);
                    fDataPtr = rhs.fDataPtr;
                    fExternallyManaged = true;
                    //effectively de-allocate anything we might have had before
                    std::vector< XValueType >().swap(fData);
                }
                else
                {
                    Resize(rhs.fDimensions);
                    //also copy its contents of data array
                    if(fTotalArraySize != 0)
                    {
                        std::copy(rhs.fData.begin(), rhs.fData.end(), fData.begin() );
                    }
                    fDataPtr = &(fData[0]);
                    fExternallyManaged = false;
                }
            }
            return *this;
        }

        //set all of the elements in an array to be equal to the object obj
        void
        SetArray(const XValueType& obj)
        {
            for(std::size_t i=0; i < fTotalArraySize; i++)
            {
                fDataPtr[i] = obj;
            }
        }

        //set all of the elements in an array to be equal to zero
        void
        ZeroArray()
        {
            XValueType* ptr = fDataPtr;
            std::size_t n_bytes = fTotalArraySize*( sizeof(XValueType) );
            std::memset(ptr, 0, n_bytes);
        }


    protected:

        XValueType* fDataPtr;
        bool fExternallyManaged;
        std::vector< XValueType > fData; //used for internally managed data
        std::size_t fDimensions[RANK]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array


    //the iterator definition //////////////////////////////////////////////////
    public:

        //TODO - Do we need a const_iterator class?
        class iterator
        {
            public:

                typedef iterator self_type;
                typedef XValueType value_type;
                typedef XValueType& reference;
                typedef XValueType* pointer;
                typedef std::forward_iterator_tag iterator_category;
                typedef int difference_type;
                typedef std::array<std::size_t, RANK> index_type;

                iterator(bool valid, pointer ptr, std::size_t* dim, std::size_t offset):
                    fValid(valid),
                    fPtr(ptr),
                    fDimensions(dim)
                {
                    //initialize the multi-dim indices
                    MHO_NDArrayMath::RowMajorIndexFromOffset<RANK>(offset, fDimensions, &(fIndices[0]) );
                };

                iterator(const self_type& copy)
                {
                    fValid = copy.fValid;
                    fPtr = copy.fPtr;
                    fDimensions = copy.fDimensions;
                    fIndices = copy.fIndices;
                };

                self_type operator++()
                {
                    fPtr++;
                    fValid = MHO_NDArrayMath::IncrementIndices<RANK>(fDimensions, &(fIndices[0]) );
                    return *this;
                }

                self_type operator--()
                {
                    fPtr--;
                    fValid = MHO_NDArrayMath::DecrementIndices<RANK>(fDimensions, &(fIndices[0]) );
                    return *this;
                }

                self_type operator++(int)
                {
                    self_type ret_val(*this);
                    fPtr++;
                    fValid = MHO_NDArrayMath::IncrementIndices<RANK>(fDimensions, &(fIndices[0]) );
                    return ret_val;
                }

                self_type operator--(int)
                {
                    self_type ret_val(*this);
                    fPtr--;
                    fValid = MHO_NDArrayMath::DecrementIndices<RANK>(fDimensions, &(fIndices[0]) );
                    return ret_val;
                }

                std::ptrdiff_t operator-(const self_type& iter)
                {
                    return std::distance(iter.GetPtr(), fPtr);
                }

                self_type operator+=(const std::ptrdiff_t& diff)
                {
                    fPtr += diff;
                    if(diff >= 0)
                    {
                        fValid = MHO_NDArrayMath::IncrementIndices<RANK>(fDimensions, &(fIndices[0]), (std::size_t)diff );
                    }
                    else
                    {
                        fValid = MHO_NDArrayMath::DecrementIndices<RANK>(fDimensions, &(fIndices[0]), (std::size_t) std::abs(diff) );
                    }
                    return (*this);
                }

                self_type operator-=(const std::ptrdiff_t& diff)
                {
                    fPtr -= diff;
                    if(diff >= 0)
                    {
                        fValid = MHO_NDArrayMath::DecrementIndices<RANK>(fDimensions, &(fIndices[0]), (std::size_t) diff );
                    }
                    else
                    {
                        fValid = MHO_NDArrayMath::IncrementIndices<RANK>(fDimensions, &(fIndices[0]), (std::size_t) std::abs(diff) );
                    }
                    return (*this);
                }

                self_type operator+(const std::ptrdiff_t& diff)
                {
                    pointer oldPtr = fPtr;
                    index_type oldIndices = fIndices;
                    bool oldValid = fValid;

                    fPtr += diff;
                    if(diff >= 0)
                    {
                        fValid = MHO_NDArrayMath::IncrementIndices<RANK>(fDimensions, &(fIndices[0]), (std::size_t)diff );
                    }
                    else
                    {
                        fValid = MHO_NDArrayMath::DecrementIndices<RANK>(fDimensions, &(fIndices[0]), (std::size_t) std::abs(diff) );
                    }
                    self_type temp(*this);

                    fPtr = oldPtr;
                    fIndices = oldIndices;
                    fValid = oldValid;

                    return temp;
                }

                self_type operator-(const std::ptrdiff_t& diff)
                {
                    pointer oldPtr = fPtr;
                    index_type oldIndices = fIndices;
                    bool oldValid = fValid;

                    fPtr -= diff;
                    if(diff >= 0)
                    {
                        fValid = MHO_NDArrayMath::DecrementIndices<RANK>(fDimensions, &(fIndices[0]), (std::size_t) diff );
                    }
                    else
                    {
                        fValid = MHO_NDArrayMath::IncrementIndices<RANK>(fDimensions, &(fIndices[0]), (std::size_t) std::abs(diff) );
                    }
                    self_type temp(*this);

                    fPtr = oldPtr;
                    fIndices = oldIndices;
                    fValid = oldValid;

                    return temp;
                }


                //access to underlying array item object
                reference operator*() { return *fPtr; }
                pointer operator->() { return fPtr; }

                self_type operator=(const self_type& rhs)
                {
                    if(this != &rhs)
                    {
                        fValid = rhs.fValid;
                        fPtr == rhs.fPtr;
                        fDimensions = rhs.fDimensions;
                        fIndices = rhs.fIndices;
                    }
                    return *this;
                }

                bool operator==(const self_type& rhs)
                {
                    return fPtr == rhs.fPtr;
                }

                bool operator!=(const self_type& rhs)
                {
                    return fPtr != rhs.fPtr;
                }

                pointer GetPtr(){return fPtr;}
                index_type GetIndexObject() const {return fIndices;}
                const std::size_t* GetIndices() const {return &(fIndices[0]);}
                bool IsValid() const {return fValid;}

            private:

                bool fValid;
                pointer fPtr;
                std::size_t* fDimensions;
                index_type fIndices;
        };


////////////////////////////////////////////////////////////////////////////////


//the strided iterator definition
public:

    //strided access to the array (useful for iterating along a single dimension)
    class stride_iterator
    {
        public:

            typedef stride_iterator self_type;
            typedef XValueType value_type;
            typedef XValueType& reference;
            typedef XValueType* pointer;
            typedef std::forward_iterator_tag iterator_category;
            typedef int difference_type;
            typedef std::array<std::size_t, RANK> index_type;

            stride_iterator(const iterator& iter, std::size_t stride):
                fIterator(iter), //initial location
                fStride(stride) //stride distance by which to access the array 
            {
                //TODO FIXME, check we have no overflow issues with (size_t -> ptrdiff_t
            };

            stride_iterator(const self_type& copy):
                fIterator(copy.fIterator)
            {
                fStride = copy.fStride;
            };

            self_type operator++()
            {
                fIterator += fStride;
                return *this;
            }

            self_type operator--()
            {
                fIterator -= fStride;
                return *this;
            }

            self_type operator++(int)
            {
                self_type ret_val(*this);
                ++(*this);
                return ret_val;
            }

            self_type operator--(int)
            {
                self_type ret_val(*this);
                ++(*this);
                return ret_val;
            }

            std::ptrdiff_t operator-(const self_type& iter)
            {
                return std::distance(iter.GetPtr(), fIterator.GetPtr());
            }

            self_type operator+=(const std::ptrdiff_t& diff)
            {
                fIterator += fStride*diff;
                return (*this);
            }

            self_type operator-=(const std::ptrdiff_t& diff)
            {
                fIterator -= fStride*diff;
                return (*this);
            }

            self_type operator+(const std::ptrdiff_t& diff)
            {
                self_type temp(*this);
                temp += diff;
                return temp;
            }

            self_type operator-(const std::ptrdiff_t& diff)
            {
                self_type temp(*this);
                temp -= diff;
                return temp;
            }

            //access to underlying array item object
            reference operator*() { return *(fIterator->GetPtr()); }
            pointer operator->() { return fIterator->GetPtr(); }

            self_type operator=(const self_type& rhs)
            {
                if(this != &rhs)
                {
                    fIterator = rhs.fIterator;
                    fStride = rhs.fStride;
                }
                return *this;
            }

            bool operator==(const self_type& rhs)
            {
                return (fIterator == rhs.fIterator && fStride == rhs.fStride);
            }

            bool operator!=(const self_type& rhs)
            {
                return !(*this == rhs);
            }

            pointer GetPtr(){return fIterator.GetPtr();}
            index_type GetIndexObject() const {return fIterator.GetIndices();}
            const std::size_t* GetIndices() const {return fIterator.GetIndices();}
            bool IsValid() const {return fIterator.IsValid();}

        private:

            iterator fIterator;
            ptrdiff_t fStride; 
    };


    public:

        iterator begin()
        {
            return iterator(true, this->fDataPtr, this->fDimensions, 0);
        }

        iterator end()
        {
            return iterator(false, this->fDataPtr + this->fTotalArraySize, this->fDimensions, this->fTotalArraySize);
        }

        iterator iterator_at(std::size_t offset)
        {
            if(offset < this->fTotalArraySize)
            {
                return iterator(true, this->fDataPtr + offset, this->fDimensions, 0);
            }
            else
            {
                return this->end();
            }
        }

        // //access via iterator_at(,,,,) -- multiple indices
        // template <typename ...XIndexTypeS >
        // typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), iterator >::type
        // iterator_at(XIndexTypeS...idx)
        // {
        //     const std::array<std::size_t, RANK> indices = {{static_cast<size_t>(idx)...}};
        //     std::size_t offset = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(fDimensions, &(indices[0]) )
        //     return this->iterator_at(offset);
        // }

        stride_iterator stride_begin(std::size_t stride)
        {
            iterator tmp(true, this->fDataPtr, this->fDimensions, 0);
            return stride_iterator(tmp, stride);
        }

        stride_iterator stride_end(std::size_t stride)
        {
            iterator tmp(false, this->fDataPtr + this->fTotalArraySize, this->fDimensions, this->fTotalArraySize);
            return stride_iterator(tmp, stride);
        }

        stride_iterator stride_iterator_at(std::size_t offset, std::size_t stride)
        {
            iterator tmp = this->iterator_at(offset);
            return stride_iterator(tmp,stride);
        }

};





//specialization for a RANK-0 (i.e. a scalar)
template< typename XValueType >
class MHO_NDArrayWrapper<XValueType, 0>
{
    public:

        using value_type = XValueType;
        typedef std::integral_constant< std::size_t, 0 > rank;

        MHO_NDArrayWrapper()
        {
            fTotalArraySize = 1;
        }

        //copy constructor
        MHO_NDArrayWrapper(const MHO_NDArrayWrapper& obj)
        {
            fTotalArraySize = 1;
            fData = obj.fData;
        }

        MHO_NDArrayWrapper(const XValueType& data)
        {
            fData = data;
            fTotalArraySize = 1;
        }

        virtual ~MHO_NDArrayWrapper(){};

        void SetData(const XValueType& value){fData = value;}
        XValueType GetData(){return fData;};

        std::size_t GetSize() const {return 1;};

        MHO_NDArrayWrapper& operator=(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                fData = rhs.fData;
            }
            return *this;
        }

        //set all of the elements in an array to be equal to the object obj
        void
        SetArray(const XValueType& obj)
        {
            fData = obj;
        }

        //set all of the elements in an array to be equal to zero
        void
        ZeroArray()
        {
            XValueType* ptr = &fData;
            std::size_t n_bytes = sizeof(XValueType);
            std::memset(ptr, 0, n_bytes);
        }

    protected:

        XValueType fData; //single value
        std::size_t fTotalArraySize; //total size of array
};



//specialization for a RANK-1 (i.e. a vector)
template< typename XValueType >
class MHO_NDArrayWrapper<XValueType, 1>
{
    public:

        using value_type = XValueType;
        typedef std::integral_constant< std::size_t, 1 > rank;

        MHO_NDArrayWrapper()
        {
            //dimensions not known at time of construction
            fDimensions[0] = 0;
            fTotalArraySize = 0;
            fDataPtr = nullptr;
            fExternallyManaged = false;
        }

        //data is externally allocated - we take no responsiblity to
        //delete the data pointed to by ptr upon destruction
        MHO_NDArrayWrapper(XValueType* ptr, std::size_t dim)
        {
            //dimensions not known at time of construction
            fDimensions[0] = dim;
            fTotalArraySize = fDimensions[0];
            fDataPtr = ptr;
            fExternallyManaged = true;
        }

        //data is internally allocated
        //we may want to improve this with an allocator type parameter
        MHO_NDArrayWrapper(std::size_t dim)
        {
            fDimensions[0] = dim;
            fTotalArraySize = dim;
            fData.resize(fTotalArraySize);
            fDataPtr = &(fData[0]);
            fExternallyManaged = false;
        }

        //copy constructor
        MHO_NDArrayWrapper(const MHO_NDArrayWrapper& obj)
        {
            fDimensions[0] = obj.fDimensions[0];
            fTotalArraySize = obj.fTotalArraySize;
            if(obj.fExternallyManaged)
            {
                fDataPtr = obj.fDataPtr;
                fExternallyManaged = true;
            }
            else
            {
                fData.resize(fTotalArraySize);
                if(fTotalArraySize != 0)
                {
                    std::copy(obj.fData.begin(), obj.fData.end(), fData.begin() );
                }
                fDataPtr = &(fData[0]);
                fExternallyManaged = false;
            }
        }

        virtual ~MHO_NDArrayWrapper(){};

        void Resize(const std::size_t* dim)
        {
            if(fExternallyManaged)
            {
                //we cannot re-size an externally managed array
                //so instead we issue a warning and reconfigure
                //our state to use an internally managed array
                msg_warn("containers", "Resize operation called on a wrapper pointing to " <<
                          "an exernally managed array will replace it with internally " <<
                          "managed memory. This may result in unexpected behavior." << eom);
            }

            fDimensions[0] = dim[0];
            fTotalArraySize = fDimensions[0];
            fData.resize(fTotalArraySize);
            fDataPtr = &(fData[0]);
            fExternallyManaged = false;
        }

        void Resize(std::size_t dim)
        {
            if(fExternallyManaged)
            {
                //we cannot re-size an externally managed array
                //so instead we issue a warning and reconfigure
                //our state to use an internally managed array
                msg_warn("containers", "Resize operation called on a wrapper pointing to " <<
                          "an exernally managed array will replace it with internally " <<
                          "managed memory. This may result in unexpected behavior." << eom);
            }

            fDimensions[0] = dim;
            fTotalArraySize = fDimensions[0];
            fData.resize(fTotalArraySize);
            fDataPtr = &(fData[0]);
            fExternallyManaged = false;
        }

        //in some cases we may need access to the underlying raw array pointer
        XValueType* GetData(){return fDataPtr;};
        const XValueType* GetData() const {return fDataPtr;};

        std::size_t GetSize() const {return fTotalArraySize;};

        void GetDimensions(std::size_t* array_dim) const
        {
            array_dim[0] = fDimensions[0];

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
            return MHO_NDArrayMath::OffsetFromRowMajorIndex<1>(fDimensions, index);
        }

        //TODO fix narrowing warning on conversion of XIndexTypeS to std::size_t
        //access operator (,,,) -- no bounds checking
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == 1), XValueType& >::type
        operator()(XIndexTypeS...idx)
        {
            const std::array<std::size_t, 1> indices = {{static_cast<size_t>(idx)...}};
            return fDataPtr[ indices[0] ];
        }

        //const operator()
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == 1), const XValueType& >::type
        operator()(XIndexTypeS...idx) const
        {
            const std::array<std::size_t, 1> indices = {{static_cast<size_t>(idx)...}};
            return fDataPtr[ indices[0] ];
        }

        //access via at(,,,,) -- TODO, make this include bounds checking on each dimension!
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == 1), XValueType& >::type
        at(XIndexTypeS...idx)
        {
            const std::array<std::size_t, 1> indices = {{static_cast<size_t>(idx)...}};
            if( MHO_NDArrayMath::CheckIndexValidity<1>( fDimensions, &(indices[0]) ) )
            {
                return fDataPtr[ indices[0] ];
            }
            else
            {
                throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");
            }
        }

        //const at()
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == 1), const XValueType& >::type
        at(XIndexTypeS...idx) const
        {
            const std::array<std::size_t, 1> indices = {{static_cast<size_t>(idx)...}};
            if( MHO_NDArrayMath::CheckIndexValidity<1>( fDimensions, &(indices[0]) ) )
            {
                return fDataPtr[ indices[0] ];
            }
            else
            {
                throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");
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

        MHO_NDArrayWrapper& operator=(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                if(rhs.fExternallyManaged)
                {
                    //cheap copies for externally managed arrays
                    //just copy dimensions and ptr
                    fDimensions[0] = rhs.fDimensions[0];
                    fTotalArraySize = fDimensions[0];
                    fDataPtr = rhs.fDataPtr;
                    fExternallyManaged = true;
                    //effectively de-allocate anything we might have had before
                    std::vector< XValueType >().swap(fData);
                }
                else
                {
                    Resize(rhs.fDimensions);
                    //also copy its contents of data array
                    if(fTotalArraySize != 0)
                    {
                        std::copy(rhs.fData.begin(), rhs.fData.end(), fData.begin() );
                    }
                    fDataPtr = &(fData[0]);
                    fExternallyManaged = false;
                }
            }
            return *this;
        }

        //set all of the elements in an array to be equal to the object obj
        void
        SetArray(const XValueType& obj)
        {
            for(std::size_t i=0; i < fTotalArraySize; i++)
            {
                fDataPtr = obj;
            }
        }

        //set all of the elements in an array to be equal to zero
        void
        ZeroArray()
        {
            XValueType* ptr = fDataPtr;
            std::size_t n_bytes = fTotalArraySize*( sizeof(XValueType) );
            std::memset(ptr, 0, n_bytes);
        }

    protected:

        XValueType* fDataPtr;
        bool fExternallyManaged;
        std::vector< XValueType > fData; //used for internally managed data
        std::size_t fDimensions[1]; //size of each dimension
        std::size_t fTotalArraySize; //total size of array







        //the iterator definition
        public:

            //TODO - Do we need a const_iterator class?
            class iterator
            {
                public:

                    typedef iterator self_type;
                    typedef XValueType value_type;
                    typedef XValueType& reference;
                    typedef XValueType* pointer;
                    typedef std::forward_iterator_tag iterator_category;
                    typedef int difference_type;
                    typedef std::array<std::size_t, 1> index_type;

                    iterator(bool valid, pointer ptr, std::size_t* dim, std::size_t offset):
                        fValid(valid),
                        fPtr(ptr),
                        fDimensions(dim)
                    {
                        //initialize the multi-dim indices
                        MHO_NDArrayMath::RowMajorIndexFromOffset<1>(offset, fDimensions, &(fIndices[0]) );
                    };

                    iterator(const self_type& copy)
                    {
                        fValid = copy.fValid;
                        fPtr = copy.fPtr;
                        fDimensions = copy.fDimensions;
                        fIndices = copy.fIndices;
                    };

                    self_type operator++()
                    {
                        fPtr++;
                        fValid = MHO_NDArrayMath::IncrementIndices<1>(fDimensions, &(fIndices[0]) );
                        return *this;
                    }

                    self_type operator--()
                    {
                        fPtr--;
                        fValid = MHO_NDArrayMath::DecrementIndices<1>(fDimensions, &(fIndices[0]) );
                        return *this;
                    }

                    self_type operator++(int)
                    {
                        self_type ret_val(*this);
                        fPtr++;
                        fValid = MHO_NDArrayMath::IncrementIndices<1>(fDimensions, &(fIndices[0]) );
                        return ret_val;
                    }

                    self_type operator--(int)
                    {
                        self_type ret_val(*this);
                        fPtr--;
                        fValid = MHO_NDArrayMath::DecrementIndices<1>(fDimensions, &(fIndices[0]) );
                        return ret_val;
                    }

                    std::ptrdiff_t operator-(const self_type& iter)
                    {
                        return std::distance(iter.GetPtr(), fPtr);
                    }

                    self_type operator+=(const std::ptrdiff_t& diff)
                    {
                        fPtr += diff;
                        if(diff >= 0)
                        {
                            fValid = MHO_NDArrayMath::IncrementIndices<1>(fDimensions, &(fIndices[0]), (std::size_t)diff );
                        }
                        else
                        {
                            fValid = MHO_NDArrayMath::DecrementIndices<1>(fDimensions, &(fIndices[0]), (std::size_t) std::abs(diff) );
                        }
                        return (*this);
                    }

                    self_type operator-=(const std::ptrdiff_t& diff)
                    {
                        fPtr -= diff;
                        if(diff >= 0)
                        {
                            fValid = MHO_NDArrayMath::DecrementIndices<1>(fDimensions, &(fIndices[0]), (std::size_t) diff );
                        }
                        else
                        {
                            fValid = MHO_NDArrayMath::IncrementIndices<1>(fDimensions, &(fIndices[0]), (std::size_t) std::abs(diff) );
                        }
                        return (*this);
                    }

                    self_type operator+(const std::ptrdiff_t& diff)
                    {
                        pointer oldPtr = fPtr;
                        index_type oldIndices = fIndices;
                        bool oldValid = fValid;

                        fPtr += diff;
                        if(diff >= 0)
                        {
                            fValid = MHO_NDArrayMath::IncrementIndices<1>(fDimensions, &(fIndices[0]), (std::size_t)diff );
                        }
                        else
                        {
                            fValid = MHO_NDArrayMath::DecrementIndices<1>(fDimensions, &(fIndices[0]), (std::size_t) std::abs(diff) );
                        }
                        self_type temp(*this);

                        fPtr = oldPtr;
                        fIndices = oldIndices;
                        fValid = oldValid;

                        return temp;
                    }

                    self_type operator-(const std::ptrdiff_t& diff)
                    {
                        pointer oldPtr = fPtr;
                        index_type oldIndices = fIndices;
                        bool oldValid = fValid;

                        fPtr -= diff;
                        if(diff >= 0)
                        {
                            fValid = MHO_NDArrayMath::DecrementIndices<1>(fDimensions, &(fIndices[0]), (std::size_t) diff );
                        }
                        else
                        {
                            fValid = MHO_NDArrayMath::IncrementIndices<1>(fDimensions, &(fIndices[0]), (std::size_t) std::abs(diff) );
                        }
                        self_type temp(*this);

                        fPtr = oldPtr;
                        fIndices = oldIndices;
                        fValid = oldValid;

                        return temp;
                    }


                    //access to underlying array item object
                    reference operator*() { return *fPtr; }
                    pointer operator->() { return fPtr; }

                    self_type operator=(const self_type& rhs)
                    {
                        if(this != &rhs)
                        {
                            fValid = rhs.fValid;
                            fPtr == rhs.fPtr;
                            fDimensions = rhs.fDimensions;
                            fIndices = rhs.fIndices;
                        }
                        return *this;
                    }

                    bool operator==(const self_type& rhs)
                    {
                        return fPtr == rhs.fPtr;
                    }

                    bool operator!=(const self_type& rhs)
                    {
                        return fPtr != rhs.fPtr;
                    }

                    pointer GetPtr(){return fPtr;}
                    index_type GetIndexObject() const {return fIndices;}
                    const std::size_t* GetIndices() const {return &(fIndices[0]);}
                    bool IsValid() const {return fValid;}

                private:

                    bool fValid;
                    pointer fPtr;
                    std::size_t* fDimensions;
                    index_type fIndices;
            };


        public:

            iterator begin()
            {
                return iterator(true, this->fDataPtr, this->fDimensions, 0);
            }

            iterator end()
            {
                return iterator(false, this->fDataPtr + this->fTotalArraySize, this->fDimensions, this->fTotalArraySize);
            }


















};





//utilities
template< class XArrayType1, class XArrayType2 >
static bool
HaveSameRank(const XArrayType1* /*arr1*/, const XArrayType2* /*arr2*/)
{
    return ( XArrayType1::rank::value == XArrayType2::rank::value );
}

template< class XArrayType1, class XArrayType2 >
static bool
HaveSameNumberOfElements(const XArrayType1* arr1, const XArrayType2* arr2)
{
    return ( arr1->GetSize() == arr2->GetSize() );
}

template< class XArrayType1, class XArrayType2 >
static bool
HaveSameDimensions(const XArrayType1* arr1, const XArrayType2* arr2)
{
    std::size_t shape1[XArrayType1::rank::value];
    std::size_t shape2[XArrayType2::rank::value];

    if( HaveSameRank(arr1, arr2) )
    {
        size_t rank = XArrayType1::rank::value;
        arr1->GetDimensions(shape1);
        arr2->GetDimensions(shape2);

        for(std::size_t i=0; i<rank; i++)
        {
            if(shape1[i] != shape2[i]){return false;}
        }
        return true;
    }
    return false;
}




}//end of hops namespace


#endif /* MHO_NDArrayWrapper_HH__ */
