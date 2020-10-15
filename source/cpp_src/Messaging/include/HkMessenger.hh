#ifndef HkMessenger_HH__
#define HkMessenger_HH__

#include <cstdlib>
#include <ostream>
#include <sstream>
#include <string>
#include <iostream>
#include <set>

/*
*File: HkMessenger.hh
*Class: HkMessenger
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

namespace hops
{

class HkMessageNewline {};
class HkMessageEndline {};

static const HkMessageNewline ret = HkMessageNewline();
static const HkMessageEndline eom = HkMessageEndline();

enum
HkMessageLevel: int
{
    eFatalErrorLevel = 0, //use for fatal errors (program termination imminent)
    eErrorLevel = 1, //use for non-fatal errors which may lead to unexpected behavior
    eWarningLevel = 2, //use to inform about unexpected state which may lead to errors
    eStatusLevel = 3, //information about the current execution status of the program
    eInfoLevel = 4, //extra information to inform about configuration/state of program
    eDebugLevel = 5 //debug information of interest primarily to developer
};

//short hand aliases
static const HkMessageLevel eFatal = HkMessageLevel::eFatalErrorLevel;
static const HkMessageLevel eError = HkMessageLevel::eErrorLevel;
static const HkMessageLevel eWarning = HkMessageLevel::eWarningLevel;
static const HkMessageLevel eStatus = HkMessageLevel::eStatusLevel;
static const HkMessageLevel eInfo = HkMessageLevel::eInfoLevel;
static const HkMessageLevel eDebug = HkMessageLevel::eDebugLevel;

using hops::eFatal;
using hops::eError;
using hops::eWarning;
using hops::eStatus;
using hops::eInfo;
using hops::eDebug;

//uses the singleton pattern (as we only have one terminal)
//TODO make this class thread safe
class HkMessenger
{

    public:
        //since this is a singleton we need to remove ability to copy/move
        HkMessenger(HkMessenger const&) = delete;
        HkMessenger(HkMessenger&&) = delete;
        HkMessenger& operator=(HkMessenger const&) = delete;
        HkMessenger& operator=(HkMessenger&&) = delete;

        //provide public access to the only static instance
        static HkMessenger& GetInstance()
        {
            if(fInstance == nullptr){fInstance = new HkMessenger();}
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
        void SetMessageLevel(HkMessageLevel level){fAllowedLevel = level;}

        HkMessenger& SendMessage(const HkMessageLevel& level, const std::string& key);
        HkMessenger& SendMessage(const HkMessageLevel& level, const char* key);

        template<class XStreamableItemType>
        HkMessenger& operator<<(const XStreamableItemType& item);

        HkMessenger& operator<<(const HkMessageNewline&);
        HkMessenger& operator<<(const HkMessageEndline&);

    private:

        //no public access to constructor
        //set up the stream, for now just point to std::cout
        //but we may want to allow this to be configured post-construction
        //perhaps we should also pipe information into log file(s)
        HkMessenger():
            fTerminalStream(&std::cout),
            fAllowedLevel(eStatus),
            fCurrentLevel(eInfo),
            fCurrentKeyIsAllowed(false),
            fAcceptAllKeys(false)
        {};
        virtual ~HkMessenger(){};

        bool PassMessage();
        std::string GetCurrentPrefix(const HkMessageLevel& level, const std::string& key);

        static HkMessenger* fInstance; //static global class instance
        std::ostream* fTerminalStream; //stream to terminal output
        std::set< std::string > fKeys; //keys of which messages we will accept for output
        HkMessageLevel fAllowedLevel;

        HkMessageLevel fCurrentLevel; //level of the current message
        bool fCurrentKeyIsAllowed; //current key is in allowed set
        bool fAcceptAllKeys;
        std::stringstream fMessageStream; //the message container to be filled/flushed


};


template<class XStreamableItemType>
HkMessenger&
HkMessenger::operator<<(const XStreamableItemType& item)
{
    if( PassMessage() )
    {
        fMessageStream << item;
    }
    return *fInstance;
}


//usage macros
#define msg_fatal(xKEY, xCONTENT) HkMessenger::GetInstance().SendMessage(eFatal,xKEY) << xCONTENT;
#define msg_error(xKEY, xCONTENT) HkMessenger::GetInstance().SendMessage(eError,xKEY) << xCONTENT;
#define msg_warn(xKEY, xCONTENT) HkMessenger::GetInstance().SendMessage(eWarning,xKEY) << xCONTENT;
#define msg_status(xKEY, xCONTENT) HkMessenger::GetInstance().SendMessage(eStatus,xKEY) << xCONTENT;
#define msg_info(xKEY, xCONTENT) HkMessenger::GetInstance().SendMessage(eInfo,xKEY) << xCONTENT;

#ifdef HOPS_ENABLE_DEBUG_MSG
//allow debug messages when debug flag is active
#define msg_debug(xKEY, xCONTENT) HkMessenger::GetInstance().SendMessage(eDebug,xKEY) << xCONTENT;
#else
//debug is not enabled, so we removed them from compilation
#define msg_debug(xKEY, xCONTENT)
#endif



}//end of namespace

#endif /* end of include guard: HkMessenger */
