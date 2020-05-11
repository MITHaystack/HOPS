#ifndef HLinearBuffer_HH__
#define HLinearBuffer_HH__

#include <mutex>
#include <stack>
#include <stdint.h>

#include "HArrayWrapper.hh"
#include "HBufferMetaData.hh"

namespace hops {

/*
*File: HLinearBuffer.hh
*Class: HLinearBuffer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

template< typename XBufferItemType >
class HLinearBuffer: public HArrayWrapper< XBufferItemType, 1 >
{
    public:
        HLinearBuffer():HArrayWrapper<XBufferItemType, 1>(){};
        HLinearBuffer(XBufferItemType* data, std::size_t length):HArrayWrapper<XBufferItemType, 1>(data, &length){};

        virtual ~HLinearBuffer(){};

        using buffer_item_type = XBufferItemType;

        //access to the buffer meta data
        HBufferMetaData* GetMetaData() {return &fMetaData;};

    public:

        std::mutex fMutex;

    protected:

        HBufferMetaData fMetaData;

};

}

#endif /* end of include guard: HLinearBuffer */
