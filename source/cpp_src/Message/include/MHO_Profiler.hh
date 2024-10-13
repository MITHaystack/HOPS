#ifndef MHO_Profiler_HH__
#define MHO_Profiler_HH__

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "MHO_SelfName.hh"
#include "MHO_Timer.hh"

#define HOPS_USE_PROFILER

namespace hops
{

/*!
 *@file MHO_Profiler.hh
 *@class MHO_Profiler
 *@author J. Barrett - barrettj@mit.edu
 *@date Sat Feb 10 11:17:05 2024 -0500
 *@brief class for running time/performance profiling
 */

namespace sn = selfname;

enum MHO_ProfilerFlag : int
{
    pStartFlag = 1, //start time for this segment
    pStopFlag = 2   //stop timer for this segment
};

//short hand aliases
static const MHO_ProfilerFlag pStart = MHO_ProfilerFlag::pStartFlag;
static const MHO_ProfilerFlag pStop = MHO_ProfilerFlag::pStopFlag;

using hops::pStart;
using hops::pStop;

#define PROFILE_INFO_LEN 128

struct MHO_ProfileEvent
{
        int fFlag;       //indicates start/stop
        int fLineNumber; //line number of the file
        uint64_t fThreadID;
        double fTime;
        char fFilename[PROFILE_INFO_LEN]; //truncated filename
        char fFuncname[PROFILE_INFO_LEN]; //truncated function name
};

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
            if(fInstance == nullptr)
            {
                fInstance = new MHO_Profiler();
            }
            return *fInstance;
        }

        void Enable() { fDisabled = false; };

        void Disable() { fDisabled = true; };

        bool IsEnabled() const { return !fDisabled; }

        //TODO we need to eliminate the need for locks...which sort of interferes
        //with the objective of profiling. However, to do that we would
        //probably need a lock free map implementation in order to map the
        //thread_id's to a local index and push the events into a thread-specific vector
        //so for now, mutex it is...
        void Lock() { fMutex.lock(); };

        void Unlock() { fMutex.unlock(); };

        void AddEntry(int flag, uint64_t thread_id, std::string filename, int line_num, std::string func_name);

        //add end of program, retrieve and utilize the profiler events
        void GetEvents(std::vector< MHO_ProfileEvent >& events) { events = fEvents; }

        void DumpEvents();

    private:
        MHO_Profiler(): fNThreads(1)
        {
            fDisabled = true; //disabled by default
            fEvents.reserve(1000);
            fTimer.Start();
        };

        virtual ~MHO_Profiler(){};

        std::mutex fMutex;
        static MHO_Profiler* fInstance; //static global class instance
        std::size_t fNThreads;
        bool fDisabled;

        //map each thread to a vector of events
        // std::vector< std::vector< MHO_ProfileEvent > > fThreadEvents;
        std::vector< MHO_ProfileEvent > fEvents;

        MHO_Timer fTimer;
};

#ifdef HOPS_USE_PROFILER

    //abuse do-while for multiline macros
    #define profiler_start()                                                                                                   \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Profiler::GetInstance().Lock();                                                                                \
            MHO_Profiler::GetInstance().AddEntry(pStart, std::hash< std::thread::id >{}(std::this_thread::get_id()),           \
                                                 std::string(sn::file_basename(__FILE__)), __LINE__,                           \
                                                 std::string(__PRETTY_FUNCTION__));                                            \
            MHO_Profiler::GetInstance().Unlock();                                                                              \
        }                                                                                                                      \
        while(0)

    #define profiler_stop()                                                                                                    \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Profiler::GetInstance().Lock();                                                                                \
            MHO_Profiler::GetInstance().AddEntry(pStop, std::hash< std::thread::id >{}(std::this_thread::get_id()),            \
                                                 std::string(sn::file_basename(__FILE__)), __LINE__,                           \
                                                 std::string(__PRETTY_FUNCTION__));                                            \
            MHO_Profiler::GetInstance().Unlock();                                                                              \
        }                                                                                                                      \
        while(0)

#else

    //profiling is not turned on, ifdef out of compilation
    #define profiler_start()
    #define profiler_stop()

#endif

} // namespace hops

#endif /*! end of include guard: MHO_Profiler */
