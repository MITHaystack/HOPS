#ifndef MHO_NDArrayWrapper_HH__
#define MHO_NDArrayWrapper_HH__

#include <algorithm>
#include <array>
#include <cinttypes>
#include <cmath>
#include <cstdlib>
#include <cstring> //for memset
#include <stdexcept>
#include <string>
#include <vector>

#include "MHO_BidirectionalIterator.hh"
#include "MHO_BidirectionalStrideIterator.hh"
#include "MHO_ExtensibleElement.hh"
#include "MHO_Message.hh"
#include "MHO_Meta.hh"
#include "MHO_NDArrayMath.hh"

#include "MHO_NDArrayView.hh"

namespace hops
{

/*!
 *@file MHO_NDArrayWrapper.hh
 *@class MHO_NDArrayWrapper
 *@author J. Barrett - barrettj@mit.edu
 *@date Mon May 11 15:51:26 2020 -0400
 *@brief MHO_NDArrayWrapper is a class to represent a n-dimensional array (implemented around a 1-d memory array)
 *
 * Thu 13 Aug 2020 02:53:11 PM EDT
 */

/**
 * @brief Class MHO_NDArrayWrapper
 */
template< typename XValueType, std::size_t RANK >
class MHO_NDArrayWrapper
    : public MHO_ExtensibleElement //any and all extensions are purely a runtime concept and do NOT get streamed for I/O
{
    public:
        using value_type = XValueType;
        using index_type = std::array< std::size_t, RANK >;
        typedef std::integral_constant< std::size_t, RANK > rank;

        //constructors
        //empty constructor, to be configured later
        MHO_NDArrayWrapper() { Construct(nullptr, nullptr); };

        //data is internally allocated
        MHO_NDArrayWrapper(const std::size_t* dim) { Construct(nullptr, dim); };

        //data is interally allocated, but copied in from an external source
        MHO_NDArrayWrapper(XValueType* ptr, const std::size_t* dim) { Construct(ptr, dim); };

        //copy constructor
        MHO_NDArrayWrapper(const MHO_NDArrayWrapper& obj)
        {
            Construct(nullptr, &(obj.fDims[0]));
            std::copy(obj.fData.begin(), obj.fData.end(), fData.begin());
        }

    public:
        //destructor
        virtual ~MHO_NDArrayWrapper(){};

        /**
         * @brief clone functionality - creates a deep copy of this MHO_NDArrayWrapper object.
         *
         * @return A new MHO_NDArrayWrapper instance containing a clone of this object's data.
         */
        MHO_NDArrayWrapper* Clone() { return new MHO_NDArrayWrapper(*this); }

        //resize function -- destroys contents
        /**
         * @brief Destroys contents and resizes using passed dimensions.
         *
         * @param dim Pointer to external dimension array.
         * @note This is a virtual function.
         */
        virtual void Resize(const std::size_t* dim) { Construct(nullptr, dim); }

        /**
         * @brief Resize function that destroys contents and resizes according to dimension arguments
         *
         * @tparam XDimSizeTypeS Template parameter XDimSizeTypeS
         * @param dim Input dimensions as variadic template XDimSizeTypeS...
         * @return void
         */
        template< typename... XDimSizeTypeS >
        typename std::enable_if< (sizeof...(XDimSizeTypeS) == RANK), void >::type Resize(XDimSizeTypeS... dim)
        {
            fTmp = {{static_cast< size_t >(dim)...}}; //convert the arguments to an array
            Resize(&(fTmp[0]));
        }

        /**
         * @brief set data pointer to externally managed array with associated dimensions
         *
         * @param ptr Pointer to externally managed XValueType array
         * @param dim Array of dimension sizes
         */
        void SetExternalData(XValueType* ptr, const std::size_t* dim) { Construct(ptr, dim); }

        /**
         * @brief Getter for the rank (dimensionality) of the array
         *
         * @return std::size_t representing the rank of the array
         */
        std::size_t GetRank() const { return RANK; }

        /**
         * @brief get the total size of the array
         *
         * @return The number of elements in the data.
         */
        std::size_t GetSize() const { return fData.size(); };

        /**
         * @brief get the dimensions/shape of the array
         *
         * @return Pointer to std::size_t array representing dimensions
         */
        const std::size_t* GetDimensions() const { return &(fDims[0]); }

        /**
         * @brief Getter for dimensions, fills passed array
         *
         * @return Pointer to std::size_t array
         */
        void GetDimensions(std::size_t* dim) const
        {
            for(std::size_t i = 0; i < RANK; i++)
            {
                dim[i] = fDims[i];
            }
        }

        /**
         * @brief Getter for dimension array
         *
         * @return index_type&: Reference to the dimension array.
         */
        index_type GetDimensionArray() const { return fDims; }

        /**
         * @brief Getter for dimension
         *
         * @param param (std::size_t)
         * @return Pointer to the first element of the dimension array.
         */
        std::size_t GetDimension(std::size_t idx) const
        {
            if(idx < RANK)
            {
                return fDims[idx];
            }
            else
            {
                throw std::out_of_range("MHO_NDArrayWrapper::GetDimension() index out of range.");
            }
        }

        /**
         * @brief Getter for element strides (along each dimension)
         *
         * @return Pointer to the first element of fStrides array
         */
        const std::size_t* GetStrides() const { return &(fStrides[0]); }

        /**
         * @brief Getter for strides (along each dimension), fills passed array
         *
         * @return Pointer to std::size_t array
         */
        void GetStrides(std::size_t* strd) const
        {
            for(std::size_t i = 0; i < RANK; i++)
            {
                strd[i] = fStrides[i];
            }
        }

        index_type GetStrideArray() const { return fStrides; }

        std::size_t GetStride(std::size_t idx) const { return fStrides[idx]; }

        /**
         * @brief access operator, accepts multiple indices (,,...,) but does no bounds checking
         * @details - uses std::enable_if to do a compile-time check that the number of arguments is the same as the rank of the array
         * @param ... varargs The variable arguments (integers) representing the data element indexes
         * @return the element at the specified indexes
         */
        template< typename... XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), XValueType& >::type operator()(XIndexTypeS... idx)
        {
            fTmp = {{static_cast< size_t >(idx)...}};
            return ValueAt(fTmp);
        }

