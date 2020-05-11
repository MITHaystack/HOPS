#ifndef HBufferPool_HH__
#define HBufferPool_HH__

#include <type_traits>
#include <cstring>
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>
#include <stdexcept>

#include "HLinearBuffer.hh"
#include "HBufferAllocatorBase.hh"
#include "HRegisteringBufferPool.hh"


namespace hops {

/*
*File: HBufferPool.hh
*Class: HBufferPool
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Sat Feb 10 02:01:00 EST 2018
*Description: ring of data buffers to be shared between a single producer and multiple (sequential) consumers
*/

template< typename XBufferItemType >
class HBufferPool: public HRegisteringBufferPool
{
    public:

        HBufferPool(HBufferAllocatorBase< XBufferItemType >* allocator):
            HRegisteringBufferPool(),
            fAllocator(allocator),
            fNChunks(0),
            fNItemsPerChunk(0),
            fTotalItems(0),
            fAllocated(false)
        {
            //make sure we have at least 1 consumer queue
            fConsumerQueueVector.resize(1);
        };

        HBufferPool(HBufferAllocatorBase< XBufferItemType >* allocator, std::size_t n_chunks, std::size_t items_per_chunk):
            HRegisteringBufferPool(),
            fAllocator(allocator),
            fNChunks(n_chunks),
            fNItemsPerChunk(items_per_chunk),
            fTotalItems(n_chunks*items_per_chunk),
            fAllocated(false)
        {
            Allocate(fNChunks, fNItemsPerChunk);
            fConsumerQueueVector.resize(1);
        };

        virtual ~HBufferPool()
        {
            Deallocate();
        };

        void Allocate(std::size_t n_chunks, std::size_t items_per_chunk)
        {
            if( n_chunks < 1)
            {
                throw std::runtime_error("HBufferPool::Allocate(): Cannot allocate a ring buffer with less than one element.");
            }

            std::lock_guard<std::mutex> lock(fMutex);
            if(fAllocated){Deallocate();}

            fNChunks = n_chunks;
            fNItemsPerChunk = items_per_chunk;
            fTotalItems = fNChunks*fNItemsPerChunk;

            //for each chunk, call the allocator and create a linear buffer
            fChunks.resize(0);
            for(size_t i=0; i<fNChunks; i++ )
            {
                auto chunk = fAllocator->allocate( fNItemsPerChunk ); //TODO, deal with exceptions
                if(chunk != nullptr)
                {
                    fChunks.push_back( new HLinearBuffer< XBufferItemType >( chunk, fNItemsPerChunk) );
                }
            }

            if(fChunks.size() != 0)
            {
                fAllocated = true;
            }

            for(size_t i=0; i<fChunks.size(); i++ )
            {
                fProducerQueue.push( fChunks[i] );
            }

        }

        void Deallocate()
        {
            std::lock_guard<std::mutex> lock(fMutex);
            if(fAllocated)
            {
                for(size_t i=0; i<fChunks.size(); i++)
                {
                    fAllocator->deallocate( fChunks[i]->GetData(), fNItemsPerChunk);
                }
                fChunks.resize(0);
            }
            fAllocated = false;
        }

        bool IsAllocated() const { return fAllocated; }

        virtual void Initialize() override
        {
            //need to resize the consumer queue vector to match the number of registered consumers
            if(fNRegisteredConsumers >= 1)
            {
                fConsumerQueueVector.resize( fNRegisteredConsumers );
            }
            //std::cout<<"number of consumer = "<<fConsumerQueueVector.size()<<std::endl;
        }

        size_t GetNumberOfConsumerPools() const
        {
            return fConsumerQueueVector.size();
        }

        size_t GetConsumerPoolSize(unsigned int id = 0) const
        {
            std::lock_guard<std::mutex> lock(fMutex);
            return fConsumerQueueVector[id].size();
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
            //lock the buffer pool, so more than one thread modify the queue
            std::lock_guard<std::mutex> lock(fMutex);
            if(buff != nullptr)
            {
                fProducerQueue.push(buff);
            }
        }

        //pop a buffer off of the consumer queue for use by a registered consumer
        //with the given id
        HLinearBuffer< XBufferItemType >* PopConsumerBuffer(unsigned int id=0)
        {
            //lock the buffer pool, so more than one thread cannot grab the same buffer
            std::lock_guard<std::mutex> lock(fMutex);
            HLinearBuffer< XBufferItemType >* buff = nullptr;
            if(id < fConsumerQueueVector[id].size() )
            {
                buff = fConsumerQueueVector[id].front();
                fConsumerQueueVector[id].pop();
            }
            return buff;
        }

        //return a buffer to the (next available consumer w/ id) queue
        //if there is no next consumer, then push to the producer
        void PushConsumerBuffer(HLinearBuffer< XBufferItemType >* buff, unsigned int id=0)
        {
            //lock the buffer pool, so more than one thread can't modify the queue
            std::lock_guard<std::mutex> lock(fMutex);
            if(id < fConsumerQueueVector.size())
            {
                fConsumerQueueVector[id].push(buff);
            }
            else
            {
                fProducerQueue.push(buff);
            }
        }

    protected:

        //allocator
        HBufferAllocatorBase< XBufferItemType >* fAllocator;

        //data
        std::size_t fNChunks;
        std::size_t fNItemsPerChunk;
        std::size_t fTotalItems;
        bool fAllocated;

        //complete list of the buffer chunks 
        std::vector< HLinearBuffer< XBufferItemType >* > fChunks;

        //FIFO queue's of the buffers, for producer/consumer use
        std::queue< HLinearBuffer< XBufferItemType >* > fProducerQueue;
        std::vector< std::queue< HLinearBuffer< XBufferItemType >* > > fConsumerQueueVector;

        //modification mutex
        mutable std::mutex fMutex;

};

}

#endif /* end of include guard: HBufferPool */
