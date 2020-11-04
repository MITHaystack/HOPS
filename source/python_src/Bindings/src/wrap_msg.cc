//
// Wrapper for hops msg() function to use Messenger
// (c) Massachusetts Institute of Technology, 2020
//
// this is for initial testing...
//
extern "C" {
#include "wrap_msg.h"
// a prefix on the message with context
static char wrap_progname[200] = "hops";
} // extern "C"

#include "MHOMessage.hh"

// is this needed?
using namespace hops;

extern "C" void wrap_message(int verbosity, int severity, char *message)
{
    MHOMessage::GetInstance().AddKey(wrap_progname);
    MHOMessage::GetInstance().SetMessageLevel(eDebug);
    switch(6 - severity) {
    case eFatal:    // 0 fatal errors (most important)
        if (verbosity >= 0) { msg_fatal(wrap_progname, message << eom); }
        break;
    case eError:    // 1 non-fatal but noteworty
        if (verbosity >= 1) { msg_error(wrap_progname, message << eom); }
        break;
    case eWarning:  // 2 unexpected
        if (verbosity >= 2) { msg_warn(wrap_progname, message << eom); }
        break;
    case eStatus:   // 3 status info
        if (verbosity >= 3) { msg_status(wrap_progname, message << eom); }
        break;
    case eInfo:     // 4 extra info
        if (verbosity >= 4) { msg_info(wrap_progname, message << eom); }
        break;
    case eDebug:    // 5 debug level (least important)
        if (verbosity >= 5) { msg_debug(wrap_progname, message << eom); }
        break;
    default:
        if (verbosity >= 6) { msg_debug(wrap_progname, message << eom); }
    }
}

extern "C" {

#include <stdarg.h>

// This is a reference "importance" level which the user may set.
// It is reciprocal to the more common verbosity (-v -v -v ).
//             -m                             msglevel    codelevel
// The scale is 3 (only see most important)   verbosity 0  severity 6
//          to -3 (see absolutely everything) verbosity 6  severity 0
static int wrap_msglev = 0;

void set_wrap_msglevel(int msglev)
{
    if (msglev > 3) msglev = 3;
    if (msglev < -3) msglev = -3;
    wrap_msglev = msglev;
}
int get_wrap_msglevel(void)
{
    return(wrap_msglev);
}
void set_wrap_progname(const char *name)
{
    snprintf(wrap_progname, sizeof(wrap_progname), name);
}
char *get_wrap_progname(void)
{
    return(wrap_progname);
}

// replacement for sub/mk4util::msg()
void wrap_msg (char *fmt, int level, ...)
{
    char buffer[256];
    int used = 0;
    va_list args;

    // discard uninteresting messages
    if (level < wrap_msglev) return;

    // man stdarg(3) for understanding
    used = snprintf(buffer, sizeof(buffer), "%s: ", wrap_progname);
    va_start(args, level);
    (void)vsnprintf(buffer+used, sizeof(buffer)-used, fmt, args);
    va_end(args);

    // hand-off to the MHOMessage class shifting to verbosity/severity
    wrap_message(3-wrap_msglev, level + 3, buffer);
}


} // extern "C"


//
// eof
//
