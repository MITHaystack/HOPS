#include "HkMessenger.hh"

namespace hops
{



//initialization to nullptr
HkMessenger* HkMessenger::fInstance = nullptr;

void
HkMessenger::AddKey(const std::string& key)
{
    fKeys.insert(key);
}

void
HkMessenger::AddKey(const char* key)
{
    fKeys.insert(std::string(key));
}

void
HkMessenger::RemoveKey(const std::string& key)
{
    auto iter = fKeys.find(key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void
HkMessenger::RemoveKey(const char* key)
{
    std::string tmp_key(key);
    auto iter = fKeys.find(tmp_key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void
HkMessenger::RemoveAllKeys()
{
    fKeys.clear();
}

void
HkMessenger::Flush()
{
    if( PassMessage() )
    {
        *fTerminalStream << fMessageStream.str();
    }
    fMessageStream.clear();
    fMessageStream.str(std::string());
}

HkMessenger&
HkMessenger::SendMessage(const HkMessageLevel& level, const std::string& key)
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

HkMessenger&
HkMessenger::SendMessage(const HkMessageLevel& level, const char* key)
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

HkMessenger&
HkMessenger::operator<<(const HkMessageNewline&)
{
    fMessageStream << '\n';
    return *fInstance;
}

HkMessenger&
HkMessenger::operator<<(const HkMessageEndline&)
{
    fMessageStream << std::endl;
    Flush();
    return *fInstance;
}


bool
HkMessenger::PassMessage()
{
    return ( (fCurrentLevel <= fAllowedLevel && ( fCurrentKeyIsAllowed || fAcceptAllKeys ) )
            || fCurrentLevel == eFatal );
}

std::string
HkMessenger::GetCurrentPrefix(const HkMessageLevel& level, const std::string& key)
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
