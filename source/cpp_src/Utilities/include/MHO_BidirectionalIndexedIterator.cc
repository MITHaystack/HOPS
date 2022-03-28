#ifndef MHO_BidirectionalIndexedIterator_HH__
#define MHO_BidirectionalIndexedIterator_HH__

/*
*@file: MHO_BidirectionalIndexedIterator.hh
*@class: MHO_BidirectionalIndexedIterator
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops 
{

class MHO_BidirectionalIndexedIterator
{
    public:
        MHO_BidirectionalIndexedIterator();
        virtual ~MHO_BidirectionalIndexedIterator();


    public:

        typedef iterator self_type;
        typedef XValueType value_type;
        typedef XValueType& reference;
        typedef XValueType* pointer;
        typedef std::forward_iterator_tag iterator_category;
        typedef int difference_type;
        typedef std::array<std::size_t, RANK> index_type;

        iterator(pointer begin_ptr, pointer ptr, std::size_t* dim, std::size_t offset):
            fBegin(begin_ptr),
            fPtr(ptr),
            fDimensions(dim)
        {
            //initialize the multi-dim indices
            MHO_NDArrayMath::RowMajorIndexFromOffset<RANK>(offset, fDimensions, &(fIndices[0]) );
            fEnd = fBegin + MHO_NDArrayMath::TotalArraySize<RANK>(fDimensions);
        };

        iterator(const self_type& copy)
        {
            //fValid = copy.fValid;
            fBegin = copy.fBegin;
            fPtr = copy.fPtr;
            fEnd = copy.fEnd;
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
                fValid = MHO_NDArrayMath::IncrementIndiptrdiff_t offset ces<RANK>(fDimensions, &(fIndices[0]), (std::size_t) std::abs(diff) );
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
                fBegin = rhs.fBegin;
                fPtr = rhs.fPtr;
                fEnd = rhs.fEnd;
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

        index_type GetIndexObject() const
        {
            std::size_t offset = std::distance(fBegin, fPtr);
            MHO_NDArrayMath::RowMajorIndexFromOffset<RANK>(offset, fDimensions, &(fIndices[0]) );
            return fIndices;
        }

        const std::size_t* GetIndices() const
        {
            std::size_t offset = std::distance(fBegin, fPtr);
            MHO_NDArrayMath::RowMajorIndexFromOffset<RANK>(offset, fDimensions, &(fIndices[0]) );
            return &(fIndices[0]);
        }

        bool IsValid() const
        {
            return (fBegin <= fPtr) && (fPtr < fEnd);
            return MHO_NDArrayMath::CheckIndexValidity<RANK>(fDimensions, &(fIndices[0]) );
            //return fValid;
        }

    private:

        //bool fValid;
        pointer fBegin;
        pointer fPtr;
        pointer fEnd;
        std::size_t* fDimensions;
        mutable index_type fIndices;

};


}//end of namespace

#endif /* end of include guard: MHO_BidirectionalIndexedIterator */