        /**
         * @brief const reference access operator, accepts multiple indices (,,...,) but does no bounds checking
         * @details - uses std::enable_if to do a compile-time check that the number of arguments is the same as the rank of the array
         * @param ... varargs The variable arguments (integers) representing the data element indexes
         * @return a const reference to element at the specified indexes
         */
        template< typename... XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), const XValueType& >::type
        operator()(XIndexTypeS... idx) const
        {
            fTmp = {{static_cast< size_t >(idx)...}};
            return ValueAt(fTmp);
        }

        /**
         * @brief at(): same as operator(...) but with bounds checking with bounds checking
         * @details - uses std::enable_if to do a compile-time check that the number of arguments is the same as the rank of the array
         * @param ... varargs The variable arguments (integers) representing the data element indexes
         * @return  the element at the specified indexes,throws exception if it doesn't exist
         */
        template< typename... XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), XValueType& >::type at(XIndexTypeS... idx)
        {
            //make sure the indices are valid for the given array dimensions
            fTmp = {{static_cast< size_t >(idx)...}};
            if(CheckIndexValidity(fTmp))
            {
                return ValueAt(fTmp);
            }
            else
            {
                throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");
            }
        }

        /**
         * @brief at(): same as const operator(...) but with bounds checking with bounds checking
         * @details - uses std::enable_if to do a compile-time check that the number of arguments is the same as the rank of the array
         * @param ... varargs The variable arguments (integers) representing the data element indexes
         * @return a const reference to element at the specified indexes, throws exception if it doesn't exist
         */
        template< typename... XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), const XValueType& >::type at(XIndexTypeS... idx) const
        {
            //make sure the indices are valid for the given array dimensions
            fTmp = {{static_cast< size_t >(idx)...}};
            if(CheckIndexValidity(fTmp))
            {
                return ValueAt(fTmp);
            }
            else
            {
                throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");
            }
        }

        /**
         * @brief access to underlying data pointer (unsafe)
         */
        XValueType* GetData() { return &(fData[0]); };

        const XValueType* GetData() const { return &(fData[0]); };

        //fast access operator by 1-dim index (absolute-position) into the array
        //this assumes the data is contiguous in memory, which may not be true
        //if the array wrapper is a slice of a larger array
        XValueType& operator[](std::size_t i) { return fData[i]; }

        const XValueType& operator[](std::size_t i) const { return fData[i]; }

        //assignment operator
        MHO_NDArrayWrapper& operator=(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                Construct(nullptr, &(rhs.fDims[0]));
                std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin());
            }
            return *this;
        }

        /**
         * @brief set all elements in the array to a certain value
         */
        void SetArray(const XValueType& obj)
        {
            for(auto it = fData.begin(); it != fData.end(); it++)
            {
                *it = obj;
            }
        }

        /**
         * @brief set all elements in the array to zero
         */
        void ZeroArray() { std::memset(&(fData[0]), 0, (fData.size()) * sizeof(XValueType)); }

        //copy, effectively the same as assignment operator
        virtual void Copy(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                Construct(nullptr, &(rhs.fDims[0]));
                std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin());
            }
        }

        //copy, but from an array view with the same rank
        virtual void Copy(const MHO_NDArrayView< XValueType, RANK >& rhs)
        {
            auto dims = rhs.GetDimensionArray();
            Construct(nullptr, &(dims[0]));
            std::copy(rhs.cbegin(), rhs.cend(), this->fData.begin());
        }

        /**
         * @brief compute (memory) offset into array from a set of indexes
         */
        std::size_t GetOffsetForIndices(const std::size_t* index)
        {
            return MHO_NDArrayMath::OffsetFromStrideIndex< RANK >(&(fStrides[0]), index);
        }

        /**
         * @brief invert (memory) offset into array to indexes of the associated element
         */
        index_type GetIndicesForOffset(std::size_t offset)
        {
            index_type index;
            MHO_NDArrayMath::RowMajorIndexFromOffset< RANK >(offset, &(fDims[0]), &(index[0]));
            return index;
        }

        /**
         * @brief creates a sub-view of the array (given n < RANK leading indexes), return the remaining
         * chunk of the array with freely spanning indexes
         * for example: a ndarray X of RANK=3, and sizes [4,12,32], then SubView(2)
         * returns an MHO_NDArrayView of RANK=2, and dimensions [12,32] starting at the
         * location of X(2,0,0). Data of the subview points to data owned by X
         */
        template< typename... XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) < RANK),
                                 MHO_NDArrayView< XValueType, RANK - (sizeof...(XIndexTypeS)) > >::type
        SubView(XIndexTypeS... idx)
        {
            constexpr typename std::integral_constant< std::size_t, sizeof...(XIndexTypeS) > nfixed_t;
            std::array< std::size_t, sizeof...(XIndexTypeS) > leading_idx = {{static_cast< size_t >(idx)...}};
            for(std::size_t i = 0; i < RANK; i++)
            {
                fTmp[i] = 0;
            }
            for(std::size_t i = 0; i < leading_idx.size(); i++)
            {
                fTmp[i] = leading_idx[i];
            }
            std::size_t offset = MHO_NDArrayMath::OffsetFromStrideIndex< RANK >(&(fStrides[0]), &(fTmp[0]));
            return MHO_NDArrayView< XValueType, RANK - (sizeof...(XIndexTypeS)) >(&(fData[offset]), &(fDims[nfixed_t]),
                                                                                  &(fStrides[nfixed_t]));
        }

        ////////////////////////////////////////////////////////////////////////////////

        /**
         * @brief creates a slice-view of the array (given n < RANK indexes), return the remaining
         * chunk of the array with freely spanning indexes
         * the placeholder for the free-spanning indexes is the character ":"
         * for example: a ndarray X of RANK=3, and sizes [4,12,32], then SliceView(":",3,":")
         * returns an MHO_NDArrayView of RANK=2, and dimensions [4,32] starting at the
         * location of X(0,3,0), and spanning the data covered by X(":",3,":")
         * Data of the slice-view points to data owned by original array X
         */
        template< typename... XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK),
                                 MHO_NDArrayView< XValueType, count_instances_of_type< const char*, sizeof...(XIndexTypeS) - 1,
                                                                                       XIndexTypeS... >::value > >::type
        SliceView(XIndexTypeS... idx)
        {
            constexpr typename std::integral_constant<
                std::size_t, count_instances_of_type< const char*, sizeof...(XIndexTypeS) - 1, XIndexTypeS... >::value >
                nfree_t;

            class index_filler
            {
                public:
                    index_filler()
                    {
                        for(std::size_t i = 0; i < RANK; i++)
                        {
                            full_idx[i] = 0;
                        }
                        fixed_idx.clear();
                        free_idx.clear();
                    }

                    ~index_filler(){};

                    std::array< std::size_t, RANK > full_idx; //list the index values of the start of the slice
                    std::vector< std::size_t > fixed_idx;     //list the indexes which are fixed
                    std::vector< std::size_t > free_idx;      //list the indexs which are free to vary

                    //placeholder type sets index to zero
                    void operator()(std::size_t i, const char* /*!value*/)
                    {
                        full_idx[i] = 0;
                        free_idx.push_back(i);
                    }

                    //index types pass along their value
                    void operator()(std::size_t i, std::size_t value)
                    {
                        full_idx[i] = value;
                        fixed_idx.push_back(i);
                    }

                    //make sure the indexes are listed in increasing order
                    void reorder()
                    {
                        std::sort(free_idx.begin(), free_idx.end());
                        std::sort(fixed_idx.begin(), fixed_idx.end()); //make sure they are in increasing order
                    }
            };

            index_filler filler;
            std::tuple< XIndexTypeS... > input_idx = std::make_tuple(idx...);
            indexed_tuple_visit< RANK >::visit(input_idx, filler);
            filler.reorder();

            std::size_t offset = MHO_NDArrayMath::OffsetFromRowMajorIndex< RANK >(&(fDims[0]), &(filler.full_idx[0]));

            std::array< std::size_t, nfree_t > dim;
            std::array< std::size_t, nfree_t > strides;
            for(std::size_t i = 0; i < dim.size(); i++)
            {
                dim[i] = fDims[filler.free_idx[i]];
                strides[i] = fStrides[filler.free_idx[i]];
            }
            return MHO_NDArrayView< XValueType, nfree_t >(&(fData[offset]), &(dim[0]), &(strides[0]));
        }

        ////////////////////////////////////////////////////////////////////////////////

        //simple in-place compound assignment operators (mult/add/sub)//////////

        /**
         * @brief operator*= in place multiplication by a scalar factor
         */
        template< typename T >
        typename std::enable_if< std::is_same< XValueType, T >::value or std::is_integral< T >::value or
                                     std::is_floating_point< T >::value,
                                 MHO_NDArrayWrapper& >::type inline
        operator*=(T aScalar)
        {
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] *= aScalar;
            }
            return *this;
        }

        /**
         * @brief operator+= in place addition by a scalar amount
         */
        template< typename T >
        typename std::enable_if< std::is_same< XValueType, T >::value or std::is_integral< T >::value or
                                     std::is_floating_point< T >::value,
                                 MHO_NDArrayWrapper& >::type inline
        operator+=(T aScalar)
        {
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] += aScalar;
            }
            return *this;
        }

        /**
         * @brief operator+= in place addition by a scalar amount
         */
        template< typename T >
        typename std::enable_if< std::is_same< XValueType, T >::value or std::is_integral< T >::value or
                                     std::is_floating_point< T >::value,
                                 MHO_NDArrayWrapper& >::type inline
        operator-=(T aScalar)
        {
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] -= aScalar;
            }
            return *this;
        }

        /**
         * @brief operator*= in place point-wise multiplication by another array
         */
        inline MHO_NDArrayWrapper& operator*=(const MHO_NDArrayWrapper& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray))
            {
                throw std::out_of_range("MHO_NDArrayWrapper::*= size mismatch.");
            }
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] *= anArray.fData[i];
            }
            return *this;
        }

        /**
         * @brief operator+= in place point-wise addition by another array
         */
        inline MHO_NDArrayWrapper& operator+=(const MHO_NDArrayWrapper& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray))
            {
                throw std::out_of_range("MHO_NDArrayWrapper::+= size mismatch.");
            }
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] += anArray.fData[i];
            }
            return *this;
        }

        /**
         * @brief operator-= in place point-wise subtraction by another array
         */
        inline MHO_NDArrayWrapper& operator-=(const MHO_NDArrayWrapper& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray))
            {
                throw std::out_of_range("MHO_NDArrayWrapper::-= size mismatch.");
            }
            std::size_t length = fData.size();
            for(std::size_t i = 0; i < length; i++)
            {
                fData[i] -= anArray.fData[i];
            }
            return *this;
        }

        bool CheckIndexValidity(const index_type& idx) const
        {
            return MHO_NDArrayMath::CheckIndexValidity< RANK >(&(fDims[0]), &(idx[0]));
        }

        XValueType& ValueAt(const index_type& idx)
        {
            return fData[MHO_NDArrayMath::OffsetFromStrideIndex< RANK >(&(fStrides[0]), &(idx[0]))];
        }

        const XValueType& ValueAt(const index_type& idx) const
        {
            return fData[MHO_NDArrayMath::OffsetFromStrideIndex< RANK >(&(fStrides[0]), &(idx[0]))];
        }

    private:
        std::vector< XValueType > fData; //used for internally managed data
        index_type fDims;                //size of each dimension
        index_type fStrides;             //strides between elements in each dimension
        mutable index_type fTmp;         //temp index workspace

        void Construct(XValueType* ptr, const std::size_t* dim)
        {
            //default construction (empty)
            for(std::size_t i = 0; i < RANK; i++)
            {
                fDims[i] = 0;
                fStrides[0] = 0;
            }
            if(ptr == nullptr && dim == nullptr)
            {
                return;
            }

            //dimensions known, so create array but don't fill it
            if(dim != nullptr)
            {
                for(std::size_t i = 0; i < RANK; i++)
                {
                    fDims[i] = dim[i];
                }
            }
            std::size_t length = MHO_NDArrayMath::TotalArraySize< RANK >(&(fDims[0]));
            ComputeStrides();
            fData.resize(length);

            if(ptr != nullptr) //if given a ptr, copy in data from this location
            {
                std::memcpy(&(fData[0]), ptr, length * sizeof(XValueType));
            }
        }

        void ComputeStrides()
        {
            for(std::size_t i = 0; i < RANK; i++)
            {
                //stride for elements of this dimension
                std::size_t stride = 1;
                std::size_t j = RANK - 1;
                while(j > i)
                {
                    stride *= fDims[j];
                    j--;
                }
                fStrides[i] = stride;
            }
        }

        //the iterator definitions //////////////////////////////////////////////////
    public:
        using iterator = MHO_BidirectionalIterator< XValueType >;
        using stride_iterator = MHO_BidirectionalStrideIterator< XValueType >;

        using const_iterator = MHO_BidirectionalConstIterator< XValueType >;
        using const_stride_iterator = MHO_BidirectionalConstStrideIterator< XValueType >;

        iterator begin() { return iterator(&(fData[0]), &(fData[0]), fData.size()); }

        iterator end() { return iterator(&(fData[0]), &(fData[0]) + fData.size(), fData.size()); }

        iterator iterator_at(std::size_t offset)
        {
            return iterator(&(fData[0]), &(fData[0]) + std::min(offset, fData.size()), fData.size());
        }

        const_iterator cbegin() const { return const_iterator(&(fData[0]), &(fData[0]), fData.size()); }

        const_iterator cend() const { return const_iterator(&(fData[0]), &(fData[0]) + fData.size(), fData.size()); }

        const_iterator citerator_at(std::size_t offset) const
        {
            return const_iterator(&(fData[0]), &(fData[0]) + std::min(offset, fData.size()), fData.size());
        }

        stride_iterator stride_begin(std::size_t stride)
        {
            return stride_iterator(&(fData[0]), &(fData[0]), fData.size(), stride);
        }

        stride_iterator stride_end(std::size_t stride)
        {
            return stride_iterator(&(fData[0]), &(fData[0]) + fData.size(), fData.size(), stride);
        }

        stride_iterator stride_iterator_at(std::size_t offset, std::size_t stride)
        {
            return stride_iterator(&(fData[0]), &(fData[0]) + std::min(offset, fData.size()), fData.size(), stride);
        }

        const_stride_iterator cstride_begin(std::size_t stride) const
        {
            return const_stride_iterator(&(fData[0]), &(fData[0]), fData.size(), stride);
        }

        const_stride_iterator cstride_end(std::size_t stride) const
        {
            return const_stride_iterator(&(fData[0]), &(fData[0]) + fData.size(), fData.size(), stride);
        }

        const_stride_iterator cstride_iterator_at(std::size_t offset, std::size_t stride) const
        {
            return const_stride_iterator(&(fData[0]), &(fData[0]) + std::min(offset, fData.size()), fData.size(), stride);
        }
};

} // namespace hops

