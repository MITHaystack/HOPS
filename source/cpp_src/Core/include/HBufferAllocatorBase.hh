#include <cstddef>
#include <memory>
#include <iostream>

#ifndef HBufferAllocatorBase_HH__
#define HBufferAllocatorBase_HH__

namespace hops{

/*
*File: HBufferAllocatorBase.hh
*Class: HBufferAllocatorBase
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: abstract base class for bare-bones allocator, 
* provides the minimum requirements necessary to use std::allocator_traits
*/

template< typename XBufferItemType >
class HBufferAllocatorBase
{
    public:
        HBufferAllocatorBase() noexcept {};
        virtual ~HBufferAllocatorBase(){};

        using value_type = XBufferItemType;

        XBufferItemType* allocate(size_t size)
        {
            return this->AllocateImpl(size);
        }

        void deallocate(XBufferItemType* ptr, size_t size)
        {
            this->DeallocateImpl(ptr, size);
        }

    protected:

        virtual XBufferItemType* AllocateImpl(size_t size) = 0;
        virtual void DeallocateImpl(XBufferItemType* ptr, size_t size) = 0;

};

//the following allow allocators to be interchangeable for alloc/dealloc

template< typename XBufferItemType, typename XOtherType >
constexpr bool operator== (const HBufferAllocatorBase< XBufferItemType >&, const HBufferAllocatorBase< XOtherType >&) noexcept
{return true;}

template< typename XBufferItemType, typename XOtherType >
constexpr bool operator!= (const HBufferAllocatorBase< XBufferItemType >&, const HBufferAllocatorBase< XOtherType >&) noexcept
{return false;}

}

#endif /* end of include guard: HBufferAllocatorBase */
