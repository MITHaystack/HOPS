#ifndef MHO_NDArrayView_HH__
#define MHO_NDArrayView_HH__

/*
*File: MHO_NDArrayView.hh
*Class: MHO_NDArrayView
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:22:38.395Z
*Description:
* Thu 13 Aug 2020 02:53:11 PM EDT
*/

#include <cstring> //for memset
#include <string>
#include <vector>
#include <array>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <cinttypes>
#include <algorithm>

#include "MHO_Meta.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayMath.hh"
#include "MHO_BidirectionalIterator.hh"
#include "MHO_BidirectionalStrideIterator.hh"
#include "MHO_ExtensibleElement.hh"

namespace hops
{

template< typename XValueType, std::size_t RANK>
class MHO_NDArrayView:
    public MHO_ExtensibleElement //any and all extensions are purely a runtime concept and do NOT get streamed for I/O
{
    public:

        using value_type = XValueType;
        using index_type = std::array<std::size_t, RANK>;
        typedef std::integral_constant< std::size_t, RANK > rank;

        //constructor
        MHO_NDArrayView(XValueType* ptr, const std::size_t* dim, const std::size_t* strides){Construct(ptr,dim,strides);};

        MHO_NDArrayView(const MHO_NDArrayView& obj)
        {
            Construct(obj.fDataPtr, &(obj.fDims[0]), &(obj.fStrides[0]) );
        }

    public:

        //destructor
        virtual ~MHO_NDArrayView(){};

        //clone functionality
        MHO_NDArrayView* Clone(){ return new MHO_NDArrayView(*this); }

        //get the total size of the array
        std::size_t GetRank() const {return RANK;}
        std::size_t GetSize() const {return fSize;};

        //get the dimensions/shape of the array
        const std::size_t* GetDimensions() const {return &(fDims[0]);}
        void GetDimensions(std::size_t* dim) const { for(std::size_t i=0; i<RANK; i++){dim[i] = fDims[i];} }
        index_type GetDimensionArray() const {return fDims;}
        std::size_t GetDimension(std::size_t idx) const {return fDims[idx];}

        //get element strides
        const std::size_t* GetStrides() const {return &(fStrides[0]);}
        void GetStrides(std::size_t* strd) const { for(std::size_t i=0; i<RANK; i++){strd[i] = fStrides[i];} }
        index_type GetStrideArray() const {return fStrides;}
        std::size_t GetStride(std::size_t idx) const {return fStrides[idx];}

        //access operator (,,...,) -- no bounds checking
        //std::enable_if does a compile-time check that the number of arguments is the same as the rank of the array
        template <typename ...XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), XValueType& >::type
        operator()(XIndexTypeS...idx){ fTmp = {{static_cast<size_t>(idx)...}}; return ValueAt(fTmp); }

