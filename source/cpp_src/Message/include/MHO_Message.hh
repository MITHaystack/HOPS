#ifndef MHO_Message_HH__
#define MHO_Message_HH__

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <ostream>
#include <set>
#include <sstream>
#include <stdio.h>
#include <string>

#include "MHO_SelfName.hh"
#include "MHO_TestAssertions.hh"

//include the profiler here to make it visible just about everywhere
#include "MHO_Profiler.hh"

//MACROS handy for stringifying compiler defines
#define STR(str) #str
#define STRING(str) STR(str)

#ifdef HOPS_ENABLE_DEV_TODO
    #if defined(__clang__)
        #define DO_PRAGMA(x) _Pragma(#x)
        #define TODO_FIXME_MSG(x) DO_PRAGMA(message #x)
    #elif defined(__GNUC__)
        #define DO_PRAGMA(x) _Pragma(#x)
        #define TODO_FIXME_MSG(x) DO_PRAGMA(message #x)
    #else
        #error Unsupported compiler
    #endif
#else
    #define TODO_FIXME_MSG(x)
#endif

namespace hops
{

/*!
 *@file MHO_Message.hh
 *@class MHO_Message
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Oct 14 17:17:31 2020 -0400
 *@brief class for managing messages to stdout
 */

namespace sn = selfname;

/**
 * @brief Class MHO_MessageNewline
 */
class MHO_MessageNewline
{};

/**
 * @brief Class MHO_MessageEndline
 */
class MHO_MessageEndline
{};

static const MHO_MessageNewline ret = MHO_MessageNewline();
static const MHO_MessageNewline eol = MHO_MessageNewline();
static const MHO_MessageEndline eom = MHO_MessageEndline();

enum MHO_MessageLevel : int
{
    eSpecialLevel = -2,     //!special print level
    eSilentErrorLevel = -1, //!mute all messages entirely, including fatal ones
    eFatalErrorLevel = 0,   //!use for fatal errors (program termination imminent)
    eErrorLevel = 1,        //!use for non-fatal errors which may lead to unexpected behavior
    eWarningLevel = 2,      //!use to inform about unexpected state which may lead to errors
    eStatusLevel = 3,       //!information about the current execution status of the program
    eInfoLevel = 4,         //!extra information to inform about configuration/state of program
    eDebugLevel = 5         //!debug information of interest primarily to developer
};

//!short hand aliases
static const MHO_MessageLevel eSpecial = MHO_MessageLevel::eSpecialLevel;
static const MHO_MessageLevel eSilent = MHO_MessageLevel::eSilentErrorLevel;
static const MHO_MessageLevel eFatal = MHO_MessageLevel::eFatalErrorLevel;
static const MHO_MessageLevel eError = MHO_MessageLevel::eErrorLevel;
static const MHO_MessageLevel eWarning = MHO_MessageLevel::eWarningLevel;
static const MHO_MessageLevel eStatus = MHO_MessageLevel::eStatusLevel;
static const MHO_MessageLevel eInfo = MHO_MessageLevel::eInfoLevel;
static const MHO_MessageLevel eDebug = MHO_MessageLevel::eDebugLevel;

using hops::eDebug;
using hops::eError;
using hops::eFatal;
using hops::eInfo;
using hops::eSilent;
using hops::eSpecial;
using hops::eStatus;
using hops::eWarning;

//!uses the singleton pattern (as we only have one terminal)
class MHO_Message
{

    public:
        //! \brief Deleted copy constructor (singleton).
        MHO_Message(MHO_Message const&) = delete;
        //! \brief Deleted move constructor (singleton).
        MHO_Message(MHO_Message&&) = delete;
        //! \brief Deleted copy assignment operator (singleton).
        MHO_Message& operator=(MHO_Message const&) = delete;
        //! \brief Deleted move assignment operator (singleton).
        MHO_Message& operator=(MHO_Message&&) = delete;

        //! \brief Access the singleton instance of the message handler.
        //!
        //! \return Reference to the singleton MHO_Message instance.
        static MHO_Message& GetInstance()
        {
            if(fInstance == nullptr)
            {
                fInstance = new MHO_Message();
            }
            return *fInstance;
        }

        //! \brief Lock the message handler for thread-safe operations.
        void Lock() { fMutex.lock(); };

        //! \brief Unlock the message handler
        void Unlock() { fMutex.unlock(); };

        //! \brief Allow all message keys (no category/library filtering).
        void AcceptAllKeys() { fAcceptAllKeys = true; }

        //! \brief Restrict messages to a defined key set.
        void LimitToKeySet() { fAcceptAllKeys = false; }

