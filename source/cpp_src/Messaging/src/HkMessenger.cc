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
    if( (fCurrentLevel <= fAllowedLevel && fCurrentKeyIsAllowed) || fCurrentLevel == eFatal )
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
    if(iter != fKeys.end())
    {
        fCurrentKeyIsAllowed = true;
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
    if(iter != fKeys.end())
    {
        fCurrentKeyIsAllowed = true;
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


}
