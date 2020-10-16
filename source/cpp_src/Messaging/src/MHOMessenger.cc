#include "MHOMessenger.hh"

namespace hops
{



//initialization to nullptr
MHOMessenger* MHOMessenger::fInstance = nullptr;

void
MHOMessenger::AddKey(const std::string& key)
{
    fKeys.insert(key);
}

void
MHOMessenger::AddKey(const char* key)
{
    fKeys.insert(std::string(key));
}

void
MHOMessenger::RemoveKey(const std::string& key)
{
    auto iter = fKeys.find(key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void
MHOMessenger::RemoveKey(const char* key)
{
    std::string tmp_key(key);
    auto iter = fKeys.find(tmp_key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void
MHOMessenger::RemoveAllKeys()
{
    fKeys.clear();
}

void
MHOMessenger::Flush()
{
    if( PassMessage() )
    {
        *fTerminalStream << fMessageStream.str();
    }
    fMessageStream.clear();
    fMessageStream.str(std::string());
}

MHOMessenger&
MHOMessenger::SendMessage(const MHOMessageLevel& level, const std::string& key)
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

MHOMessenger&
MHOMessenger::SendMessage(const MHOMessageLevel& level, const char* key)
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

MHOMessenger&
MHOMessenger::operator<<(const MHOMessageNewline&)
{
    fMessageStream << '\n';
    return *fInstance;
}

MHOMessenger&
MHOMessenger::operator<<(const MHOMessageEndline&)
{
    fMessageStream << std::endl;
    Flush();
    return *fInstance;
}


bool
MHOMessenger::PassMessage()
{
    return ( (fCurrentLevel <= fAllowedLevel && ( fCurrentKeyIsAllowed || fAcceptAllKeys ) )
            || fCurrentLevel == eFatal );
}

std::string
MHOMessenger::GetCurrentPrefix(const MHOMessageLevel& level, const std::string& key)
{

    std::stringstream ss;
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
    return ss.str();
}


}
