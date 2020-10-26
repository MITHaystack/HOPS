#ifndef MHOMessage_HH__
#define MHOMessage_HH__

#include <cstdlib>
#include <ostream>
#include <sstream>
#include <string>
#include <iostream>
#include <set>

/*
*File: MHOMessage.hh
*Class: MHOMessage
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

namespace hops
{

class MHOMessageNewline {};
class MHOMessageEndline {};

static const MHOMessageNewline ret = MHOMessageNewline();
static const MHOMessageEndline eom = MHOMessageEndline();

enum
MHOMessageLevel: int
{
    eFatalErrorLevel = 0, //use for fatal errors (program termination imminent)
    eErrorLevel = 1, //use for non-fatal errors which may lead to unexpected behavior
    eWarningLevel = 2, //use to inform about unexpected state which may lead to errors
    eStatusLevel = 3, //information about the current execution status of the program
    eInfoLevel = 4, //extra information to inform about configuration/state of program
    eDebugLevel = 5 //debug information of interest primarily to developer
};

//short hand aliases
static const MHOMessageLevel eFatal = MHOMessageLevel::eFatalErrorLevel;
static const MHOMessageLevel eError = MHOMessageLevel::eErrorLevel;
static const MHOMessageLevel eWarning = MHOMessageLevel::eWarningLevel;
static const MHOMessageLevel eStatus = MHOMessageLevel::eStatusLevel;
static const MHOMessageLevel eInfo = MHOMessageLevel::eInfoLevel;
static const MHOMessageLevel eDebug = MHOMessageLevel::eDebugLevel;

using hops::eFatal;
using hops::eError;
using hops::eWarning;
using hops::eStatus;
using hops::eInfo;
using hops::eDebug;

//uses the singleton pattern (as we only have one terminal)
//TODO make this class thread safe
class MHOMessage
{

    public:
        //since this is a singleton we need to remove ability to copy/move
        MHOMessage(MHOMessage const&) = delete;
        MHOMessage(MHOMessage&&) = delete;
        MHOMessage& operator=(MHOMessage const&) = delete;
        MHOMessage& operator=(MHOMessage&&) = delete;

        //provide public access to the only static instance
        static MHOMessage& GetInstance()
        {
            if(fInstance == nullptr){fInstance = new MHOMessage();}
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
        void SetMessageLevel(MHOMessageLevel level){fAllowedLevel = level;}

        MHOMessage& SendMessage(const MHOMessageLevel& level, const std::string& key);
        MHOMessage& SendMessage(const MHOMessageLevel& level, const char* key);

        template<class XStreamableItemType>
        MHOMessage& operator<<(const XStreamableItemType& item);

        MHOMessage& operator<<(const MHOMessageNewline&);
        MHOMessage& operator<<(const MHOMessageEndline&);

    private:

        //no public access to constructor
        //set up the stream, for now just point to std::cout
        //but we may want to allow this to be configured post-construction
        //perhaps we should also pipe information into log file(s)
        MHOMessage():
            fTerminalStream(&std::cout),
            fAllowedLevel(eStatus),
            fCurrentLevel(eInfo),
            fCurrentKeyIsAllowed(false),
            fAcceptAllKeys(false)
        {};
        virtual ~MHOMessage(){};

        bool PassMessage();
        std::string GetCurrentPrefix(const MHOMessageLevel& level, const std::string& key);

        static MHOMessage* fInstance; //static global class instance
        std::ostream* fTerminalStream; //stream to terminal output
        std::set< std::string > fKeys; //keys of which messages we will accept for output
        MHOMessageLevel fAllowedLevel;

        MHOMessageLevel fCurrentLevel; //level of the current message
        bool fCurrentKeyIsAllowed; //current key is in allowed set
        bool fAcceptAllKeys;
        std::stringstream fMessageStream; //the message container to be filled/flushed


};


template<class XStreamableItemType>
MHOMessage&
MHOMessage::operator<<(const XStreamableItemType& item)
{
    if( PassMessage() )
    {
        fMessageStream << item;
    }
    return *fInstance;
}


//usage macros
#define msg_fatal(xKEY, xCONTENT) MHOMessage::GetInstance().SendMessage(eFatal,xKEY) << xCONTENT;
#define msg_error(xKEY, xCONTENT) MHOMessage::GetInstance().SendMessage(eError,xKEY) << xCONTENT;
#define msg_warn(xKEY, xCONTENT) MHOMessage::GetInstance().SendMessage(eWarning,xKEY) << xCONTENT;
#define msg_status(xKEY, xCONTENT) MHOMessage::GetInstance().SendMessage(eStatus,xKEY) << xCONTENT;
#define msg_info(xKEY, xCONTENT) MHOMessage::GetInstance().SendMessage(eInfo,xKEY) << xCONTENT;

#ifdef HOPS_ENABLE_DEBUG_MSG
//allow debug messages when debug flag is active
#define msg_debug(xKEY, xCONTENT) MHOMessage::GetInstance().SendMessage(eDebug,xKEY) << xCONTENT;
#else
//debug is not enabled, so we removed them from compilation
#define msg_debug(xKEY, xCONTENT)
#endif



}//end of namespace

#endif /* end of include guard: MHOMessage */
