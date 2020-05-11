#ifndef HBufferAllocatorNew_HH__
#define HBufferAllocatorNew_HH__

#include <new>

#include "HBufferAllocatorBase.hh"


namespace hops{

/*
*File: HBufferAllocatorNew.hh
*Class: HBufferAllocatorNew
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: buffer allocator which uses new (for testing)
*/

template< typename XBufferItemType >
class HBufferAllocatorNew: public HBufferAllocatorBase< XBufferItemType >
{
    public:

        HBufferAllocatorNew():HBufferAllocatorBase< XBufferItemType >() {};
        virtual ~HBufferAllocatorNew(){};

    protected:

        virtual XBufferItemType* AllocateImpl(size_t size) override;
        virtual void DeallocateImpl(XBufferItemType* ptr, size_t size) override;

};

template< typename XBufferItemType >
XBufferItemType* 
HBufferAllocatorNew< XBufferItemType >::AllocateImpl(size_t size)
{
    //don't throw exeception on alloc error, just return nullptr
    XBufferItemType* ptr = new(std::nothrow) XBufferItemType[size];
    return ptr;
}

template< typename XBufferItemType >
void 
HBufferAllocatorNew< XBufferItemType >::DeallocateImpl(XBufferItemType* ptr, size_t /*size*/)
{
    //currently we don't use size parameter
    delete[] ptr;
}

}

#endif /* end of include guard: HBufferAllocatorNew */
