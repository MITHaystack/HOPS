#ifndef MHO_Profiler_HH__
#define MHO_Profiler_HH__

#include <cstdlib>
#include <ostream>
#include <sstream>
#include <string>
#include <mutex>


#include "MHO_SelfName.hh"

/*
*File: MHO_Profiler.hh
*Class: MHO_Profiler
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

namespace hops
{


enum
MHO_ProfilerFlag: int
{
    eStart = 1, //start time for this segment
    eStop = 2 //stop timer for this segment
};

//short hand aliases
static const MHO_ProfilerFlag pStart = MHO_ProfilerFlag::eSpecialLevel;
static const MHO_ProfilerFlag pStop = MHO_ProfilerFlag::eSilentErrorLevel;
using hops::eStart;
using hops::eStop;


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

        //TODO we need to eliminate these locks...would need a lock free map impl
        void Lock(){fMutex.lock();};
        void Unlock(){fMutex.unlock();};

        void AddEntry(int flag, int thread_id, std::string filename, std::string func_name);

    private:

        //no public access to constructor
        //set up the stream, for now just point to std::cout
        //but we may want to allow this to be configured post-construction
        //perhaps we should also pipe information into log file(s)
        MHO_Profiler():
        {


        };
        virtual ~MHO_Profiler(){};


        std::mutex fMutex;

        static MHO_Profiler* fInstance; //static global class instance
        std::ostream* fTerminalStream; //stream to terminal output
        std::set< std::string > fKeys; //keys of which messages we will accept for output
        MHO_ProfilerLevel fAllowedLevel;

};


//abuse do-while for multiline message macros

    // #define msg_fatal(xKEY, xCONTENT) \
    // do { \
    //     MHO_Profiler::GetInstance().Lock(); \
    //     MHO_Profiler::GetInstance().SendMessage(eFatal,xKEY) << "(" << file_base_name(__FILE__) << ":" << __LINE__ << ") " << xCONTENT; \
    //     MHO_Profiler::GetInstance().Unlock(); \
    // } \
    // while(0)
    // 
    // //ERROR/////////////////////////////////////////////////////////////////////////
    // #define msg_error(xKEY, xCONTENT) \
    // do { \
    //     MHO_Profiler::GetInstance().Lock(); \
    //     MHO_Profiler::GetInstance().SendMessage(eError,xKEY) << "(" << file_base_name(__FILE__) << ":" << __LINE__ << ") " << xCONTENT; \
    //     MHO_Profiler::GetInstance().Unlock(); \
    // } \
    // while(0)
    // 
    // //WARNING///////////////////////////////////////////////////////////////////////
    // #define msg_warn(xKEY, xCONTENT) \
    // do { \
    //     MHO_Profiler::GetInstance().Lock(); \
    //     MHO_Profiler::GetInstance().SendMessage(eWarning,xKEY) << "(" << file_base_name(__FILE__) << ":" << __LINE__ << ") " << xCONTENT; \
    //     MHO_Profiler::GetInstance().Unlock(); \
    // } \
    // while(0)
    // 
    // //STATUS////////////////////////////////////////////////////////////////////////
    // #define msg_status(xKEY, xCONTENT) \
    // do { \
    //     MHO_Profiler::GetInstance().Lock(); \
    //     MHO_Profiler::GetInstance().SendMessage(eStatus,xKEY) << "(" << file_base_name(__FILE__) << ":" << __LINE__ << ") " << xCONTENT; \
    //     MHO_Profiler::GetInstance().Unlock(); \
    // } \
    // while(0)
    // 
    // //INFO//////////////////////////////////////////////////////////////////////////
    // #define msg_info(xKEY, xCONTENT) \
    // do { \
    //     MHO_Profiler::GetInstance().Lock(); \
    //     MHO_Profiler::GetInstance().SendMessage(eInfo,xKEY) << "(" << file_base_name(__FILE__) << ":" << __LINE__ << ") " << xCONTENT; \
    //     MHO_Profiler::GetInstance().Unlock(); \
    // } \
    // while(0)
    // 
    // //allow debug messages when debug flag is active
    // #ifdef HOPS_ENABLE_DEBUG_MSG  //this is defined as a compiler flag via build system
    // 
    // //DEBUG/////////////////////////////////////////////////////////////////////////
    // #define msg_debug(xKEY, xCONTENT) \
    //     do { \
    //         MHO_Profiler::GetInstance().Lock(); \
    //         MHO_Profiler::GetInstance().SendMessage(eDebug,xKEY) << "(" << file_base_name(__FILE__) << ":" << __LINE__ << ") " << xCONTENT; \
    //         MHO_Profiler::GetInstance().Unlock(); \
    //     } \
    //     while(0)
    // #else
    // //debug is not enabled, so we remove them from compilation
    // #define msg_debug(xKEY, xCONTENT)
    // #endif


}//end of namespace

#endif /* end of include guard: MHO_Profiler */
