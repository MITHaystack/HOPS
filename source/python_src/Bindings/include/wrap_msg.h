/*
 * Wrapper for hops msg() function to use Messenger
 * (c) Massachusetts Institute of Technology, 2020
 * The contents of the package Copyright statement apply here.
 *
 * It is not clear anything requires this, but it is useful
 * for some of the module testing, perhaps.  Note that the
 * character strings are not mutable, but inserting 'const'
 * would generate warnings compiling most of the consumers.
 * ie. ... const char *string ... const char *message ...
 *
 * FIXME: wrap the extra functionality (Keys) of MHOMessage.
 * FIXME: provide a more Pythonic interface to message.
 */

#ifndef SWIG_NODEFINE
#define MIN_SEVERITY    (-3)
#define MAX_SEVERITY    ( 3)
#define MIN_VERBOSITY   ( 0)
#define MAX_VERBOSITY   (MAX_SEVERITY - (MIN_SEVERITY))
#endif /* SWIG_NODEFINE */

/* public replacement for msg() call */
extern void wrap_msg(char *string, int level, ...);

/* the actual MHOMessage wrapper function */
extern void wrap_message(int verbosity, int severity, char *message);

/* set/get methods for the importance level */
extern void set_wrap_msglevel(int msglev);
extern int get_wrap_msglevel(void);

/* set/get methods for the program name */
extern void set_wrap_progname(const char *name);
extern char *get_wrap_progname(void);

/*
 * eof
 */
