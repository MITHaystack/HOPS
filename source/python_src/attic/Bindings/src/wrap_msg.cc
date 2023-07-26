//
// Wrapper for hops msg() function to use Messenger
// (c) Massachusetts Institute of Technology, 2020
// The contents of the package Copyright statement apply here.
//
// this is for initial testing...
//
extern "C" {
#include "wrap_msg.h"
// a default prefix on the message with context
static char wrap_progname[200] = "hops";
} // extern "C"

#include "MHO_Message.hh"
using namespace hops;

extern "C" void wrap_message(int verbosity, int severity, char *message)
{
    MHO_Message::GetInstance().AddKey(wrap_progname);
    MHO_Message::GetInstance().SetMessageLevel(eDebug);
    switch(MAX_VERBOSITY - severity) {
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

#include <assert.h>
#include <stdarg.h>

/*
 * This is a reference "importance" level which the user may set.
 * It is reciprocal to the more common verbosity (-v -v -v ).
 *             -m                             msglevel    codelevel
 * The scale is 3 (only see most important)   verbosity 0  severity 6
 *          to -3 (see absolutely everything) verbosity 6  severity 0
 * MIN/MAX SEVERITY were -3 and 3 in the original HOPS.
 */
static int wrap_msglev = 0;

void set_wrap_msglevel(int msglev)
{
    /* asserts to catch inadvertent changes */
    assert(MAX_SEVERITY ==  3);
    assert(MIN_SEVERITY == -3);
    if (msglev > MAX_SEVERITY) msglev = MAX_SEVERITY;
    if (msglev < MIN_SEVERITY) msglev = MIN_SEVERITY;
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

/* replacement for sub/mk4util::msg() */
void wrap_msg (char *fmt, int level, ...)
{
    char buffer[256];
    int used = 0;
    va_list args;

    /* discard uninteresting messages */
    if (level < wrap_msglev) return;

    /* man stdarg(3) for understanding */
    used = snprintf(buffer, sizeof(buffer), "%s: ", wrap_progname);
    va_start(args, level);
    (void)vsnprintf(buffer+used, sizeof(buffer)-used, fmt, args);
    va_end(args);

    /* hand-off to the MHO_Message class shifting to verbosity/severity */
    wrap_message(MAX_SEVERITY-wrap_msglev, level - MIN_SEVERITY, buffer);
}

} // extern "C"

//
// eof
//