//include the partial specializations for RANK=0 and RANK=1
#include "MHO_NDArrayWrapper_0.hh"
#include "MHO_NDArrayWrapper_1.hh"

namespace hops
{

//utilities ////////////////////////////////////////////////////////////////////
/**
 * @brief Checks if two arrays have the same rank.
 *
 * @tparam XArrayType1 Template parameter XArrayType1
 * @tparam XArrayType2 Template parameter XArrayType2
 * @param !arr1 Parameter description
 * @param !arr2 Parameter description
 * @return True if both arrays have the same rank, false otherwise.
 * @note This is a static function.
 */
template< class XArrayType1, class XArrayType2 >
static bool HaveSameRank(const XArrayType1* /*!arr1*/, const XArrayType2* /*!arr2*/)
{
    return (XArrayType1::rank::value == XArrayType2::rank::value);
}

/**
 * @brief Checks if two arrays have the same number of elements.
 *
 * @tparam XArrayType1 Template parameter XArrayType1
 * @tparam XArrayType2 Template parameter XArrayType2
 * @param arr1 First array to compare
 * @param arr2 Second array to compare
 * @return True if both arrays have the same size, false otherwise.
 * @note This is a static function.
 */
template< class XArrayType1, class XArrayType2 >
static bool HaveSameNumberOfElements(const XArrayType1* arr1, const XArrayType2* arr2)
{
    return (arr1->GetSize() == arr2->GetSize());
}

/**
 * @brief Function HaveSameDimensions
 *
 * @tparam XArrayType1 Template parameter XArrayType1
 * @tparam XArrayType2 Template parameter XArrayType2
 * @param arr1 (const XArrayType1*)
 * @param arr2 (const XArrayType2*)
 * @return Return value (bool)
 * @note This is a static function.
 */
template< class XArrayType1, class XArrayType2 >
static bool HaveSameDimensions(const XArrayType1* arr1, const XArrayType2* arr2)
{
    std::size_t shape1[XArrayType1::rank::value];
    std::size_t shape2[XArrayType2::rank::value];

    if(HaveSameRank(arr1, arr2))
    {
        size_t rank = XArrayType1::rank::value;
        arr1->GetDimensions(shape1);
        arr2->GetDimensions(shape2);

        for(std::size_t i = 0; i < rank; i++)
        {
            if(shape1[i] != shape2[i])
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

} // namespace hops

#endif /*! MHO_NDArrayWrapper_HH__ */
