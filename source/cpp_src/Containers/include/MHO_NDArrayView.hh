#ifndef MHO_NDArrayView_HH__
#define MHO_NDArrayView_HH__

#include <algorithm>
#include <array>
#include <cinttypes>
#include <cmath>
#include <cstdlib>
#include <cstring> //for memset
#include <stdexcept>
#include <string>
#include <vector>

#include "MHO_BidirectionalIndexedIterator.hh"
#include "MHO_Message.hh"
#include "MHO_Meta.hh"
#include "MHO_NDArrayMath.hh"

namespace hops
{

/*!
 *@file MHO_NDArrayView.hh
 *@class MHO_NDArrayView
 *@author J. Barrett - barrettj@mit.edu
 *@date Mon Mar 28 10:47:46 2022 -0400
 *@brief MHO_NDArrayView is a class to represent a view (slice) of a n-dimensional array
 * Thu 13 Aug 2020 02:53:11 PM EDT
 */


template< typename XValueType, std::size_t RANK > class MHO_NDArrayView
{
    public:
        using value_type = XValueType;
        using index_type = std::array< std::size_t, RANK >;
        typedef std::integral_constant< std::size_t, RANK > rank;

        //constructors
        MHO_NDArrayView(XValueType* ptr, const std::size_t* dim, const std::size_t* strides) { Construct(ptr, dim, strides); };

        MHO_NDArrayView(const MHO_NDArrayView& obj) { Construct(obj.fDataPtr, &(obj.fDims[0]), &(obj.fStrides[0])); }

        //destructor
        virtual ~MHO_NDArrayView(){};

        /**
         * @brief clone functionality - Creates a deep copy of this MHO_NDArrayView object.
         * 
         * @return A new MHO_NDArrayView instance containing the same data.
         */
        MHO_NDArrayView* Clone() { return new MHO_NDArrayView(*this); }

        /**
         * @brief copy functionality, calling array view must have same shape as rhs
         * 
         * @param rhs (const MHO_NDArrayView&)
         */
        void Copy(const MHO_NDArrayView& rhs)
        {
            //check the sizes are the same
            bool ok = true;
            std::size_t j = 0;
            for(j = 0; j < RANK; j++)
            {
                if(fDims[j] != rhs.fDims[j])
                {
                    ok = false;
                    break;
                }
            }
            if(ok)
            {
                index_type idx;
                idx.fill(0);
                for(std::size_t i = 0; i < fSize; i++)
                {
                    MHO_NDArrayMath::IncrementIndices< RANK >(&(fDims[0]), &(idx[0]));
                    this->ValueAt(idx) = rhs.ValueAt(idx);
                }
            }
            else
            {
                msg_error("containers", "array view copy failed due to mismatched sizes on dimension: " << j << "." << eom);
            }
        }

        /**
         * @brief Getter for rank of the array view
         * 
         * @return std::size_t representing the rank (total size) of the array
         */
        std::size_t GetRank() const { return RANK; }

        /**
         * @brief get the total size of the array view
         * 
         * @return Current size as std::size_t.
         */
        std::size_t GetSize() const { return fSize; };

        /**
         * @brief get the dimensions/shape of the array
         * 
         * @return Pointer to an array of std::size_t representing the dimensions
         */
        const std::size_t* GetDimensions() const { return &(fDims[0]); }

        /**
         * @brief get the dimensions/shape of the array
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
         * @brief get the dimensions/shape of the array as std::array
         * 
         * @return index_type&: Reference to the dimension array
         */
        index_type GetDimensionArray() const { return fDims; }

        /**
         * @brief Getter for a single dimension dimension
         * 
         * @param idx (std::size_t)
         * @return value of the specified (idx) dimension
         */
        std::size_t GetDimension(std::size_t idx) const { return fDims[idx]; }

        /**
         * @brief get element strides
         * 
         * @return get the array of strides for the array
         */
        const std::size_t* GetStrides() const { return &(fStrides[0]); }

        /**
         * @brief Getter for strides array (fills passed array)
         * 
         * @param strd (std::size_t*) pointer to an array of size RANK
         */
        void GetStrides(std::size_t* strd) const
        {
            for(std::size_t i = 0; i < RANK; i++)
            {
                strd[i] = fStrides[i];
            }
        }

        /**
         * @brief Getter for stride array
         * 
         * @return index_type: the stride array
         */
        index_type GetStrideArray() const { return fStrides; }

        /**
         * @brief Getter for stride at index
         * 
         * @param idx (std::size_t)
         * @return stride of the dimensions specified by idx.
         */
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
                throw std::out_of_range("MHO_NDArrayView::at() indices out of range.");
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
                throw std::out_of_range("MHO_NDArrayView::at() indices out of range.");
            }
        }

        //assignment operator
        MHO_NDArrayView& operator=(const MHO_NDArrayView& rhs)
        {
            if(this != &rhs)
            {
                Construct(rhs.fDataPtr, &(rhs.fDims[0]), &(rhs.fStrides[0]));
            }
            return *this;
        }

        /**
         * @brief set all elements in the array to a certain value
         */
        void SetArray(const XValueType& obj)
        {
            auto bit = this->begin();
            auto eit = this->end();
            for(auto it = bit; it != eit; ++it)
            {
                *it = obj;
            }
        }

        /**
         * @brief set all elements in the array to zero
         */
        void ZeroArray()
        {
            auto bit = this->begin();
            auto eit = this->end();
            for(auto it = bit; it != eit; ++it)
            {
                *it = 0;
            }
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

        ////////////////////////////////////////////////////////////////////////////////

        //simple in-place compound assignment operators (mult/add/sub)//////////

        /**
         * @brief operator*= in place multiplication by a scalar factor
         */
        template< typename T >
        typename std::enable_if< std::is_same< XValueType, T >::value or std::is_integral< T >::value or
                                     std::is_floating_point< T >::value,
                                 MHO_NDArrayView& >::type inline
        operator*=(T aScalar)
        {
            auto bit = this->begin();
            auto eit = this->end();
            for(auto it = bit; it != eit; ++it)
            {
                (*it) *= aScalar;
            }
            return *this;
        }

        /**
         * @brief operator+= in place addition by a scalar amount
         */
        template< typename T >
        typename std::enable_if< std::is_same< XValueType, T >::value or std::is_integral< T >::value or
                                     std::is_floating_point< T >::value,
                                 MHO_NDArrayView& >::type inline
        operator+=(T aScalar)
        {
            auto bit = this->begin();
            auto eit = this->end();
            for(auto it = bit; it != eit; ++it)
            {
                (*it) += aScalar;
            }
            return *this;
        }

        /**
         * @brief operator-= in place subtraction by a scalar amount
         */
        template< typename T >
        typename std::enable_if< std::is_same< XValueType, T >::value or std::is_integral< T >::value or
                                     std::is_floating_point< T >::value,
                                 MHO_NDArrayView& >::type inline
        operator-=(T aScalar)
        {
            auto bit = this->begin();
            auto eit = this->end();
            for(auto it = bit; it != eit; ++it)
            {
                (*it) -= aScalar;
            }
            return *this;
        }

        /**
         * @brief operator*= in place point-wise multiplication by another array
         */
        inline MHO_NDArrayView& operator*=(const MHO_NDArrayView& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray))
            {
                throw std::out_of_range("MHO_NDArrayView::*= size mismatch.");
            }
            auto bit1 = this->begin();
            auto bit2 = anArray.begin();
            auto it1 = bit1;
            auto it2 = bit2;
            for(std::size_t i = 0; i < fSize; i++)
            {
                (*it1) *= (*it2);
                ++it1;
                ++it2;
            }
            return *this;
        }

        /**
         * @brief operator+= in place point-wise addition by another array
         */
        inline MHO_NDArrayView& operator+=(const MHO_NDArrayView& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray))
            {
                throw std::out_of_range("MHO_NDArrayView::+= size mismatch.");
            }
            auto bit1 = this->begin();
            auto bit2 = anArray.begin();
            auto it1 = bit1;
            auto it2 = bit2;
            for(std::size_t i = 0; i < fSize; i++)
            {
                (*it1) += (*it2);
                ++it1;
                ++it2;
            }
            return *this;
        }

        /**
         * @brief operator-= in place point-wise subtraction by another array
         */
        inline MHO_NDArrayView& operator-=(const MHO_NDArrayView& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray))
            {
                throw std::out_of_range("MHO_NDArrayView::-= size mismatch.");
            }
            auto bit1 = this->begin();
            auto bit2 = anArray.begin();
            auto it1 = bit1;
            auto it2 = bit2;
            for(std::size_t i = 0; i < fSize; i++)
            {
                (*it1) -= (*it2);
                ++it1;
                ++it2;
            }
            return *this;
        }

        bool CheckIndexValidity(const index_type& idx) const
        {
            return MHO_NDArrayMath::CheckIndexValidity< RANK >(&(fDims[0]), &(idx[0]));
        }

        XValueType& ValueAt(const index_type& idx)
        {
            return fDataPtr[MHO_NDArrayMath::OffsetFromStrideIndex< RANK >(&(fStrides[0]), &(idx[0]))];
        }

        const XValueType& ValueAt(const index_type& idx) const
        {
            return fDataPtr[MHO_NDArrayMath::OffsetFromStrideIndex< RANK >(&(fStrides[0]), &(idx[0]))];
        }

    protected:
        XValueType* fDataPtr;    //data for an array view is always externally managed
        index_type fDims;        //size of each dimension
        index_type fStrides;     //strides between elements in each dimension
        uint64_t fSize;          //total size of array
        mutable index_type fTmp; //temp index workspace

    private:
        //special constructor for when strides are pre-determined (not from array dimensions)
        //this is only called with building a 'SliceView'
        void Construct(XValueType* ptr, const std::size_t* dim, const std::size_t* strides)
        {
            //default construction (empty)
            for(std::size_t i = 0; i < RANK; i++)
            {
                fDims[i] = 0;
                fStrides[i] = 0;
            }
            fSize = 0;
            fDataPtr = nullptr;
            if(ptr == nullptr || dim == nullptr || strides == nullptr)
            {
                msg_error("containers", "cannot construct array slice." << eom);
                return;
            }

            fDataPtr = ptr;
            //set the dimensions, and the strides directly
            for(std::size_t i = 0; i < RANK; i++)
            {
                fDims[i] = dim[i];
                fStrides[i] = strides[i];
            }
            fSize = MHO_NDArrayMath::TotalArraySize< RANK >(&(fDims[0]));
        }

        //the iterator definitions //////////////////////////////////////////////////
    public:
        using iterator = MHO_BidirectionalIndexedIterator< XValueType, RANK >;
        using const_iterator = MHO_BidirectionalIndexedIterator< XValueType, RANK >;

        iterator begin() { return iterator(fDataPtr, 0, &(fDims[0]), &(fStrides[0])); }

        iterator end() { return iterator(fDataPtr, fSize, &(fDims[0]), &(fStrides[0])); }

        iterator iterator_at(std::size_t offset)
        {
            return iterator(fDataPtr, std::min(offset, fSize), &(fDims[0]), &(fStrides[0]));
        }

        const_iterator cbegin() const { return const_iterator(fDataPtr, 0, &(fDims[0]), &(fStrides[0])); }

        const_iterator cend() const { return const_iterator(fDataPtr, fSize, &(fDims[0]), &(fStrides[0])); }

        const_iterator citerator_at(std::size_t offset) const
        {
            return const_iterator(fDataPtr, std::min(offset, fSize), &(fDims[0]), &(fStrides[0]));
        }
};

} // namespace hops

#endif /*! MHO_NDArrayView_HH__ */