        //! \brief Add a message key to the allowed key set.
        //! \param key The key string to add.
        void AddKey(const std::string& key);
        void AddKey(const char* key);

        //! \brief Remove a message key from the allowed key set.
        //! \param key The key string to remove.
        void RemoveKey(const std::string& key);
        void RemoveKey(const char* key);

        //! \brief Remove all message keys (disables all filtering).
        void RemoveAllKeys();

        //! \brief Flush the current output stream buffer.
        void Flush();

        //! \brief Set the allowed message level threshold.
        //! \param level The new minimum message level to accept.
        void SetMessageLevel(MHO_MessageLevel level) { fAllowedLevel = level; }

        //! \brief Set the message level using a legacy integer interface.
        //! \param legacy_message_level legacy integer representation of message level (lower is more verbose, min: -2, max: 5).
        void SetLegacyMessageLevel(int legacy_message_level);

        //! \brief Get the currently configured message level threshold.
        //! \return The currently allowed message level.
        MHO_MessageLevel GetMessageLevel() const { return fAllowedLevel; }

        //! \brief Begin a new message with a specific level and key.
        //!
        //! \param level The message level.
        //! \param key The message key to associate with this message.
        //! \return Reference to the MHO_Message instance for streaming content.
        MHO_Message& SendMessage(const MHO_MessageLevel& level, const std::string& key);
        MHO_Message& SendMessage(const MHO_MessageLevel& level, const char* key);

        //! \brief Stream an item into the current message.
        //!
        //! \tparam XStreamableItemType Any type that supports output streaming via `operator<<`.
        //! \param item The item to insert into the message stream.
        //! \return Reference to the MHO_Message instance for chaining.
        template< class XStreamableItemType > MHO_Message& operator<<(const XStreamableItemType& item);

        MHO_Message& operator<<(const MHO_MessageNewline&);
        MHO_Message& operator<<(const MHO_MessageEndline&);

    private:
        //no public access to constructor
        //set up the stream, for now just point to std::cout
        //but we may want to allow this to be configured post-construction
        //perhaps we should also pipe information into log file(s)
        MHO_Message()
            : fTerminalStream(&std::cout), fAllowedLevel(eStatus), fCurrentLevel(eInfo), fCurrentKeyIsAllowed(false),
              fAcceptAllKeys(false), fWasLastLineNewLine(false){};
        virtual ~MHO_Message(){};

        bool PassMessage();
        std::string GetCurrentPrefix(const MHO_MessageLevel& level, const std::string& key);

        std::mutex fMutex;

        static MHO_Message* fInstance; //static global class instance
        std::ostream* fTerminalStream; //stream to terminal output
        std::set< std::string > fKeys; //keys of which messages we will accept for output
        MHO_MessageLevel fAllowedLevel;

        MHO_MessageLevel fCurrentLevel; //level of the current message
        bool fCurrentKeyIsAllowed;      //current key is in allowed set
        bool fAcceptAllKeys;
        std::stringstream fMessageStream; //the message container to be filled/flushed

        static std::string fRed;         //fatal
        static std::string fYellow;      //error
        static std::string fOrange;      //orange
        static std::string fBlue;        //status
        static std::string fGreen;       //info
        static std::string fCyan;        //debug
        static std::string fWhite;       //default
        static std::string fColorSuffix; //color close

        bool fWasLastLineNewLine;
};

template< class XStreamableItemType > MHO_Message& MHO_Message::operator<<(const XStreamableItemType& item)
{
    if(PassMessage())
    {
        fMessageStream << item;
    }
    return *fInstance;
}

//abuse do-while for multiline message macros

//FATAL/////////////////////////////////////////////////////////////////////////

#ifndef HOPS_EXTRA_VERBOSE_MSG

    #define msg_fatal(xKEY, xCONTENT)                                                                                          \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eFatal, xKEY) << xCONTENT;                                                  \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //ERROR/////////////////////////////////////////////////////////////////////////
    #define msg_error(xKEY, xCONTENT)                                                                                          \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eError, xKEY) << xCONTENT;                                                  \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //WARNING///////////////////////////////////////////////////////////////////////
    #define msg_warn(xKEY, xCONTENT)                                                                                           \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eWarning, xKEY) << xCONTENT;                                                \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //STATUS////////////////////////////////////////////////////////////////////////
    #define msg_status(xKEY, xCONTENT)                                                                                         \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eStatus, xKEY) << xCONTENT;                                                 \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //INFO//////////////////////////////////////////////////////////////////////////
    #define msg_info(xKEY, xCONTENT)                                                                                           \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eInfo, xKEY) << xCONTENT;                                                   \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //allow debug messages when debug flag is active
    #ifdef HOPS_ENABLE_DEBUG_MSG //this is defined as a compiler flag via build system

