#ifndef MHO_BidirectionalIterator_HH__
#define MHO_BidirectionalIterator_HH__

/*!
*@file  MHO_BidirectionalIterator.hh
*@class  MHO_BidirectionalIterator
*@author  J. Barrett - barrettj@mit.edu 
*@date
*@brief
*/

#include <iterator>

namespace hops
{

template< typename XValueType >
class MHO_BidirectionalIterator
{
    public:

        //typedef MHO_BidirectionalIterator self_type;
        // typedef XValueType value_type;
        // typedef XValueType& reference;
        // typedef XValueType* pointer;
        // typedef std::bidirectional_iterator_tag iterator_category;
        // typedef std::ptrdiff_t difference_type;

        using self_type = MHO_BidirectionalIterator;
        using value_type = XValueType;
        using reference = XValueType&;
        using pointer = XValueType*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;

        MHO_BidirectionalIterator(pointer begin_ptr, pointer ptr, std::size_t length):
            fBegin(begin_ptr),
            fPtr(ptr),
            fLength(length)
        {};

        MHO_BidirectionalIterator(const self_type& copy):
            fBegin(copy.fBegin),
            fPtr(copy.fPtr),
            fLength(copy.fLength)
        {};

        virtual ~MHO_BidirectionalIterator(){};

        self_type operator++()
        {
            ++fPtr;
            return *this;
        }

        self_type operator--()
        {
            --fPtr;
            return *this;
        }

        self_type operator++(int)
        {
            self_type ret_val(*this);
            ++fPtr;
            return ret_val;
        }

        self_type operator--(int)
        {
            self_type ret_val(*this);
            --fPtr;
            return ret_val;
        }

        difference_type operator-(const self_type& iter)
        {
            return std::distance(iter.GetPtr(), fPtr);
        }

        self_type operator+=(const std::ptrdiff_t& diff)
        {
            fPtr += diff;
            return *this;
        }

        self_type operator-=(const std::ptrdiff_t& diff)
        {
            fPtr -= diff;
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
        const reference operator*() const { return *fPtr; }

        pointer operator->() { return fPtr; }
        const pointer operator->() const { return fPtr; }

        self_type operator=(const self_type& rhs)
        {
            if(this != &rhs)
            {
                fBegin = rhs.fBegin;
                fPtr = rhs.fPtr;
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

        std::size_t GetOffset() const
        {
            return std::distance(fBegin, fPtr);
        }

        bool IsValid() const
        {
            return ( (fBegin <= fPtr) && (fPtr < fBegin + fLength ) );
        }

    protected:

        pointer fBegin;
        pointer fPtr;
        std::size_t fLength;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


template< typename XValueType >
class MHO_BidirectionalConstIterator
{
    public:

        using self_type = MHO_BidirectionalConstIterator;
        using value_type = XValueType;
        using reference = const XValueType&;
        using pointer = const XValueType*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;

        MHO_BidirectionalConstIterator(pointer begin_ptr, pointer ptr, std::size_t length):
            fBegin(begin_ptr),
            fPtr( const_cast<XValueType*>(ptr) ), //we promise not to touch the contents
            fLength(length)
        {};

        MHO_BidirectionalConstIterator(const self_type& copy):
            fBegin(copy.fBegin),
            fPtr(copy.fPtr),
            fLength(copy.fLength)
        {};

        virtual ~MHO_BidirectionalConstIterator(){};

        self_type operator++()
        {
            ++fPtr;
            return *this;
        }

        self_type operator--()
        {
            --fPtr;
            return *this;
        }

        self_type operator++(int)
        {
            self_type ret_val(*this);
            ++fPtr;
            return ret_val;
        }

        self_type operator--(int)
        {
            self_type ret_val(*this);
            --fPtr;
            return ret_val;
        }

        difference_type operator-(const self_type& iter)
        {
            return std::distance(iter.GetPtr(), fPtr);
        }

        self_type operator+=(const std::ptrdiff_t& diff)
        {
            fPtr += diff;
            return *this;
        }

        self_type operator-=(const std::ptrdiff_t& diff)
        {
            fPtr -= diff;
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

        //access to underlying array item object is always const
        reference operator*() const { return *fPtr; }
        pointer operator->() const { return fPtr; }

        self_type operator=(const self_type& rhs)
        {
            if(this != &rhs)
            {
                fBegin = rhs.fBegin;
                fPtr = rhs.fPtr;
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

        pointer GetPtr() const {return fPtr;}

        std::size_t GetOffset() const
        {
            return std::distance(fBegin, fPtr);
        }

        bool IsValid() const
        {
            return ( (fBegin <= fPtr) && (fPtr < fBegin + fLength ) );
        }

    protected:

        const pointer fBegin;
        pointer fPtr;
        std::size_t fLength;
};



}//end of namespace

#endif /*! end of include guard: MHO_BidirectionalIterator */
