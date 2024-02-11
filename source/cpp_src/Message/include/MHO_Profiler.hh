#ifndef MHO_Profiler_HH__
#define MHO_Profiler_HH__

#include <cstdlib>
#include <string>
#include <mutex>
#include <vector>
#include <cstring>
#include <iostream>
#include "MHO_SelfName.hh"
#include "MHO_Timer.hh"

/*
*File: MHO_Profiler.hh
*Class: MHO_Profiler
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#define HOPS_USE_PROFILER

namespace hops
{

namespace sn = selfname;

enum
MHO_ProfilerFlag: int
{
    pStartFlag = 1, //start time for this segment
    pStopFlag = 2 //stop timer for this segment
};

//short hand aliases
static const MHO_ProfilerFlag pStart = MHO_ProfilerFlag::pStartFlag;
static const MHO_ProfilerFlag pStop = MHO_ProfilerFlag::pStopFlag;

using hops::pStart;
using hops::pStop;

#define PROFILE_INFO_LEN 128

//uses the singleton pattern (as we only have one terminal)
class MHO_Profiler
{

    public:
        //since this is a singleton we need to remove ability to copy/move
        MHO_Profiler(MHO_Profiler const&) = delete;
        MHO_Profiler(MHO_Profiler&&) = delete;
        MHO_Profiler& operator=(MHO_Profiler const&) = delete;
        MHO_Profiler& operator=(MHO_Profiler&&) = delete;

        //provide public access to the only static instance
        static MHO_Profiler& GetInstance()
        {
            if(fInstance == nullptr){fInstance = new MHO_Profiler();}
            return *fInstance;
        }

        // std:size_t GetNThreads(){return fNThreads;}
        // void SetNThreads(std::size_t nthreads)
        // {
        //     fNThreads = nthreads;
        //     fThreadEvents.resize(fNThreads);
        // }

        // //TODO we need to eliminate these locks...would need a lock free map impl
        // void Lock(){fMutex.lock();};
        // void Unlock(){fMutex.unlock();};

        void AddEntry(int flag, int thread_id, std::string filename, int line_num, std::string func_name);
        void DumpEvents();

    private:

        MHO_Profiler():fNThreads(1)
        {
            // fEvents.resize(fNThreads);
            fTimer.Start();
        };
        virtual ~MHO_Profiler(){};


        std::mutex fMutex;
        static MHO_Profiler* fInstance; //static global class instance
        std::size_t fNThreads;

        struct ProfileEvent 
        {
            int fFlag; //indicates start/stop
            int fLineNumber; //line number of the file
            int fThreadID;
            double fTime;
            char fFilename[PROFILE_INFO_LEN]; //truncated filename
            char fFuncname[PROFILE_INFO_LEN]; //truncated function name
        };

        //map each thread to a vector of events
        // std::vector< std::vector< ProfileEvent > > fThreadEvents;    
        std::vector< ProfileEvent > fEvents;


        MHO_Timer fTimer;

};



#ifdef HOPS_USE_PROFILER

    //abuse do-while for multiline macros
    #define profiler_start() \
    do { \
        MHO_Profiler::GetInstance().AddEntry(pStart, 0, std::string( sn::file_basename(__FILE__) ), __LINE__ , std::string(  __PRETTY_FUNCTION__ ) ); \
    } \
    while(0)

    #define profiler_stop() \
    do { \
        MHO_Profiler::GetInstance().AddEntry(pStop, 0, std::string( sn::file_basename(__FILE__) ), __LINE__ , std::string(  __PRETTY_FUNCTION__ ) ); \
    } \
    while(0)

#else

    //profiling is not turned on, ifdef out of compilation
    #define profiler_start()
    #define profiler_stop()

#endif

}//end of namespace

#endif /* end of include guard: MHO_Profiler */
