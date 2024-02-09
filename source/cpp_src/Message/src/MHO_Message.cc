#include "MHO_Message.hh"

namespace hops
{



//static initialization to nullptr
MHO_Message* MHO_Message::fInstance = nullptr;

std::string MHO_Message::fRed = "\33[31;1m"; //fatal
std::string MHO_Message::fYellow = "\33[93;1m"; //error
std::string MHO_Message::fOrange = "\33[33;1m"; //warning
std::string MHO_Message::fBlue = "\33[34;1m"; //status
std::string MHO_Message::fGreen = "\33[32;1m"; //info
std::string MHO_Message::fCyan = "\33[36;1m"; //debug
std::string MHO_Message::fWhite = "\33[37;1m"; //default
std::string MHO_Message::fColorSuffix = "\33[0m"; //color close

void
MHO_Message::AddKey(const std::string& key)
{
    fKeys.insert(key);
}

void
MHO_Message::AddKey(const char* key)
{
    fKeys.insert(std::string(key));
}

void
MHO_Message::RemoveKey(const std::string& key)
{
    auto iter = fKeys.find(key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}


void
MHO_Message::RemoveKey(const char* key)
{
    std::string tmp_key(key);
    auto iter = fKeys.find(tmp_key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void
MHO_Message::RemoveAllKeys()
{
    fKeys.clear();
}

void
MHO_Message::Flush()
{
    if( PassMessage() )
    {
        *fTerminalStream << fMessageStream.str();
    }
    fMessageStream.clear();
    fMessageStream.str(std::string());
}

MHO_Message&
MHO_Message::SendMessage(const MHO_MessageLevel& level, const std::string& key)
{
    fCurrentLevel = level;
    fCurrentKeyIsAllowed = false;
    auto iter = fKeys.find(key);
    if(iter != fKeys.end()){fCurrentKeyIsAllowed = true;}

    if( PassMessage() )
    {
        fMessageStream << GetCurrentPrefix(level,key);
    }

    return *fInstance;
}

MHO_Message&
MHO_Message::SendMessage(const MHO_MessageLevel& level, const char* key)
{
    fCurrentLevel = level;
    fCurrentKeyIsAllowed = false;
    std::string tmp_key(key);
    auto iter = fKeys.find(tmp_key);
    if(iter != fKeys.end()){fCurrentKeyIsAllowed = true;}

    if( PassMessage() )
    {
        fMessageStream << GetCurrentPrefix(level, tmp_key);
    }

    return *fInstance;
}

MHO_Message&
MHO_Message::operator<<(const MHO_MessageNewline&)
{
    fMessageStream << '\n';
    return *fInstance;
}

MHO_Message&
MHO_Message::operator<<(const MHO_MessageEndline&)
{
    #ifdef HOPS_COLOR_MSG
    fMessageStream << fColorSuffix << std::endl;
    #else 
    fMessageStream << std::endl;
    #endif

    Flush();
    return *fInstance;
}


bool
MHO_Message::PassMessage()
{
    return ( (fCurrentLevel <= fAllowedLevel && ( fCurrentKeyIsAllowed || fAcceptAllKeys ) ) );
}

std::string
MHO_Message::GetCurrentPrefix(const MHO_MessageLevel& level, const std::string& key)
{

    std::stringstream ss;
    #ifdef HOPS_COLOR_MSG

    switch (level)
    {
        case eFatal:
            ss << fRed << "FATAL[" << key << "] ";
            break;
        case eError:
            ss << fYellow << "ERROR[" << key << "] ";
            break;
        case eWarning:
            ss << fOrange << "WARNING["  << key << "] ";
            break;
        case eStatus:
            ss << fBlue << "STATUS[" << key << "] ";
            break;
        case eInfo:
            ss << fGreen << "INFO[" << key << "] ";
            break;
        case eDebug:
            ss << fCyan << "DEBUG[" << key << "] ";
            break;
    }

    #else // HOPS_COLOR_MSG is disabled

    switch (level)
    {
        case eFatal:
            ss << "FATAL[" << key << "] ";
            break;
        case eError:
            ss << "ERROR[" << key << "] ";
            break;
        case eWarning:
            ss << "WARNING["  << key << "] ";
            break;
        case eStatus:
            ss << "STATUS[" << key << "] ";
            break;
        case eInfo:
            ss << "INFO[" << key << "] ";
            break;
        case eDebug:
            ss << "DEBUG[" << key << "] ";
            break;
    }
    #endif //end of HOPS_COLOR_MSG


    return ss.str();
}


}
