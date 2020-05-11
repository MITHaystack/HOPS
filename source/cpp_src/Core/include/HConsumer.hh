#ifndef HConsumer_HH__
#define HConsumer_HH__

#include <mutex>
#include <thread>
#include <unistd.h>

#include "HBufferPool.hh"
#include "HThreadPool.hh"
#include "HConsumerBufferHandlerPolicy.hh"

namespace hops
{

/*
*File: HConsumer.hh
*Class: HConsumer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

template< typename XBufferItemType, typename XConsumerBufferHandlerPolicyType > 
class HConsumer: public HRegisteredConsumer, public HThreadPool
{
    public:

        HConsumer():
            HRegisteredConsumer(),
            HThreadPool(),
            fStopConsumption(false),
            fConsumptionManagementThread(),
            fBufferPool(nullptr)
        {};

        virtual ~HConsumer(){};

        void SetBufferPool(HBufferPool<XBufferItemType>* buffer_pool)
        {
            fBufferPool = buffer_pool;
            fBufferPool->RegisterConsumer(this);
        };

        HBufferPool<XBufferItemType>* GetBufferPool() {return fBufferPool;};
        const HBufferPool<XBufferItemType>* GetBufferPool() const {return fBufferPool;};

        //start the producer running (in a separate thread in the background)
        void StartConsumption()
        {
            //launch the work threads in the thread pool
            Launch();

            //now run the management thread (responsible for generation of work for the thread pool)
            fStopConsumption = false;
            fConsumptionManagementThread = std::thread(&HConsumer::ConsumeWork,this);
        }

        //stop the producer and join thread
        void StopConsumption()
        {
            //signal termination to thread pool
            SignalTerminateOnComplete();
            //signal and stop the management thread
            fStopConsumption = true;
            fConsumptionManagementThread.join();

            //join the thread pool
            Join();
        }

        //stop the producer and join thread
        void ForceStopConsumption()
        {
            //kill the thread pool
            ForceTermination();
            //signal and stop the management thread
            fStopConsumption = true;
            fConsumptionManagementThread.join();

            //join the thread pool
            Join();
        }

    protected:

        void ConsumeWork()
        {
            while(!fStopConsumption)
            {
                DoWork();
            }
        }

        virtual void DoWork(){ usleep(10); }; //manages work items for the threads (this may do nothing if all threads can do work independently)

        volatile bool fStopConsumption;
        std::thread fConsumptionManagementThread;
        HBufferPool<XBufferItemType>* fBufferPool;
        XConsumerBufferHandlerPolicyType fBufferHandler;


};

}

#endif /* end of include guard: HConsumer */
