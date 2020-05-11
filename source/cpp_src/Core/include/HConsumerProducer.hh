#ifndef HConsumerProducer_HH__
#define HConsumerProducer_HH__

#include <mutex>
#include <thread>

#include "HBufferPool.hh"
#include "HThreadPool.hh"
#include "HConsumerBufferHandlerPolicy.hh"
#include "HProducerBufferHandlerPolicy.hh"

namespace hops
{

/*
*File: HConsumerProducer.hh
*Class: HConsumerProducer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:fManagementThread
*Description:
*/

template< typename XSourceBufferItemType, typename XSinkBufferItemType, typename XProducerSinkBufferHandlerPolicyType, typename XConsumerSourceBufferHandlerPolicyType> 
class HConsumerProducer: public HRegisteredConsumer, public HThreadPool
{
    public:

        HConsumerProducer():
            HRegisteredConsumer(),
            HThreadPool(),
            fStopConsumptionProduction(false),
            fManagementThread(),
            fSourceBufferPool(nullptr),
            fSinkBufferPool(nullptr)
        {};

        virtual ~HConsumerProducer(){};

        //source is where consumer grabs new data
        void SetSourceBufferPool(HBufferPool<XSourceBufferItemType>* buffer_pool)
        {
            fSourceBufferPool = buffer_pool;
            fSourceBufferPool->RegisterConsumer(this);
        };

        HBufferPool<XSourceBufferItemType>* GetSourceBufferPool() {return fSourceBufferPool;};
        const HBufferPool<XSourceBufferItemType>* GetSourceBufferPool() const {return fSourceBufferPool;};

        //sink is where the producer dumps results
        void SetSinkBufferPool(HBufferPool<XSinkBufferItemType>* buffer_pool)
        {
            fSinkBufferPool = buffer_pool;
        };
        HBufferPool<XSinkBufferItemType>* GetSinkBufferPool() {return fSinkBufferPool;};
        const HBufferPool<XSinkBufferItemType>* GetSinkBufferPool() const {return fSinkBufferPool;};

        //start the producer running (in a separate thread in the background)
        void StartConsumptionProduction()
        {
            //launch the work threads in the thread pool
            Launch();

            //now run the management thread (responsible for generation of work for the thread pool)
            fStopConsumptionProduction = false;
            fManagementThread = std::thread(&HConsumerProducer::ConsumeProduceWork,this);
        }

        //stop the producer and join thread
        void StopConsumptionProduction()
        {
            //signal termination to thread pool
            SignalTerminateOnComplete();

            //signal and stop the management thread
            fStopConsumptionProduction = true;
            fManagementThread.join();

            //join the thread pool
            Join();
        }

        //stop the producer and join thread
        void ForceStopConsumptionProduction()
        {
            //kill the thread pool
            ForceTermination();

            //signal and stop the management thread
            fStopConsumptionProduction = true;
            fManagementThread.join();

            //join the thread pool
            Join();
        }

    protected:

        void ConsumeProduceWork()
        {
            ConfigureSinkBufferHandler();
            ConfigureSourceBufferHandler();

            ExecutePreConsumptionProductionTasks();

            while(!fStopConsumptionProduction) 
            {
                //prepare things as needed (reserve buffer, etc)
                ExecutePreWorkTasks();

                //spawn off work associated with this buffer
                DoWork();

                //do post-work tasks (release the buffer, etc)
                ExecutePostWorkTasks();
            }

            ExecutePostConsumptionProductionTasks();
        }

        virtual void ExecutePreConsumptionProductionTasks(){};
        virtual void ExecutePostConsumptionProductionTasks(){};

        virtual void ConfigureSinkBufferHandler(){};
        virtual void ConfigureSourceBufferHandler(){};
        virtual void ExecutePreWorkTasks(){};
        virtual void DoWork(){}; //override if thread tasks are not independent
        virtual void ExecutePostWorkTasks(){};

        volatile bool fStopConsumptionProduction;
        std::thread fManagementThread;
        HBufferPool<XSourceBufferItemType>* fSourceBufferPool;
        HBufferPool<XSinkBufferItemType>* fSinkBufferPool;
        XProducerSinkBufferHandlerPolicyType fSourceBufferHandler;
        XConsumerSourceBufferHandlerPolicyType fSinkBufferHandler;


};

}

#endif /* end of include guard: HConsumerProducer */
