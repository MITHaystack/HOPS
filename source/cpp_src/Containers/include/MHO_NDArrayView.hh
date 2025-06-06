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
 *@brief
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

        //clone functionality
        MHO_NDArrayView* Clone() { return new MHO_NDArrayView(*this); }

        //copy functionality, calling array view must have same shape as rhs
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

        //get the total size of the array
        std::size_t GetRank() const { return RANK; }

        std::size_t GetSize() const { return fSize; };

        //get the dimensions/shape of the array
        const std::size_t* GetDimensions() const { return &(fDims[0]); }

        void GetDimensions(std::size_t* dim) const
        {
            for(std::size_t i = 0; i < RANK; i++)
            {
                dim[i] = fDims[i];
            }
        }

        index_type GetDimensionArray() const { return fDims; }

        std::size_t GetDimension(std::size_t idx) const { return fDims[idx]; }

        //get element strides
        const std::size_t* GetStrides() const { return &(fStrides[0]); }

        void GetStrides(std::size_t* strd) const
        {
            for(std::size_t i = 0; i < RANK; i++)
            {
                strd[i] = fStrides[i];
            }
        }

        index_type GetStrideArray() const { return fStrides; }

        std::size_t GetStride(std::size_t idx) const { return fStrides[idx]; }

        //access operator (,,...,) -- no bounds checking
        //std::enable_if does a compile-time check that the number of arguments is the same as the rank of the array
        template< typename... XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), XValueType& >::type operator()(XIndexTypeS... idx)
        {
            fTmp = {{static_cast< size_t >(idx)...}};
            return ValueAt(fTmp);
        }

        //const reference access operator()
        template< typename... XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), const XValueType& >::type
        operator()(XIndexTypeS... idx) const
        {
            fTmp = {{static_cast< size_t >(idx)...}};
            return ValueAt(fTmp);
        }

        //access via at(,,,,) -- same as operator() but with bounds checking
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

        //const at()
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

        //set all elements in the array to a certain value
        void SetArray(const XValueType& obj)
        {
            auto bit = this->begin();
            auto eit = this->end();
            for(auto it = bit; it != eit; ++it)
            {
                *it = obj;
            }
        }

        //set all elements in the array to zero
        void ZeroArray()
        {
            auto bit = this->begin();
            auto eit = this->end();
            for(auto it = bit; it != eit; ++it)
            {
                *it = 0;
            }
        }

        //linear offset into the array
        std::size_t GetOffsetForIndices(const std::size_t* index)
        {
            return MHO_NDArrayMath::OffsetFromStrideIndex< RANK >(&(fStrides[0]), index);
        }

        //linear offset into the array
        index_type GetIndicesForOffset(std::size_t offset)
        {
            index_type index;
            MHO_NDArrayMath::RowMajorIndexFromOffset< RANK >(offset, &(fDims[0]), &(index[0]));
            return index;
        }

        ////////////////////////////////////////////////////////////////////////////////

        //simple in-place compound assignment operators (mult/add/sub)//////////

        //in place multiplication by a scalar factor
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

        //in place addition by a scalar amount
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

        //in place subraction by a scalar amount
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

        //in place point-wise multiplication by another array
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

        //in place point-wise addition by another array
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

        //in place point-wise subtraction of another array
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
