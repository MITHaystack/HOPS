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
 * when the profile is enabled, a section of code can be profiled by
 * wrapping it between a call to profiler_start() and profiler_stop(), for example:
 *
 * profiler_start();
 * function_to_profile();
 * profiler_stop();
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

/**
 * @brief Class MHO_ProfileEvent
 */
struct MHO_ProfileEvent
{
        int fFlag;       //indicates start/stop
        int fLineNumber; //line number of the file
        uint64_t fThreadID;
        double fTime;
        char fFilename[PROFILE_INFO_LEN]; //truncated filename
        char fFuncname[PROFILE_INFO_LEN]; //truncated function name
};

/**
 * @brief Class MHO_Profiler - uses the singleton pattern
 */
class MHO_Profiler
{

    public:
        //since this is a singleton we need to remove ability to copy/move
        MHO_Profiler(MHO_Profiler const&) = delete;
        MHO_Profiler(MHO_Profiler&&) = delete;
        MHO_Profiler& operator=(MHO_Profiler const&) = delete;
        MHO_Profiler& operator=(MHO_Profiler&&) = delete;

        /**
         * @brief provide public access to the only static instance
         *
         * @return Reference to the singleton instance of MHO_Profiler
         * @note This is a static function.
         */
        static MHO_Profiler& GetInstance()
        {
            if(fInstance == nullptr)
            {
                fInstance = new MHO_Profiler();
            }
            return *fInstance;
        }

        /**
         * @brief Sets the enabled state to true.
         */
        void Enable() { fDisabled = false; };

        /**
         * @brief Sets HOPS_COLOR_MSG to disabled state.
         */
        void Disable() { fDisabled = true; };

        /**
         * @brief Checks if the feature is enabled.
         *
         * @return True if enabled, false otherwise.
         */
        bool IsEnabled() const { return !fDisabled; }

        /**
         * @brief Acquires a lock using fMutex for thread synchronization.
         * TODO we need to eliminate the need for locks...which sort of interferes
         * with the objective of profiling. However, to do that we would
         * probably need a lock free map implementation in order to map the
         * thread_id's to a local index and push the events into a thread-specific vector
         * so for now, mutex it is...
         */
        void Lock() { fMutex.lock(); };

        /**
         * @brief Releases control of the mutex.
         */
        void Unlock() { fMutex.unlock(); };

        /**
         * @brief Adds a profiling event to the internal list if not disabled.
         *
         * @param flag A flag indicating the type of event (e.g., stop timer for this segment).
         * @param thread_id The ID of the thread generating the event.
         * @param filename The truncated filename associated with the event.
         * @param line_num The line number in the source code where the event occurred.
         * @param func_name The function name associated with the event.
         */
        void AddEntry(int flag, uint64_t thread_id, std::string filename, int line_num, std::string func_name);

        /**
         * @brief Getter for events - at end of program, retrieve and utilize the profiler events
         *
         * @param events Reference to std::vector<MHO_ProfileEvent to store retrieved events
         */
        void GetEvents(std::vector< MHO_ProfileEvent >& events) { events = fEvents; }

        /**
         * @brief Prints all stored events along with their details.
         */
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
