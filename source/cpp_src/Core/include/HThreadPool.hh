#ifndef HThreadPool_HH__
#define HThreadPool_HH__

#include <mutex>
#include <thread>
#include <vector>
#include <set>
#include <map>
#include <utility>

#include <unistd.h>

namespace hops
{

/*
*File: HThreadPool.hh
*Class: HThreadPool
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

class HThreadPool
{
    public:
        HThreadPool();
        virtual ~HThreadPool();

        void Launch();

        //calling this function will cause threads to exit the process loop once there is no longer work present
        void SignalTerminateOnComplete();

        //calling this function will cause threads to exit as soon as possible
        void ForceTermination();

        void Join();

        //thread <-> CPU affinity settings
        virtual void SetNThreads(unsigned int n);
        unsigned int GetNThreads() const {return fNThreads;};

        //set cpu affinities for a particular thread (default is any thread can be associated with any processor)
        void AssociateThreadWithAllProcessors(unsigned int local_thread_id);
        void AssociateThreadWithProcessorSet(unsigned int local_thread_id, const std::set< unsigned int >& cpu_id_set);
        void AssociateThreadWithSingleProcessor(unsigned int local_thread_id, unsigned int cpu_id);

        bool AllThreadsAreIdle();

    protected:

        void ProcessLoop();

        virtual void ExecuteThreadTask() = 0; //derived class must define work to be done
        virtual bool WorkPresent() = 0; //derived class must provide an indicator if there is useful work to be done
        virtual void Idle(){}; //derived class can define function for threads run while waiting for work

        void InsertIdleIndicator();
        void SetIdleIndicatorFalse();
        void SetIdleIndicatorTrue();

        //thread pool 
        bool fHasLaunched;
        unsigned int fNThreads;
        unsigned int fNPhysicalCores;
        volatile bool fSignalTerminate;
        volatile bool fForceTerminate;
        std::vector< std::thread > fThreads;
        std::mutex fIdleMutex;
        std::map< std::thread::id, bool > fThreadIdleMap;

};

}

#endif /* end of include guard: HThreadPool */
