#include "HkMessenger.hh"

//static initialization to nullptr
static HkMessenger* HkMessenger::fInstance = nullptr;

void
HkMessenger::AddKey(const std::string& key)
{
    fKeys.insert(key);
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
HkMessenger::RemoveAllKeys()
{
    fKeys.clear();
}

void
HkMessenger::Flush()
{
    if(fCurrentLevel <= fAllowedLevel && fCurrentKeyIsAllowed)
    {
        fTerminalStream << fMessageStream.str();
    }
    fMessageStream.clear();
    fMessageStream.str(std::string());
}

static HkMessenger&
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

static HkMessenger&
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

static HkMessenger&
HkMessenger::operator<<(const HkMessengerNewline&)
{
    fMessageStream << '\n';
    return fInstance;
}

static HkMessenger&
HkMessenger::operator<<(const HkMessengerEndline&)
{
    fMessageStream << std::endl;
    Flush();
    return *fInstance;
}