        //const reference access operator()
        template <typename ...XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) == RANK), const XValueType& >::type
        operator()(XIndexTypeS...idx) const { fTmp = {{static_cast<size_t>(idx)...}}; return ValueAt(fTmp);}

        //access via at(,,,,) -- same as operator() but with bounds checking
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), XValueType& >::type
        at(XIndexTypeS...idx)
        {
            //make sure the indices are valid for the given array dimensions
            fTmp = {{static_cast<size_t>(idx)...}};
            if( CheckIndexValidity(fTmp) ){ return ValueAt(fTmp); }
            else{ throw std::out_of_range("MHO_NDArrayView::at() indices out of range.");}
        }

        //const at()
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), const XValueType& >::type
        at(XIndexTypeS...idx) const
        {
            //make sure the indices are valid for the given array dimensions
            fTmp = {{static_cast<size_t>(idx)...}};
            if( CheckIndexValidity(fTmp) ){ return ValueAt(fTmp); }
            else{ throw std::out_of_range("MHO_NDArrayView::at() indices out of range.");}
        }

        //assignment operator
        MHO_NDArrayView& operator=(const MHO_NDArrayView& rhs)
        {
            if(this != &rhs)
            {
                Construct(rhs.fDataPtr, &(rhs.fDims[0]), &(rhs.fStrides[0]) ); 
            }
            return *this;
        }

        //set all elements in the array to a certain value
        void SetArray(const XValueType& obj)
        {
            if(!fIsSlice){for(std::size_t i=0; i < fSize; i++){fDataPtr[i] = obj; } }
            else{ msg_debug("containers", "warning not yet implemented"); }
        }

        //set all elements in the array to zero
        void ZeroArray()
        { 
            if(!fIsSlice){std::memset(fDataPtr, 0, fSize*sizeof(XValueType) ); }
            else{ msg_debug("containers", "warning not yet implemented"); }
        }

        //expensive copy (as opposed to the assignment operator,
        //pointers to exernally managed memory are not transfer)
        virtual void Copy(const MHO_NDArrayView& rhs)
        {
            if(this != &rhs)
            {
                if(rhs.IsSlice())
                {
                    Construct(nullptr,  &(rhs.fDims[0]));
                    if(fSize != 0)
                    {
                        msg_debug("containers", "warning not yet implemented");
                    }
                }
                else 
                {
                    Construct(nullptr,  &(rhs.fDims[0]));
                    if(fSize != 0){std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin() );}
                }
            }
        }

        //linear offset into the array
        std::size_t GetOffsetForIndices(const std::size_t* index)
        {
            return MHO_NDArrayMath::OffsetFromStrideIndex<RANK>(&(fStrides[0]), index);
        }



