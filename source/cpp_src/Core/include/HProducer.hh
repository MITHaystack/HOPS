#ifndef HProducer_HH__
#define HProducer_HH__

#include <mutex>
#include <thread>

#include "HBufferPool.hh"
#include "HThreadPool.hh"
#include "HProducerBufferHandlerPolicy.hh"

namespace hops
{

/*
*File: HProducer.hh
*Class: HProducer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

template< typename XBufferItemType, typename XProducerBufferHandlerPolicyType > 
class HProducer: public HThreadPool
{
    public:

        HProducer():
            HThreadPool(),
            fStopProduction(false),
            fStopAfterNextBuffer(false),
            fProductionManagementThread(),
            fBufferPool(nullptr)
        {};

        virtual ~HProducer(){};

        void SetBufferPool(HBufferPool<XBufferItemType>* buffer_pool){fBufferPool = buffer_pool;};
        HBufferPool<XBufferItemType>* GetBufferPool() {return fBufferPool;};
        const HBufferPool<XBufferItemType>* GetBufferPool() const {return fBufferPool;};

        //start the producer running (in a separate thread in the background)
        void StartProduction()
        {
            //launch the work threads in the thread pool
            Launch();

            //now run the management thread (responsible for generation of work for the thread pool)
            fStopProduction = false;
            fProductionManagementThread = std::thread(&HProducer::ProduceWork,this);
        }

        //stop the producer and join thread
        void StopProduction()
        {
            //signal termination to thread pool
            SignalTerminateOnComplete();
            //signal and stop the management thread
            fStopProduction = true;
            fProductionManagementThread.join();
            //join the thread pool
            Join();
        }

        //stop the producer and join thread
        void ForceStopProduction()
        {
            //kill the thread pool
            ForceTermination();
            //signal and stop the management thread
            fStopProduction = true;
            fProductionManagementThread.join();
            //join the thread pool
            Join();
        }

        //signaling bool
        void StopAfterNextBuffer(){fStopAfterNextBuffer = true;}

    protected:

        void ProduceWork()
        {
            ConfigureBufferHandler(fBufferHandler);
            ExecutePreProductionTasks();

            
            while(!fStopProduction) 
            {
                //prepare things as needed (reserve buffer, etc)
                ExecutePreWorkTasks();

                //spawn off work associated with this buffer
                DoWork();

                //do post-work tasks (release the buffer, etc)
                ExecutePostWorkTasks();

            }

            ExecutePostProductionTasks();

        }

        virtual void ExecutePreProductionTasks(){};
        virtual void ExecutePostProductionTasks(){};

        virtual void ConfigureBufferHandler(XProducerBufferHandlerPolicyType& /*handler*/){};
        virtual void ExecutePreWorkTasks(){};
        virtual void DoWork(){};
        virtual void ExecutePostWorkTasks(){};

        volatile bool fStopProduction;
        volatile bool fStopAfterNextBuffer;
        std::thread fProductionManagementThread;
        HBufferPool<XBufferItemType>* fBufferPool;
        XProducerBufferHandlerPolicyType fBufferHandler;


};

}

#endif /* end of include guard: HProducer */
