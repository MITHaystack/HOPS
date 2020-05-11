#ifndef HBufferAllocatorMalloc_HH__
#define HBufferAllocatorMalloc_HH__

#include <cstdlib>

#include "HBufferAllocatorBase.hh"


namespace hops{

/*
*File: HBufferAllocatorMalloc.hh
*Class: HBufferAllocatorMalloc
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: buffer allocator which uses new (for testing)
*/

template< typename XBufferItemType >
class HBufferAllocatorMalloc: public HBufferAllocatorBase< XBufferItemType >
{
    public:

        HBufferAllocatorMalloc():HBufferAllocatorBase< XBufferItemType >() {};
        virtual ~HBufferAllocatorMalloc(){};

    protected:

        virtual XBufferItemType* AllocateImpl(size_t size) override;
        virtual void DeallocateImpl(XBufferItemType* ptr, size_t size) override;

};

template< typename XBufferItemType >
XBufferItemType* 
HBufferAllocatorMalloc< XBufferItemType >::AllocateImpl(size_t size)
{
    //don't throw exeception on alloc error, just return nullptr
    XBufferItemType* ptr =  (XBufferItemType*) malloc( size*sizeof(XBufferItemType) );
    return ptr;
}

template< typename XBufferItemType >
void 
HBufferAllocatorMalloc< XBufferItemType >::DeallocateImpl(XBufferItemType* ptr, size_t /*size*/)
{
    //currently we don't use size parameter
    free(ptr);
}

}

#endif /* end of include guard: HBufferAllocatorMalloc */
