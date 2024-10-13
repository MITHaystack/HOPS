#include "MHO_Message.hh"

#define MSG_ALIGN_PAD 32
constexpr const char* PADDING = "                                ";

namespace hops
{

//static initialization to nullptr
MHO_Message* MHO_Message::fInstance = nullptr;

std::string MHO_Message::fRed = "\33[31;1m";      //fatal
std::string MHO_Message::fYellow = "\33[93;1m";   //error
std::string MHO_Message::fOrange = "\33[33;1m";   //warning
std::string MHO_Message::fBlue = "\33[34;1m";     //status
std::string MHO_Message::fGreen = "\33[32;1m";    //info
std::string MHO_Message::fCyan = "\33[36;1m";     //debug
std::string MHO_Message::fWhite = "\33[37;1m";    //default
std::string MHO_Message::fColorSuffix = "\33[0m"; //color close

void MHO_Message::AddKey(const std::string& key)
{
    fKeys.insert(key);
}

void MHO_Message::AddKey(const char* key)
{
    fKeys.insert(std::string(key));
}

void MHO_Message::RemoveKey(const std::string& key)
{
    auto iter = fKeys.find(key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void MHO_Message::RemoveKey(const char* key)
{
    std::string tmp_key(key);
    auto iter = fKeys.find(tmp_key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void MHO_Message::RemoveAllKeys()
{
    fKeys.clear();
}

void MHO_Message::Flush()
{
    fWasLastLineNewLine = false;
    if(PassMessage())
    {
        *fTerminalStream << fMessageStream.str();
    }
    fMessageStream.clear();
    fMessageStream.str(std::string());
}

MHO_Message& MHO_Message::SendMessage(const MHO_MessageLevel& level, const std::string& key)
{
    fCurrentLevel = level;
    fCurrentKeyIsAllowed = false;
    auto iter = fKeys.find(key);
    if(iter != fKeys.end())
    {
        fCurrentKeyIsAllowed = true;
    }

    if(PassMessage())
    {
        fMessageStream << GetCurrentPrefix(level, key);
    }

    return *fInstance;
}

MHO_Message& MHO_Message::SendMessage(const MHO_MessageLevel& level, const char* key)
{
    fCurrentLevel = level;
    fCurrentKeyIsAllowed = false;
    std::string tmp_key(key);
    auto iter = fKeys.find(tmp_key);
    if(iter != fKeys.end())
    {
        fCurrentKeyIsAllowed = true;
    }

    if(PassMessage())
    {
        fMessageStream << GetCurrentPrefix(level, tmp_key);
    }

    return *fInstance;
}

MHO_Message& MHO_Message::operator<<(const MHO_MessageNewline&)
{
    fMessageStream << '\n';
    fWasLastLineNewLine = true;
    return *fInstance;
}

MHO_Message& MHO_Message::operator<<(const MHO_MessageEndline&)
{
#ifdef HOPS_COLOR_MSG
    fMessageStream << fColorSuffix << std::endl;
#else
    fMessageStream << std::endl;
#endif
    fWasLastLineNewLine = false;
    Flush();
    return *fInstance;
}

bool MHO_Message::PassMessage()
{
    return ((fCurrentLevel <= fAllowedLevel && (fCurrentKeyIsAllowed || fAcceptAllKeys)));
}

std::string MHO_Message::GetCurrentPrefix(const MHO_MessageLevel& level, const std::string& key)
{
    std::size_t color_size;
    std::stringstream ss;
    if(fWasLastLineNewLine)
    {
        // std::string pad;
        // pad.resize(MSG_ALIGN_PAD,' ');
        // ss << pad;
        ss << PADDING;
        return ss.str();
    }

#ifdef HOPS_COLOR_MSG

    switch(level)
    {
        case eFatal:
            color_size = fRed.size();
            ss << fRed << "FATAL[" << key << "] ";
            break;
        case eError:
            color_size = fYellow.size();
            ss << fYellow << "ERROR[" << key << "] ";
            break;
        case eWarning:
            color_size = fOrange.size();
            ss << fOrange << "WARNING[" << key << "] ";
            break;
        case eStatus:
            color_size = fBlue.size();
            ss << fBlue << "STATUS[" << key << "] ";
            break;
        case eInfo:
            color_size = fGreen.size();
            ss << fGreen << "INFO[" << key << "] ";
            break;
        case eDebug:
            color_size = fCyan.size();
            ss << fCyan << "DEBUG[" << key << "] ";
            break;
    }

#else  // HOPS_COLOR_MSG is disabled

    switch(level)
    {
        case eFatal:
            ss << "FATAL[" << key << "] ";
            break;
        case eError:
            ss << "ERROR[" << key << "] ";
            break;
        case eWarning:
            ss << "WARNING[" << key << "] ";
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

    //pad out for alignment
    std::string msg_label = ss.str();
    std::size_t s = msg_label.size() - color_size;
    if(s < MSG_ALIGN_PAD)
    {
        msg_label.insert(msg_label.end(), MSG_ALIGN_PAD - s, ' ');
    }

    return msg_label;
}

void MHO_Message::SetLegacyMessageLevel(int legacy_message_level)
{
    //set the message level according to the fourfit (legacy) style
    //where 3 is least verbose, and '-1' is most verbose
    switch(legacy_message_level)
    {
        case -2:
//NOTE: debug messages must be compiled-in
#ifndef HOPS_ENABLE_DEBUG_MSG
            SetMessageLevel(eInfo);
            msg_warn("fringe",
                     "debug messages are toggled via compiler flag, re-compile with ENABLE_DEBUG_MSG=ON to enable." << eom);
#else
            SetMessageLevel(eDebug);
#endif
            break;
        case -1:
            SetMessageLevel(eInfo);
            break;
        case 0:
            SetMessageLevel(eStatus);
            break;
        case 1:
            SetMessageLevel(eWarning);
            break;
        case 2:
            SetMessageLevel(eError);
            break;
        case 3:
            SetMessageLevel(eFatal);
            break;
        case 4:
            //silent, but prints out the mk4 fringe file name to stderr if it is created
            //this is for backwards compatiblity with VGOS post-processing scripts
            SetMessageLevel(eSpecial);
            break;
        case 5:
            //completely silent
            SetMessageLevel(eSilent);
            break;
        default:
            //for now default is most verbose, eventually will change this to silent
            SetMessageLevel(eDebug);
    }
}

} // namespace hops
