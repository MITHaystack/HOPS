#ifndef HUnallocatedBufferPool_HH__
#define HUnallocatedBufferPool_HH__

#include <type_traits>
#include <cstring>
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>
#include <stdexcept>

#include "HLinearBuffer.hh"
#include "HBufferAllocatorBase.hh"

namespace hops {

/*
*File: HUnallocatedBufferPool.hh
*Class: HUnallocatedBufferPool
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Sat Feb 10 02:01:00 EST 2018
*Description: ring of data buffers (do not allocate anything, only provides queues)
*/

template< typename XBufferItemType >
class HUnallocatedBufferPool
{
    public:

        HUnallocatedBufferPool(){};
        virtual ~HUnallocatedBufferPool(){};

        size_t GetConsumerPoolSize() const
        {
            std::lock_guard<std::mutex> lock(fMutex);
            return fConsumerQueue.size();
        }

        size_t GetProducerPoolSize() const 
        {
            std::lock_guard<std::mutex> lock(fMutex);
            return fProducerQueue.size();
        }

        //pop a buffer off of the producer queue for use by a producer 
        HLinearBuffer< XBufferItemType >* PopProducerBuffer()
        {
            //lock the buffer pool, so more than one thread cannot grab the same buffer
            std::lock_guard<std::mutex> lock(fMutex);
            HLinearBuffer< XBufferItemType >* buff = nullptr;
            if(fProducerQueue.size() != 0 )
            {
                buff = fProducerQueue.front();
                fProducerQueue.pop();
            }
            return buff;
        }

        //return a buffer to the producer queue
        void PushProducerBuffer(HLinearBuffer< XBufferItemType >* buff)
        {
            //for now, we do not check if the buff is an actual member of this pool
            //TODO: determine if this check is needed

            //lock the buffer pool, so more than one thread modify the queue
            std::lock_guard<std::mutex> lock(fMutex);
            fProducerQueue.push(buff);
        }


        //pop a buffer off of the consumer queue for use by a consumer
        HLinearBuffer< XBufferItemType >* PopConsumerBuffer()
        {
            //lock the buffer pool, so more than one thread cannot grab the same buffer
            std::lock_guard<std::mutex> lock(fMutex);
            HLinearBuffer< XBufferItemType >* buff = nullptr;
            if(fConsumerQueue.size() != 0 )
            {
                buff = fConsumerQueue.front();
                fConsumerQueue.pop();
            }
            return buff;
        }

        //return a buffer to the consumer queue
        void PushConsumerBuffer(HLinearBuffer< XBufferItemType >* buff)
        {
            //for now, we do not check if the buff is an actual member of this pool
            //TODO: determine if this check is needed

            //lock the buffer pool, so more than one thread modify the queue
            std::lock_guard<std::mutex> lock(fMutex);
            fConsumerQueue.push(buff);
        }

    protected:

        //FIFO queue's of the buffers, for producer/consumer use
        std::queue< HLinearBuffer< XBufferItemType >* > fProducerQueue;
        std::queue< HLinearBuffer< XBufferItemType >* > fConsumerQueue;

        //modification mutex
        mutable std::mutex fMutex;

};

}

#endif /* end of include guard: HUnallocatedBufferPool */
