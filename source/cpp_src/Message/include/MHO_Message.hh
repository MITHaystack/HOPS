#ifndef MHO_Message_HH__
#define MHO_Message_HH__

#include <cstdlib>
#include <ostream>
#include <sstream>
#include <string>
#include <iostream>
#include <set>

#include "MHO_TestAssertions.hh"

/*
*File: MHO_Message.hh
*Class: MHO_Message
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

namespace hops
{

class MHO_MessageNewline {};
class MHO_MessageEndline {};

static const MHO_MessageNewline ret = MHO_MessageNewline();
static const MHO_MessageEndline eom = MHO_MessageEndline();

enum
MHO_MessageLevel: int
{
    eFatalErrorLevel = 0, //use for fatal errors (program termination imminent)
    eErrorLevel = 1, //use for non-fatal errors which may lead to unexpected behavior
    eWarningLevel = 2, //use to inform about unexpected state which may lead to errors
    eStatusLevel = 3, //information about the current execution status of the program
    eInfoLevel = 4, //extra information to inform about configuration/state of program
    eDebugLevel = 5 //debug information of interest primarily to developer
};

//short hand aliases
static const MHO_MessageLevel eFatal = MHO_MessageLevel::eFatalErrorLevel;
static const MHO_MessageLevel eError = MHO_MessageLevel::eErrorLevel;
static const MHO_MessageLevel eWarning = MHO_MessageLevel::eWarningLevel;
static const MHO_MessageLevel eStatus = MHO_MessageLevel::eStatusLevel;
static const MHO_MessageLevel eInfo = MHO_MessageLevel::eInfoLevel;
static const MHO_MessageLevel eDebug = MHO_MessageLevel::eDebugLevel;

using hops::eFatal;
using hops::eError;
using hops::eWarning;
using hops::eStatus;
using hops::eInfo;
using hops::eDebug;

//uses the singleton pattern (as we only have one terminal)
//TODO make this class thread safe
class MHO_Message
{

    public:
        //since this is a singleton we need to remove ability to copy/move
        MHO_Message(MHO_Message const&) = delete;
        MHO_Message(MHO_Message&&) = delete;
        MHO_Message& operator=(MHO_Message const&) = delete;
        MHO_Message& operator=(MHO_Message&&) = delete;

        //provide public access to the only static instance
        static MHO_Message& GetInstance()
        {
            if(fInstance == nullptr){fInstance = new MHO_Message();}
            return *fInstance;
        }

        void AcceptAllKeys(){fAcceptAllKeys = true;}
        void LimitToKeySet(){fAcceptAllKeys = false;}
        void AddKey(const std::string& key);
        void AddKey(const char* key);
        void RemoveKey(const std::string& key);
        void RemoveKey(const char* key);
        void RemoveAllKeys();

        void Flush();
        void SetMessageLevel(MHO_MessageLevel level){fAllowedLevel = level;}

        MHO_Message& SendMessage(const MHO_MessageLevel& level, const std::string& key);
        MHO_Message& SendMessage(const MHO_MessageLevel& level, const char* key);

        template<class XStreamableItemType>
        MHO_Message& operator<<(const XStreamableItemType& item);

        MHO_Message& operator<<(const MHO_MessageNewline&);
        MHO_Message& operator<<(const MHO_MessageEndline&);

    private:

        //no public access to constructor
        //set up the stream, for now just point to std::cout
        //but we may want to allow this to be configured post-construction
        //perhaps we should also pipe information into log file(s)
        MHO_Message():
            fTerminalStream(&std::cout),
            fAllowedLevel(eStatus),
            fCurrentLevel(eInfo),
            fCurrentKeyIsAllowed(false),
            fAcceptAllKeys(false)
        {};
        virtual ~MHO_Message(){};

        bool PassMessage();
        std::string GetCurrentPrefix(const MHO_MessageLevel& level, const std::string& key);

        static MHO_Message* fInstance; //static global class instance
        std::ostream* fTerminalStream; //stream to terminal output
        std::set< std::string > fKeys; //keys of which messages we will accept for output
        MHO_MessageLevel fAllowedLevel;

        MHO_MessageLevel fCurrentLevel; //level of the current message
        bool fCurrentKeyIsAllowed; //current key is in allowed set
        bool fAcceptAllKeys;
        std::stringstream fMessageStream; //the message container to be filled/flushed


};


template<class XStreamableItemType>
MHO_Message&
MHO_Message::operator<<(const XStreamableItemType& item)
{
    if( PassMessage() )
    {
        fMessageStream << item;
    }
    return *fInstance;
}


//usage macros
#define msg_fatal(xKEY, xCONTENT) MHO_Message::GetInstance().SendMessage(eFatal,xKEY) << xCONTENT;
#define msg_error(xKEY, xCONTENT) MHO_Message::GetInstance().SendMessage(eError,xKEY) << xCONTENT;
#define msg_warn(xKEY, xCONTENT) MHO_Message::GetInstance().SendMessage(eWarning,xKEY) << xCONTENT;
#define msg_status(xKEY, xCONTENT) MHO_Message::GetInstance().SendMessage(eStatus,xKEY) << xCONTENT;
#define msg_info(xKEY, xCONTENT) MHO_Message::GetInstance().SendMessage(eInfo,xKEY) << xCONTENT;

#ifdef HOPS_ENABLE_DEBUG_MSG  //this is defined as a compiler flag via build system
//#warning "HOPS_ENABLE_DEBUG_MSG is defined -- this is not a bug!"
//allow debug messages when debug flag is active
#define msg_debug(xKEY, xCONTENT) MHO_Message::GetInstance().SendMessage(eDebug,xKEY) << xCONTENT;
#else
//debug is not enabled, so we remove them from compilation
#define msg_debug(xKEY, xCONTENT)
#endif


#ifdef HOPS_ENABLE_STEPWISE_CHECK  //this is defined as a compiler flag via build system
//error check is enabled, so that we can verify a boolean return value is true 
#define check_step_error(xVALUE, xKEY, xCONTENT) if(!xVALUE){ MHO_Message::GetInstance().SendMessage(eError,xKEY) << xCONTENT; }
#define check_step_fatal(xVALUE, xKEY, xCONTENT) if(!xVALUE){ MHO_Message::GetInstance().SendMessage(eFatal,xKEY) << xCONTENT; HOPS_ASSERT_THROW(xVALUE); }
#else
//error check is competely disabled 
#define check_step_error(xVALUE, xKEY, xCONTENT)
#define check_step_fatal(xVALUE, xKEY, xCONTENT)
#endif





}//end of namespace

#endif /* end of include guard: MHO_Message */
