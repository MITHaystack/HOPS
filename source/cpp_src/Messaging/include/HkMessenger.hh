#ifndef HkMessenger_HH__
#define HkMessenger_HH__

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

class HkMessageNewline {};
class HKMessageEndline {};

enum
HkMessageLevel : int
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

//uses the singleton pattern (as we only have one terminal)
//TODO make this class thread safe
class HkMessenger
{

    public:
        //provide public access to the only static instance, and pass messages
        static HkMessenger& GetInstance()
        {
            static HkMessenger instance;
            return instance;

        }

        //since this is a singleton we need to remove ability to copy/move
        HkMessenger(HkMessenger const&) = delete;
        HkMessenger(HkMessenger&&) = delete;
        HkMessenger& operator=(HkMessenger const&) = delete;
        HkMessenger& operator=(HkMessenger&&) = delete;

        void AddKey(const std::string& key);
        void RemoveKey(const std::string& key);

        static HkMessenger& SendMessage(const HkMessageLevel& level, const std::string& key);
        static HkMessenger& SendMessage(const HkMessageLevel& level, const char* key);

        template<class XStreamItemType>
        HkMessenger& operator<<(const XStreamItemType& chunk);

        HkMessenger& operator<<(const HkMessengerNewline&);
        HkMessenger& operator<<(const HkMessengerEndline&);

    private:

        //no public access to constructor
        HkSingleton()
        {
            //set up the stream, for now just point to std::cout
            //but we may want to allow this to be configured post-construction
            fTerminalStream = &std::cout;
        };

        virtual ~HkSingleton(){};

        //could also provide a stream for logging
        std::ostream* fTerminalStream;

        //keys of which messages we will accept for output
        std::set< std::string > fKeys

};





#endif /* end of include guard: HkMessenger */