        //DEBUG/////////////////////////////////////////////////////////////////////////
        #define msg_debug(xKEY, xCONTENT)                                                                                      \
            do                                                                                                                 \
            {                                                                                                                  \
                MHO_Message::GetInstance().Lock();                                                                             \
                MHO_Message::GetInstance().SendMessage(eDebug, xKEY) << xCONTENT;                                              \
                MHO_Message::GetInstance().Unlock();                                                                           \
            }                                                                                                                  \
            while(0)
    #else
        //debug is not enabled, so we remove them from compilation
        #define msg_debug(xKEY, xCONTENT)
    #endif

#else //HOPS_EXTRA_VERBOSE_MSG is defined

    #define msg_fatal(xKEY, xCONTENT)                                                                                          \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eFatal, xKEY)                                                               \
                << "(" << sn::file_basename(__FILE__) << ":" << __LINE__ << ") " << xCONTENT;                                  \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //ERROR/////////////////////////////////////////////////////////////////////////
    #define msg_error(xKEY, xCONTENT)                                                                                          \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eError, xKEY)                                                               \
                << "(" << sn::file_basename(__FILE__) << ":" << __LINE__ << ") " << xCONTENT;                                  \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //WARNING///////////////////////////////////////////////////////////////////////
    #define msg_warn(xKEY, xCONTENT)                                                                                           \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eWarning, xKEY)                                                             \
                << "(" << sn::file_basename(__FILE__) << ":" << __LINE__ << ") " << xCONTENT;                                  \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //STATUS////////////////////////////////////////////////////////////////////////
    #define msg_status(xKEY, xCONTENT)                                                                                         \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eStatus, xKEY)                                                              \
                << "(" << sn::file_basename(__FILE__) << ":" << __LINE__ << ") " << xCONTENT;                                  \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //INFO//////////////////////////////////////////////////////////////////////////
    #define msg_info(xKEY, xCONTENT)                                                                                           \
        do                                                                                                                     \
        {                                                                                                                      \
            MHO_Message::GetInstance().Lock();                                                                                 \
            MHO_Message::GetInstance().SendMessage(eInfo, xKEY)                                                                \
                << "(" << sn::file_basename(__FILE__) << ":" << __LINE__ << ") " << xCONTENT;                                  \
            MHO_Message::GetInstance().Unlock();                                                                               \
        }                                                                                                                      \
        while(0)

    //allow debug messages when debug flag is active
    #ifdef HOPS_ENABLE_DEBUG_MSG //this is defined as a compiler flag via build system

        //DEBUG/////////////////////////////////////////////////////////////////////////
        #define msg_debug(xKEY, xCONTENT)                                                                                      \
            do                                                                                                                 \
            {                                                                                                                  \
                MHO_Message::GetInstance().Lock();                                                                             \
                MHO_Message::GetInstance().SendMessage(eDebug, xKEY)                                                           \
                    << "(" << sn::file_basename(__FILE__) << ":" << __LINE__ << ") " << xCONTENT;                              \
                MHO_Message::GetInstance().Unlock();                                                                           \
            }                                                                                                                  \
            while(0)
    #else
        //debug is not enabled, so we remove them from compilation
        #define msg_debug(xKEY, xCONTENT)
    #endif

#endif

#ifdef HOPS_ENABLE_STEPWISE_CHECK //this is defined as a compiler flag via build system
    //error check is enabled, so that we can verify a boolean return value is true
    #define check_step_error(xVALUE, xKEY, xCONTENT)                                                                           \
        if(!xVALUE)                                                                                                            \
        {                                                                                                                      \
            MHO_Message::GetInstance().SendMessage(eError, xKEY) << xCONTENT;                                                  \
        }
    #define check_step_fatal(xVALUE, xKEY, xCONTENT)                                                                           \
        if(!xVALUE)                                                                                                            \
        {                                                                                                                      \
            MHO_Message::GetInstance().SendMessage(eFatal, xKEY) << xCONTENT;                                                  \
            HOPS_ASSERT_THROW(xVALUE);                                                                                         \
        }
#else
    //error check is competely disabled
    #define check_step_error(xVALUE, xKEY, xCONTENT)
    #define check_step_fatal(xVALUE, xKEY, xCONTENT)
#endif

} // namespace hops

#endif /*! end of include guard: MHO_Message */
