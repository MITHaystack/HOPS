#ifndef HDigitizer_HH__
#define HDigitizer_HH__

#include <mutex>

#include "HLinearBuffer.hh"
#include "HBufferAllocatorBase.hh"

namespace hops {

/*
*File: HDigitizer.hh
*Class: HDigitizer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

//enum of return codes
enum class HDigitizerErrorCode
{
    success,
    card_buffer_overflow,
    host_buffer_overflow,
    premature_stop,
    unknown
};


template< typename XSampleType, typename XConcreteDigitizerType >
class HDigitizer
{
    public:

        HDigitizer():
            fBuffer(nullptr),
            fHaveBufferLock(false),
            fAllocator(nullptr)
        {};

        virtual ~HDigitizer()
        {
            //release lock on this buffer, if we still have one
            if(fHaveBufferLock && fBuffer != nullptr)
            {
                fBuffer->fMutex.unlock();
                fHaveBufferLock = false;
            }
        };

        //sample type and allocator type
        using sample_type = XSampleType;
        using allocator_type = HBufferAllocatorBase< XSampleType >;

        //buffer getter/setters
        void SetBuffer(HLinearBuffer< XSampleType >* buffer)
        {
            std::lock_guard<std::mutex> lock( fMutex );
            //if have a buffer which is already locked release it
            //before we drop this buffer
            if(fHaveBufferLock && fBuffer != nullptr)
            {
                fBuffer->fMutex.unlock();
                fHaveBufferLock = false;
            }
            //acquire lock on this buffer
            buffer->fMutex.lock();
            fHaveBufferLock = true;
            fBuffer = buffer;
        };

        HLinearBuffer< XSampleType >* GetBuffer() {return fBuffer;};
        const HLinearBuffer< XSampleType >* GetBuffer() const {return fBuffer;};

        //each digitizer must provide access to a buffer allocator
        //the XConcreteDigitizerType is responsible for constructing and setting fAllocator
        HBufferAllocatorBase< XSampleType >* GetAllocator() {return fAllocator;};

        //configure and initialize the device
        bool Initialize()
        {
            std::lock_guard<std::mutex> lock( fMutex );
            bool retval = static_cast< XConcreteDigitizerType* >(this)->InitializeImpl(); 
            return retval;
        }

        //arm for trigger, or start acquring data
        void Acquire()
        {
            std::lock_guard<std::mutex> lock( fMutex );
            static_cast< XConcreteDigitizerType* >(this)->AcquireImpl(); 
        }

        //transfer data to buffer, this may be a blocking or non-blocking call
        void Transfer()
        {
            std::lock_guard<std::mutex> lock( fMutex );
            static_cast< XConcreteDigitizerType* >(this)->TransferImpl(); 
        }

        //finish transfer to buffer, this is a blocking call
        HDigitizerErrorCode Finalize()
        {
            std::lock_guard<std::mutex> lock( fMutex );
            HDigitizerErrorCode ret_val = static_cast< XConcreteDigitizerType* >(this)->FinalizeImpl();
            //release lock on this buffer
            if(fHaveBufferLock && fBuffer != nullptr)
            {
                fBuffer->fMutex.unlock();
                fHaveBufferLock = false;
            }
            return ret_val;
        }

        //stop acquisition
        void Stop()
        {
            std::lock_guard<std::mutex> lock( fMutex );
            static_cast< XConcreteDigitizerType* >(this)->StopImpl();
            //release lock on this buffer
            if(fHaveBufferLock && fBuffer != nullptr)
            {
                fBuffer->fMutex.unlock();
                fHaveBufferLock = false;
            }
        }

        //tear down device for quitting
        void TearDown()
        {
            std::lock_guard<std::mutex> lock( fMutex );
            static_cast< XConcreteDigitizerType* >(this)->TearDownImpl();
            //release lock on this buffer
            if(fHaveBufferLock && fBuffer != nullptr)
            {
                fBuffer->fMutex.unlock();
                fHaveBufferLock = false;
            }
        }

        //should override this!
        virtual double GetSamplingFrequency() const {return 0.0;};

    protected:

        mutable std::mutex fMutex;
        //buffer memory should not be owned
        HLinearBuffer< XSampleType >* fBuffer;
        bool fHaveBufferLock;
        
        //pointer to buffer allocator, this must be set by the XConcreteDigitizerType class 
        HBufferAllocatorBase< XSampleType >* fAllocator;


};

}

#endif /* end of include guard: HDigitizer */
