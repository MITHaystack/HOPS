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
#include <string>
#include <vector>
#include <array>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <cinttypes>
#include <algorithm>

#include "MHO_Message.hh"
#include "MHO_NDArrayMath.hh"
#include "MHO_BidirectionalIterator.hh"
#include "MHO_BidirectionalStrideIterator.hh"
#include "MHO_ExtensibleElement.hh"

namespace hops
{

template< typename XValueType, std::size_t RANK>
class MHO_NDArrayWrapper:
    public MHO_ExtensibleElement //any and all extensions are purely a runtime concept and do NOT get streamed for I/O
{
    public:

        using value_type = XValueType;
        using index_type = std::array<std::size_t, RANK>;
        typedef std::integral_constant< std::size_t, RANK > rank;

        //constructors
        MHO_NDArrayWrapper(){Construct(nullptr, nullptr);}; //empty constructor, to be configured later
        MHO_NDArrayWrapper(const std::size_t* dim){Construct(nullptr, dim);}; //data is internally allocated
        MHO_NDArrayWrapper(XValueType* ptr, const std::size_t* dim){Construct(ptr,dim);}; //data is externally allocated/managed
        MHO_NDArrayWrapper(const MHO_NDArrayWrapper& obj)
        {
            if(obj.fExternallyManaged){Construct(obj.fDataPtr, &(obj.fDims[0]) );}
            else
            {
                Construct(nullptr, &(obj.fDims[0]) );
                if(fSize != 0){std::copy(obj.fData.begin(), obj.fData.end(), fData.begin() );}
            }
        }

        //destructor
        virtual ~MHO_NDArrayWrapper(){};

        //clone functionality
        MHO_NDArrayWrapper* Clone(){ return new MHO_NDArrayWrapper(*this); }

        //resize functions
        virtual void Resize(const std::size_t* dim)
        {
            if(fExternallyManaged)
            {
                msg_warn("containers", "Resize operation called on a wrapper pointing to " <<
                          "an exernally managed array will replace it with internally " <<
                          "managed memory. This may result in unexpected behavior." << eom);
            }
            Construct(nullptr, dim);
        }

        template <typename ...XDimSizeTypeS >
        typename std::enable_if< (sizeof...(XDimSizeTypeS) == RANK), void >::type
        Resize(XDimSizeTypeS...dim)
        {
            fTmp = {{static_cast<size_t>(dim)...}}; //convert the arguments to an array
            Resize(&(fTmp[0]));
        }

        //set pointer to externally managed array with associated dimensions
        void SetExternalData(XValueType* ptr, const std::size_t* dim){Construct(ptr, dim);}

        //access to underlying raw array pointer
        XValueType* GetData(){return fDataPtr;};
        const XValueType* GetData() const {return fDataPtr;};

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
            else{ throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");}
        }

        //const at()
        template <typename ...XIndexTypeS >
        typename std::enable_if<(sizeof...(XIndexTypeS) == RANK), const XValueType& >::type
        at(XIndexTypeS...idx) const
        {
            //make sure the indices are valid for the given array dimensions
            fTmp = {{static_cast<size_t>(idx)...}};
            if( CheckIndexValidity(fTmp) ){ return ValueAt(fTmp); }
            else{ throw std::out_of_range("MHO_NDArrayWrapper::at() indices out of range.");}
        }

        //fast access operator by 1-dim index (absolute-position) into the array
        XValueType& operator[](std::size_t i){return fDataPtr[i];}
        const XValueType& operator[](std::size_t i) const {return fDataPtr[i];}

        //assignment operator
        MHO_NDArrayWrapper& operator=(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                if(rhs.fExternallyManaged)
                {
                    Construct(rhs.fDataPtr, &(rhs.fDims[0]) );
                }
                else
                {
                    Construct(nullptr,  &(rhs.fDims[0]) );
                    if(fSize != 0){std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin() );}
                }
            }
            return *this;
        }

        //convenience functions
        void SetArray(const XValueType& obj){ for(std::size_t i=0; i < fSize; i++){fDataPtr[i] = obj; } }
        void ZeroArray(){ std::memset(fDataPtr, 0, fSize*sizeof(XValueType) ); }; //set all elements in the array to zero

        //expensive copy (as opposed to the assignment operator,
        //pointers to exernally managed memory are not transfer)
        virtual void Copy(const MHO_NDArrayWrapper& rhs)
        {
            if(this != &rhs)
            {
                Construct(nullptr,  &(rhs.fDims[0]));
                if(fSize != 0){std::copy(rhs.fData.begin(), rhs.fData.end(), this->fData.begin() );}
            }
        }

        //linear offset into the array
        std::size_t GetOffsetForIndices(const std::size_t* index)
        {
            return MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(&(fDims[0]), index);
        }

        //sub-view of the array (given n < RANK leading indexes), return the remaining
        //chunk of the array with freely spanning indexes
        //for example: a ndarray X of RANK=3, and sizes [4,12,32], then SubView(2)
        //returns an ndarray of RANK=2, and dimensions [12,32] starting at the
        //location of X(2,0,0). Data of the subview points to data owned by X
        template <typename ...XIndexTypeS >
        typename std::enable_if< (sizeof...(XIndexTypeS) < RANK), MHO_NDArrayWrapper<XValueType, RANK - ( sizeof...(XIndexTypeS) ) > >::type
        SubView(XIndexTypeS...idx)
        {
            std::array<std::size_t, sizeof...(XIndexTypeS) > leading_idx = {{static_cast<size_t>(idx)...}};
            for(std::size_t i=0; i<RANK; i++){fTmp[i] = 0;}
            for(std::size_t i=0; i<leading_idx.size(); i++){fTmp[i] = leading_idx[i];}
            std::size_t offset = MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(&(fDims[0]), &(fTmp[0]));
            std::array<std::size_t, RANK - (sizeof...(XIndexTypeS)) > dim;
            for(std::size_t i=0; i<dim.size(); i++){dim[i] = fDims[i + (sizeof...(XIndexTypeS) )];}
            return  MHO_NDArrayWrapper<XValueType, RANK - ( sizeof...(XIndexTypeS) ) >(&(fDataPtr[offset]) , &(dim[0]) );
        }

        //simple in-place compound assignment operators (mult/add/sub)//////////

        //in place multiplication by a scalar factor
        template< typename T>
        typename std::enable_if< std::is_same<XValueType,T>::value or std::is_integral<T>::value or std::is_floating_point<T>::value, MHO_NDArrayWrapper& >::type
        inline operator*=(T aScalar)
        {
            for(std::size_t i=0; i<fSize; i++){fDataPtr[i] *= aScalar;}
            return *this;
        }

        //in place addition by a scalar amount
        template< typename T>
        typename std::enable_if< std::is_same<XValueType,T>::value or std::is_integral<T>::value or std::is_floating_point<T>::value, MHO_NDArrayWrapper& >::type
        inline operator+=(T aScalar)
        {
            for(std::size_t i=0; i<fSize; i++){fDataPtr[i] += aScalar;}
            return *this;
        }

        //in place subraction by a scalar amount
        template< typename T>
        typename std::enable_if< std::is_same<XValueType,T>::value or std::is_integral<T>::value or std::is_floating_point<T>::value, MHO_NDArrayWrapper& >::type
        inline operator-=(T aScalar)
        {
            for(std::size_t i=0; i<fSize; i++){fDataPtr[i] -= aScalar;}
            return *this;
        }

        //in place point-wise multiplication by another array
        inline MHO_NDArrayWrapper& operator*=(const MHO_NDArrayWrapper& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray)){throw std::out_of_range("MHO_NDArrayWrapper::*= size mismatch.");}
            for(std::size_t i=0; i<fSize; i++){fDataPtr[i] *= anArray.fDataPtr[i];}
            return *this;
        }

        //in place point-wise addition by another array
        inline MHO_NDArrayWrapper& operator+=(const MHO_NDArrayWrapper& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray)){throw std::out_of_range("MHO_NDArrayWrapper::+= size mismatch.");}
            for(std::size_t i=0; i<fSize; i++){fDataPtr[i] += anArray.fDataPtr[i];}
            return *this;
        }

        //in place point-wise subtraction of another array
        inline MHO_NDArrayWrapper& operator-=(const MHO_NDArrayWrapper& anArray)
        {
            if(!HaveSameNumberOfElements(this, &anArray)){throw std::out_of_range("MHO_NDArrayWrapper::-= size mismatch.");}
            for(std::size_t i=0; i<fSize; i++){fDataPtr[i] -= anArray.fDataPtr[i];}
            return *this;
        }


    protected:

        // //only meta-data types we store are a name, and unit type
        // std::string fName;
        // //until we develop a proper units/dimensions type, 
        // //we just store units as a string (the units class must be able to convert to <-> from a string)
        // std::string fUnits;

        XValueType* fDataPtr;
        bool fExternallyManaged;
        std::vector< XValueType > fData; //used for internally managed data
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
            //return fDataPtr[ MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(&(fDims[0]), &(idx[0]) ) ];
        }

        const XValueType& ValueAt(const index_type& idx) const
        {
            return fDataPtr[ MHO_NDArrayMath::OffsetFromStrideIndex<RANK>(&(fStrides[0]), &(idx[0]) ) ];
            //return fDataPtr[ MHO_NDArrayMath::OffsetFromRowMajorIndex<RANK>(&(fDims[0]), &(idx[0]) ) ];
        }




    private:

        void Construct(XValueType* ptr, const std::size_t* dim)
        {
            //default construction (empty)
            for(std::size_t i=0; i<RANK; i++){fDims[i] = 0; fStrides[0] = 0;}
            fSize = 0;
            fDataPtr = nullptr;
            fExternallyManaged = false;
            if(ptr == nullptr && dim == nullptr){return;}

            //dimensions known
            if(dim != nullptr){for(std::size_t i=0; i<RANK; i++){fDims[i] = dim[i];}}
            fSize = MHO_NDArrayMath::TotalArraySize<RANK>(&(fDims[0]));
            ComputeStrides();

            if(ptr != nullptr) //using externally managed memory
            {
                //effectively de-allocate anything we might have had before
                std::vector< XValueType >().swap(fData);
                fDataPtr = ptr;
                fExternallyManaged = true;
            }
            else //use internally managed memory
            {
                fData.resize(fSize);
                fDataPtr = &(fData[0]);
                fExternallyManaged = false;
            }
        }

        void ComputeStrides()
        {
            for(std::size_t i=0; i<RANK; i++)
            {
                //stride for elements of this dimension
                std::size_t stride = 1;
                std::size_t j = RANK-1;
                while(j > i){stride *= fDims[j]; j--;}
                fStrides[i] = stride;
            }
        }


    //the iterator definitions //////////////////////////////////////////////////
    public:

        using iterator = MHO_BidirectionalIterator<XValueType>;
        using stride_iterator = MHO_BidirectionalStrideIterator<XValueType>;

        using const_iterator = MHO_BidirectionalIterator<XValueType>;
        using const_stride_iterator = MHO_BidirectionalStrideIterator<XValueType>;

        iterator begin(){ return iterator(fDataPtr, fDataPtr, fSize);}
        iterator end(){ return iterator(fDataPtr, fDataPtr + fSize, fSize);}
        iterator iterator_at(std::size_t offset){return iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize);}

        const_iterator cbegin() const{ return const_iterator(fDataPtr, fDataPtr, fSize);}
        const_iterator cend() const { return const_iterator(fDataPtr, fDataPtr + fSize, fSize);}
        const_iterator citerator_at(std::size_t offset) const {return const_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize);}

        stride_iterator stride_begin(std::size_t stride){ return stride_iterator(fDataPtr, fDataPtr, fSize, stride);}
        stride_iterator stride_end(std::size_t stride){ return stride_iterator(fDataPtr, fDataPtr + fSize, fSize, stride);}
        stride_iterator stride_iterator_at(std::size_t offset, std::size_t stride)
        {
            return stride_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize, stride);
        }

        const_stride_iterator cstride_begin(std::size_t stride) const { return stride_iterator(fDataPtr, fDataPtr, fSize, stride);}
        const_stride_iterator cstride_end(std::size_t stride) const { return stride_iterator(fDataPtr, fDataPtr + fSize, fSize, stride);}
        const_stride_iterator cstride_iterator_at(std::size_t offset, std::size_t stride) const
        {
            return stride_iterator(fDataPtr, fDataPtr + std::min(offset, fSize), fSize, stride);
        }


};


}//end of namespace

//include the partial specializations for RANK=0 and RANL=1
#include "MHO_NDArrayWrapper_0.hh"
#include "MHO_NDArrayWrapper_1.hh"

namespace hops
{

//utilities ////////////////////////////////////////////////////////////////////
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
