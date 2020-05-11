#include "HThreadPool.hh"

//need pthreads for thread native_handle (setting cpu affinity)
#include <pthread.h>
#include <iostream>

namespace hops
{

HThreadPool::HThreadPool():
    fHasLaunched(false),
    fNThreads(1),
    fNPhysicalCores(1),
    fSignalTerminate(false),
    fForceTerminate(false)
{
    fNPhysicalCores = std::thread::hardware_concurrency();
}

HThreadPool::~HThreadPool()
{

}

void
HThreadPool::Launch()
{
    if(fThreads.size() == 0 || fHasLaunched)
    {
        fThreadIdleMap.clear();
        fSignalTerminate = false;
        fForceTerminate = false;
        for(unsigned int i=0; i<fNThreads; i++)
        {
            fThreads.push_back( std::thread( &HThreadPool::ProcessLoop, this ) );
        }
        fHasLaunched = true;
    }
    else
    {
        std::cout<<"HThreadPool::Launch: Warning, threads already launched."<<std::endl;
    }
}

void
HThreadPool::SignalTerminateOnComplete()
{
    fSignalTerminate = true;
}

void
HThreadPool::ForceTermination()
{
    fForceTerminate = true;
}

void
HThreadPool::Join()
{
    unsigned int n_threads = fThreads.size();
    unsigned int n_stopped = 0;
    std::vector<unsigned int> stopped_ids;
    while(n_stopped != n_threads)
    {
        for(unsigned int i=0; i<n_threads; i++)
        {
            bool have_already_stopped = false;
            for(unsigned int j=0; j<stopped_ids.size(); j++)
            {
                if(stopped_ids[j] == i){have_already_stopped = true;};
            }

            if(!have_already_stopped)
            {
                auto It = fThreadIdleMap.find( fThreads[i].get_id() );
                if(It->second)
                {
                    fThreads[i].join();
                    n_stopped++;
                    stopped_ids.push_back(i);
                }
            }
        }
    }

    fThreads.clear();
    fThreadIdleMap.clear();
    fHasLaunched = false;
}

void HThreadPool::SetNThreads(unsigned int n)
{
    if(!fHasLaunched)
    {
        fNThreads = n;
    }
    else
    {
        //issue warning, cannot change the number of threads after we have launched them

        //TODO...could relax this restriction with some changes, adding more threads is no problem but reducing
        //requires individual kill flags for each thread, so we can signal it to die when idle

        std::cout<<"HThreadPool::SetNThreads: Error, cannot change the number of threads after launch."<<std::endl;
    }
}

//allow the kernel to schedule a particular thread on any cpu (this is the default behavior)
void
HThreadPool::AssociateThreadWithAllProcessors(unsigned int local_thread_id)
{
    if(fHasLaunched)
    {
        std::set<unsigned int> cpu_ids;
        for(unsigned int i=0; i<fNPhysicalCores; i++)
        {
            cpu_ids.insert(i);
        }
        AssociateThreadWithProcessorSet(local_thread_id, cpu_ids);
    }
    else
    {
        std::cout<<"HThreadPool::AssociateThreadWithAllProcessors: Error, cannot set thread affinity before launch."<<std::endl;
    }
}

//set cpu affinities for a particular thread
void
HThreadPool::AssociateThreadWithProcessorSet(unsigned int local_thread_id, const std::set< unsigned int >& cpu_id_set)
{
    if(fHasLaunched)
    {
        if(local_thread_id < fNThreads)
        {
            //construct the cpu_set
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            for(auto iter = cpu_id_set.begin(); iter != cpu_id_set.end(); iter++)
            {
                unsigned int cpu_id = *iter;
                //issue warning if cpu_id is more than number of processors
                if(fNPhysicalCores <= cpu_id)
                {
                    std::cout<<"HThreadPool::AssociateThreadWithProcessorSet: Warning, cpu_id: "<<cpu_id<<" exceeds the number of physical cores: "<<fNPhysicalCores<<std::endl;
                }
                cpu_set_t temp_cpuset;
                CPU_ZERO(&temp_cpuset);
                CPU_SET((cpu_id)%fNPhysicalCores, &temp_cpuset);
                CPU_AND(&cpuset, &cpuset, &temp_cpuset);
            }

            int rc = pthread_setaffinity_np(fThreads[local_thread_id].native_handle(), sizeof(cpu_set_t), &cpuset);
            if(rc != 0)
            {
                std::cout<<"HThreadPool::AssociateThreadWithProcessorSet: Error, could not set thread affinity."<<std::endl;
            }
        }
    }
    else
    {
        std::cout<<"HThreadPool::AssociateThreadWithProcessorSet: Error, cannot set thread affinity before launch."<<std::endl;
    }
}


void
HThreadPool::AssociateThreadWithSingleProcessor(unsigned int local_thread_id, unsigned int cpu_id)
{
    if(fHasLaunched)
    {
        if(local_thread_id < fNThreads)
        {
            //construct the cpu_set
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            //issue warning if cpu_id is more than number of processors
            if(fNPhysicalCores <= cpu_id)
            {
                std::cout<<"HThreadPool::AssociateThreadWithSingleProcessor: Warning, cpu_id: "<<cpu_id<<" exceeds the number of physical cores: "<<fNPhysicalCores<<std::endl;
            }
            //set cpu id modulo the number of cores
            CPU_SET((cpu_id)%fNPhysicalCores, &cpuset);
            int rc = pthread_setaffinity_np(fThreads[local_thread_id].native_handle(), sizeof(cpu_set_t), &cpuset);
            if(rc != 0)
            {
                std::cout<<"HThreadPool::AssociateThreadWithSingleProcessor: Error, could not set thread affinity."<<std::endl;
            }
            else
            {
                std::cout<<"HThreadPool::AssociateThreadWithSingleProcessor: successfully set thread: "<<local_thread_id<<" affinity to core: "<<cpu_id<<"."<<std::endl;
            }
        }
    }
    else
    {
        std::cout<<"HThreadPool::AssociateThreadWithSingleProcessor: Error, cannot set thread affinity before launch."<<std::endl;
    }
}


void
HThreadPool::InsertIdleIndicator()
{
    std::lock_guard< std::mutex > lock(fIdleMutex);
    fThreadIdleMap.insert( std::pair<std::thread::id, bool>( std::this_thread::get_id(), false) );
}

void
HThreadPool::SetIdleIndicatorFalse()
{
    std::lock_guard< std::mutex > lock(fIdleMutex);
    auto indicator = fThreadIdleMap.find( std::this_thread::get_id() );
    indicator->second = false;
}

void
HThreadPool::SetIdleIndicatorTrue()
{
    std::lock_guard< std::mutex > lock(fIdleMutex);
    auto indicator = fThreadIdleMap.find( std::this_thread::get_id() );
    indicator->second = true;
}

bool
HThreadPool::AllThreadsAreIdle()
{
    std::lock_guard< std::mutex > lock(fIdleMutex);
    for(auto it=fThreadIdleMap.begin(); it != fThreadIdleMap.end(); it++)
    {
        if(it->second == false)
        {
            return false;
        }
    }
    return true;
}

void
HThreadPool::ProcessLoop()
{
    //basic processing loop...we assume the derived class has put all actual work
    //into the virtual function ExecuteThreadTask()

    //register this thread with the idle indicator
    InsertIdleIndicator();
    SetIdleIndicatorTrue();

    //loop until we are done
    while( !fForceTerminate && (!fSignalTerminate || WorkPresent() ) )
    {
        if( WorkPresent() )
        {
            SetIdleIndicatorFalse();
            ExecuteThreadTask(); //perform granular unit of work
            SetIdleIndicatorTrue();
        }
        else
        {
            SetIdleIndicatorTrue();
            Idle();
        }
    }

    SetIdleIndicatorTrue();
}


}
