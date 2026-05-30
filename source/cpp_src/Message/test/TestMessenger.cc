#include "MHO_Message.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{

    //add a list of acceptable message categories/keys
    //this should be done once on a per-application setting basis
    MHO_Message::GetInstance().AddKey("math");
    MHO_Message::GetInstance().AddKey("core");
    //set the message level to the lowest possible
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    //send some messages
    double pi_value = 3.14159265;
    int a = 2;
    int n = 5;
    msg_info("math", "I have a message for you: pi = " << pi_value << eom);
    msg_status("core", "I have message for you: n = " << n << eom);

    //dummy is not in the set of keys, so this shouldn't make it to the terminal
    msg_status("dummy", "I have message for you: "
                            << "you better not see this!" << eom);

    MHO_Message::GetInstance().AddKey("dummy");

    MHO_Message::GetInstance().SetMessageLevel(eInfo);
    msg_error("dummy", "I have message for you: "
                           << "you should see this!" << eom);

    MHO_Message::GetInstance().RemoveKey("dummy");

    //pass an error message
    msg_error("math", "I have found: " << a << " + " << a << " = " << n << eom);
    //now change the message level to the most strict,
    //the following message shouldn't make it through, even though it is
    //in the 'math' category
    MHO_Message::GetInstance().SetMessageLevel(eFatal);
    msg_error("math", "I have found: " << a << " + " << a << " = " << n << eom);

    //reset the message level
    MHO_Message::GetInstance().SetMessageLevel(eDebug);
    //now set things up so any message with any key can be passed
    //(even if it is not is the list of acceptable keys)
    MHO_Message::GetInstance().AcceptAllKeys();
    msg_status("dummy", "I have message for you: "
                            << "its OK if you see this!" << eom);

    //try to pass a debug message
    msg_debug("core", "If you have enabled the option ENABLE_DEBUG_MSG you will see me." << eom);

#ifndef HOPS_ENABLE_DEBUG_MSG
    msg_info("core", "ENABLE_DEBUG_MSG is disabled, debug messages will be hidden." << eom);
#endif

    MHO_Message::GetInstance().SetLegacyMessageLevel(-1);
    MHO_Message::GetInstance().SetLegacyMessageLevel(0);
    MHO_Message::GetInstance().SetLegacyMessageLevel(1);
    msg_warn("dummy", "This is a warning." << eom);
    MHO_Message::GetInstance().SetLegacyMessageLevel(2);
    MHO_Message::GetInstance().SetLegacyMessageLevel(3);
    msg_fatal("dummy", "This is a fatal error." << eom);
    MHO_Message::GetInstance().SetLegacyMessageLevel(4);
    MHO_Message::GetInstance().SetLegacyMessageLevel(5);
    msg_fatal("dummy", "This is not visible." << eom);

    // Cover the remaining legacy-level branches: '-2' (debug) and the default
    // catch-all.  These have no side effects beyond changing the cached level.
    MHO_Message::GetInstance().SetLegacyMessageLevel(-2);
    MHO_Message::GetInstance().SetLegacyMessageLevel(42);

    // Exercise the SendMessage(level, std::string const&) overload directly --
    // the msg_* macros always pass a string literal, which binds to the
    // const char* overload, leaving the std::string overload uncovered.
    MHO_Message::GetInstance().SetMessageLevel(eDebug);
    MHO_Message::GetInstance().AddKey("math");
    {
        std::string skey("math");
        MHO_Message::GetInstance().SendMessage(eInfo, skey) << "via std::string overload" << eom;
    }

    // Cover operator<<(MHO_MessageNewline) (the `eol` token) and, via the
    // resulting fWasLastLineNewLine == true, the PADDING branch in
    // GetCurrentPrefix.
    msg_info("math", "line one" << eol << "line two" << eom);

    MHO_Message::GetInstance().RemoveAllKeys();

    return 0;
}
