#ifndef HConsumerBufferHandlerPolicy_HH__
#define HConsumerBufferHandlerPolicy_HH__

#include <chrono>
#include <thread>


#include "HBufferPool.hh"
#include "HLinearBuffer.hh"

namespace hops
{

/*
*File: HConsumerBufferHandlerPolicy.hh
*Class: HConsumerBufferHandlerPolicy
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

//enum of return codes
enum class HConsumerBufferPolicyCode
{
    unset = 0x0,
    fail = 0x1, //failed to reserve buffer
    success = 0x8, //successfully reserved (or released) a buffer
};

//ugly hack
inline int operator & (HConsumerBufferPolicyCode lhs,HConsumerBufferPolicyCode rhs)
{
    return (static_cast<int>(lhs) & static_cast<int>(rhs));
}

//base class for releasing buffers (common to all handler policies)
template< typename XBufferItemType >
class HConsumerBufferReleaser
{
    public:
        HConsumerBufferReleaser(){;};
        virtual ~HConsumerBufferReleaser(){;};

        HConsumerBufferPolicyCode ReleaseBufferToProducer(HBufferPool<XBufferItemType>* pool, HLinearBuffer<XBufferItemType>*& buffer)
        {
            pool->PushProducerBuffer(buffer);
            return HConsumerBufferPolicyCode::success;
        }

        HConsumerBufferPolicyCode ReleaseBufferToConsumer(HBufferPool<XBufferItemType>* pool, HLinearBuffer<XBufferItemType>*& buffer, unsigned int id=0)
        {
            pool->PushConsumerBuffer(buffer,id);
            return HConsumerBufferPolicyCode::success;
        }
};


//get a buffer immediately, if no buffer is available fail and return nullptr
template< typename XBufferItemType >
class HConsumerBufferHandler_Immediate: public HConsumerBufferReleaser< XBufferItemType >
{
    public:
        HConsumerBufferHandler_Immediate(){;};
        virtual ~HConsumerBufferHandler_Immediate(){;};

        HConsumerBufferPolicyCode ReserveBuffer(HBufferPool<XBufferItemType>* pool, HLinearBuffer<XBufferItemType>*& buffer, unsigned int id=0)
        {
            if(pool->GetConsumerPoolSize(id) != 0)
            {
                buffer = pool->PopConsumerBuffer(id);
                return HConsumerBufferPolicyCode::success;
            }
            else
            {
                buffer = nullptr;
                return HConsumerBufferPolicyCode::fail;
            }
        }
};

//wait indefinitely until a buffer is available
template< typename XBufferItemType >
class HConsumerBufferHandler_Wait: public HConsumerBufferReleaser< XBufferItemType >
{
    public:

        HConsumerBufferHandler_Wait():fSleepDurationNanoSeconds(100){;};
        virtual ~HConsumerBufferHandler_Wait(){;};

        void SetSleepDurationNanoSeconds(unsigned int ns){fSleepDurationNanoSeconds = ns;};
        unsigned int GetSleepDurationNanoSeconds() const {return fSleepDurationNanoSeconds;};

        HConsumerBufferPolicyCode ReserveBuffer(HBufferPool<XBufferItemType>* pool, HLinearBuffer<XBufferItemType>*& buffer, unsigned int id=0)
        {
            while(true)
            {
                if(pool->GetConsumerPoolSize(id) != 0)
                {
                    buffer = pool->PopConsumerBuffer(id);
                    return HConsumerBufferPolicyCode::success;
                }
                else
                {
                    //sleep for the specified duration if it is non-zero
                    if(fSleepDurationNanoSeconds != 0)
                    {
                        std::this_thread::sleep_for(std::chrono::nanoseconds(fSleepDurationNanoSeconds));
                    }
                }
            };
        }

    protected:

        unsigned int fSleepDurationNanoSeconds;

};


//wait for buffer until time-out is reached, then fail
template< typename XBufferItemType >
class HConsumerBufferHandler_WaitWithTimeout: public HConsumerBufferReleaser< XBufferItemType >
{
    public:
        HConsumerBufferHandler_WaitWithTimeout():fNAttempts(100),fSleepDurationNanoSeconds(100){;};
        virtual ~HConsumerBufferHandler_WaitWithTimeout(){;};

        //total time-out wait time will be fNAttempts*fSleepDurationNanoSeconds
        void SetNAttempts(unsigned int n){fNAttempts = n;};
        unsigned int GetNAttempts() const {return fNAttempts;};

        void SetSleepDurationNanoSeconds(unsigned int ns){fSleepDurationNanoSeconds = ns;};
        unsigned int GetSleepDurationNanoSeconds() const {return fSleepDurationNanoSeconds;};

        HConsumerBufferPolicyCode ReserveBuffer(HBufferPool<XBufferItemType>* pool, HLinearBuffer<XBufferItemType>*& buffer, unsigned int id=0)
        {
            if(pool->GetConsumerPoolSize(id) != 0)
            {
                buffer = pool->PopConsumerBuffer(id);
                return HConsumerBufferPolicyCode::success;
            }
            else
            {
                //wait for the consumer buffer pool to become empty
                unsigned int count = 0;
                while( pool->GetConsumerPoolSize(id) != 0 && count < fNAttempts)
                {
                    //sleep for the specified duration if it is non-zero
                    if(fSleepDurationNanoSeconds != 0)
                    {
                        std::this_thread::sleep_for(std::chrono::nanoseconds(fSleepDurationNanoSeconds));
                    }

                    if(pool->GetConsumerPoolSize(id) != 0)
                    {
                        buffer = pool->PopConsumerBuffer(id);
                        return HConsumerBufferPolicyCode::success;
                    }

                    count++;
                };

                //last attempt to grab buffer
                if(pool->GetConsumerPoolSize(id) != 0)
                {
                    buffer = pool->PopConsumerBuffer();
                    return HConsumerBufferPolicyCode::success;
                }
                else
                {
                    buffer = nullptr;
                    return HConsumerBufferPolicyCode::fail;
                }
            }
        }

    protected:

        unsigned int fNAttempts;
        unsigned int fSleepDurationNanoSeconds;
};





}


#endif /* end of include guard: HConsumerBufferHandlerPolicy */