////////////////////////////////////////////////////////////////////////////////

        //simple in-place compound assignment operators (mult/add/sub)//////////

        //in place multiplication by a scalar factor
        template< typename T>
        typename std::enable_if< std::is_same<XValueType,T>::value or std::is_integral<T>::value or std::is_floating_point<T>::value, MHO_NDArrayView& >::type
        inline operator*=(T aScalar)
        {
            if(!fIsSlice){for(std::size_t i=0; i<fSize; i++){fDataPtr[i] *= aScalar;}}
            else{msg_debug("containers", "warning not yet implemented");};
            return *this;
        }

        //in place addition by a scalar amount
        template< typename T>
        typename std::enable_if< std::is_same<XValueType,T>::value or std::is_integral<T>::value or std::is_floating_point<T>::value, MHO_NDArrayView& >::type
        inline operator+=(T aScalar)
        {
            if(!fIsSlice){for(std::size_t i=0; i<fSize; i++){fDataPtr[i] += aScalar;}}
            else{msg_debug("containers", "warning not yet implemented");}
            return *this;
        }

        //in place subraction by a scalar amount
        template< typename T>
        typename std::enable_if< std::is_same<XValueType,T>::value or std::is_integral<T>::value or std::is_floating_point<T>::value, MHO_NDArrayView& >::type
        inline operator-=(T aScalar)
        {
            if(!fIsSlice){for(std::size_t i=0; i<fSize; i++){fDataPtr[i] -= aScalar;}}
            else{msg_debug("containers", "warning not yet implemented");}
            return *this;
        }

        //in place point-wise multiplication by another array
        inline MHO_NDArrayView& operator*=(const MHO_NDArrayView& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray)){throw std::out_of_range("MHO_NDArrayView::*= size mismatch.");}
            if(!fIsSlice && !anArray.IsSlice()){for(std::size_t i=0; i<fSize; i++){fDataPtr[i] *= anArray.fDataPtr[i];}}
            else{msg_debug("containers", "warning not yet implemented");}
            return *this;
        }

        //in place point-wise addition by another array
        inline MHO_NDArrayView& operator+=(const MHO_NDArrayView& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray)){throw std::out_of_range("MHO_NDArrayView::+= size mismatch.");}
            if(!fIsSlice && !anArray.IsSlice()){for(std::size_t i=0; i<fSize; i++){fDataPtr[i] += anArray.fDataPtr[i];}}
            else{msg_debug("containers", "warning not yet implemented");}
            return *this;
        }

        //in place point-wise subtraction of another array
        inline MHO_NDArrayView& operator-=(const MHO_NDArrayView& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray)){throw std::out_of_range("MHO_NDArrayView::-= size mismatch.");}
            if(!fIsSlice && !anArray.IsSlice()){for(std::size_t i=0; i<fSize; i++){fDataPtr[i] -= anArray.fDataPtr[i];}}
            else{msg_debug("containers", "warning not yet implemented");}
            return *this;
        }


    protected:

        XValueType* fDataPtr; //data for an array view is always externally managed
        index_type fDims; //size of each dimension
        index_type fStrides; //strides between elements in each dimension
        uint64_t fSize; //total size of array
        mutable index_type fTmp; //temp index workspace

        bool CheckIndexValidity(const index_type& idx)
        {
            return MHO_NDArrayMath::CheckIndexValidity<RANK>(&(fDims[0]), &(idx[0]) );
        }

        XValueType& ValueAt(const index_type& idx)
        {
            return fDataPtr[ MHO_NDArrayMath::OffsetFromStrideIndex<RANK>(&(fStrides[0]), &(idx[0]) ) ];
        }

        const XValueType& ValueAt(const index_type& idx) const
        {
            return fDataPtr[ MHO_NDArrayMath::OffsetFromStrideIndex<RANK>(&(fStrides[0]), &(idx[0]) ) ];
        }


    private:

        //special constructor for when strides are pre-determined (not from array dimensions)
        //this is only called with building a 'SliceView'
        void Construct(XValueType* ptr, const std::size_t* dim, const std::size_t* strides)
        {
            //default construction (empty)
            for(std::size_t i=0; i<RANK; i++){fDims[i] = 0; fStrides[i] = 0;}
            fSize = 0;
            fDataPtr = nullptr;
            if(ptr == nullptr || dim == nullptr || strides == nullptr)
            {
                msg_error("containers", "Cannot construct array slice." << eom);
                return;
            }

            fDataPtr = ptr;
            //set the dimensions, and the strides directly
            for(std::size_t i=0; i<RANK; i++){fDims[i] = dim[i]; fStrides[i] = strides[i];}
            fSize = MHO_NDArrayMath::TotalArraySize<RANK>(&(fDims[0]));
        }


    //the iterator definitions //////////////////////////////////////////////////
    public:

        // using iterator = MHO_BidirectionalIterator<XValueType>;
        // using stride_iterator = MHO_BidirectionalStrideIterator<XValueType>;
        // 
        // using const_iterator = MHO_BidirectionalIterator<XValueType>;
        // using const_stride_iterator = MHO_BidirectionalStrideIterator<XValueType>;
        // 
        // iterator begin(){ return iterator(fDataPtr, fDataPtr, fSize);}
        // iterator end(){ return iterator(fDataPtr, fDataPtr + fSize, fSize);}
        // iterator iterator_at(std::size_t offset){return iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize);}
        // 
        // const_iterator cbegin() const{ return const_iterator(fDataPtr, fDataPtr, fSize);}
        // const_iterator cend() const { return const_iterator(fDataPtr, fDataPtr + fSize, fSize);}
        // const_iterator citerator_at(std::size_t offset) const {return const_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize);}
        // 
        // stride_iterator stride_begin(std::size_t stride){ return stride_iterator(fDataPtr, fDataPtr, fSize, stride);}
        // stride_iterator stride_end(std::size_t stride){ return stride_iterator(fDataPtr, fDataPtr + fSize, fSize, stride);}
        // stride_iterator stride_iterator_at(std::size_t offset, std::size_t stride)
        // {
        //     return stride_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize, stride);
        // }
        // 
        // const_stride_iterator cstride_begin(std::size_t stride) const { return stride_iterator(fDataPtr, fDataPtr, fSize, stride);}
        // const_stride_iterator cstride_end(std::size_t stride) const { return stride_iterator(fDataPtr, fDataPtr + fSize, fSize, stride);}
        // const_stride_iterator cstride_iterator_at(std::size_t offset, std::size_t stride) const
        // {
        //     return stride_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize, stride);
        // }

};


}//end of namespace

#endif /* MHO_NDArrayView_HH__ */
