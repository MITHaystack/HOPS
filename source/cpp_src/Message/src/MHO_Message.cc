#include "MHO_Message.hh"

namespace hops
{


//initialization to nullptr
MHO_Message* MHO_Message::fInstance = nullptr;

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
    fMessageStream << std::endl;
    Flush();
    return *fInstance;
}


bool
MHO_Message::PassMessage()
{
    return ( (fCurrentLevel <= fAllowedLevel && ( fCurrentKeyIsAllowed || fAcceptAllKeys ) )
            || fCurrentLevel == eFatal );
}

std::string
MHO_Message::GetCurrentPrefix(const MHO_MessageLevel& level, const std::string& key)
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
