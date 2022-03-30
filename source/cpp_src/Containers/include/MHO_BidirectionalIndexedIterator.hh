#ifndef MHO_BidirectionalIndexedIterator_HH__
#define MHO_BidirectionalIndexedIterator_HH__

/*
*@file: MHO_BidirectionalIndexedIterator.hh
*@class: MHO_BidirectionalIndexedIterator
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: This is an interator for more complicated n-darrays (primarily array "slices").
* Because we cannot guarantee that adjacent elements (in index space) are contiguous in
* memory for an array slice, we need to ensure that the proper strided access takes place.
*/


#include <iterator>

#include "MHO_Message.hh"
#include "MHO_NDArrayMath.hh"

namespace hops 
{

template< typename XValueType, std::size_t RANK>
class MHO_BidirectionalIndexedIterator
{
    public:

        typedef MHO_BidirectionalIndexedIterator self_type;
        typedef XValueType value_type;
        typedef XValueType& reference;
        typedef XValueType* pointer;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef std::ptrdiff_t difference_type;
        typedef std::array<std::size_t, RANK> index_type;

        MHO_BidirectionalIndexedIterator(pointer begin_ptr, std::size_t position_offset, std::size_t* dims, std::size_t* strides):
            fBegin(begin_ptr),
            fPositionOffset(position_offset)
        {
            for(std::size_t i=0; i<RANK; i++){fDimensions[i] = dims[i]; fStrides[i] = strides[i];}
            fLength = MHO_NDArrayMath::TotalArraySize<RANK>(&(fDimensions[0])); //determine size of array
            CalculateOffsets();
            fPtr = fBegin + fMemoryOffset;
        }

        MHO_BidirectionalIndexedIterator(const self_type& copy):
            fBegin(copy.fBegin),
            fPositionOffset(copy.fPositionOffset)
        {
            for(std::size_t i=0; i<RANK; i++){fDimensions[i] = copy.fDimensions[i]; fStrides[i] = copy.fStrides[i];}
            fLength = MHO_NDArrayMath::TotalArraySize<RANK>(&(fDimensions[0])); //determine size of array 
            CalculateOffsets();
            fPtr = fBegin + fMemoryOffset;
        }


        virtual ~MHO_BidirectionalIndexedIterator(){};

        self_type operator++()
        {
            fPositionOffset++;
            CalculateOffsets();
            fPtr = fBegin + fMemoryOffset;
            return *this;
        }

        self_type operator--()
        {
            fPositionOffset--;
            CalculateOffsets();
            fPtr = fBegin + fMemoryOffset;
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

        difference_type operator-(const self_type& iter)
        {
            return fPositionOffset - iter.GetPositionOffset();
        }

        self_type operator+=(const std::ptrdiff_t& diff)
        {
            fPositionOffset += diff; //TODO CHECK AGAINST out_of_range ERRORS
            CalculateOffsets();
            fPtr = fBegin + fMemoryOffset;//*sizeof(XValueType);
            return *this;
        }

        self_type operator-=(const std::ptrdiff_t& diff)
        {
            fPositionOffset -= diff; //TODO CHECK AGAINST out_of_range ERRORS
            CalculateOffsets();
            fPtr = fBegin + fMemoryOffset;
            return *this;
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
        reference operator*() { return *fPtr; }
        pointer operator->() { return fPtr; }
        reference operator*() const { return *fPtr; }
        const pointer operator->() const { return fPtr; }

        self_type operator=(const self_type& rhs)
        {
            if(this != &rhs)
            {
                fBegin = rhs.fBegin;
                fPtr = rhs.fPtr;
                fPositionOffset = rhs.fPositionOffset;
                for(std::size_t i=0; i<RANK; i++){fDimensions[i] = rhs.fDimensions[i]; fStrides[i] = rhs.fStrides[i];}
                CalculateOffsets();
            }
            return *this;
        }

        bool operator==(const self_type& rhs) const
        {
            return fPtr == rhs.fPtr;
        }

        bool operator!=(const self_type& rhs) const
        {
            return fPtr != rhs.fPtr;
        }

        pointer GetPtr(){return fPtr;}
        const pointer GetPtr() const {return fPtr;}

        difference_type GetPositionOffset() const
        {
            return fPositionOffset;
        }

        difference_type GetMemoryOffset() const
        {
            return std::distance(fBegin, fPtr);
        }

        bool IsValid() const
        {
            return ( (fBegin <= fPtr) && (fPtr < fBegin + fLength ) );
        }

    protected:

        //array description
        pointer fBegin;
        index_type fDimensions;
        index_type fStrides;
        std::size_t fLength;

        //current position in array
        pointer fPtr;
        std::size_t fPositionOffset; //the position in the flattened array [0,fLength)
        std::size_t fMemoryOffset; //absolute difference in memory from fBegin
        index_type fIdx;


        void CalculateOffsets()
        {
            if(fPositionOffset < fLength)
            {
                MHO_NDArrayMath::RowMajorIndexFromOffset<RANK>(fPositionOffset, &(fDimensions[0]), &(fIdx[0]) ); //determine current indexes
                fMemoryOffset = MHO_NDArrayMath::OffsetFromStrideIndex<RANK>(&(fStrides[0]), &(fIdx[0]) );
            }
            else 
            {
                //clamp to the end of the array + 1 (the 'end')
                fPositionOffset = fLength-1;
                MHO_NDArrayMath::RowMajorIndexFromOffset<RANK>(fPositionOffset, &(fDimensions[0]), &(fIdx[0]) );
                fIdx[RANK-1] += 1; //this inde is now out of range (1 past the end)
                fMemoryOffset = MHO_NDArrayMath::OffsetFromStrideIndex<RANK>(&(fStrides[0]), &(fIdx[0]) );
                fPositionOffset = fLength;
            }

        }

};

}//end of namespace

#endif /* end of include guard: MHO_BidirectionalIndexedIterator */








