/*
 *  Ops include file
 *
 *  $Id: isoclog.h 842 2007-12-10 16:37:49Z gbc $
 */

/* just disable the whole thing */

#ifndef ISOCOPS_H
#define ISOCOPS_H

/* provoke a segfault if used */
#define errlog_enabled	*((int*)0)
#define comlog_enabled	*((int*)0)

/* stub out the library calls */
#define isoclog(X)  (void)0;
#define errlog(X)   (void)0;
#define comlog(X)   (void)0;

#endif /* ISOCOPS_H */

/* original include is below */

#ifndef ISOCOPS_H
#define ISOCOPS_H
/*
 *  Ops error logging routines
 */
extern int errlog_enabled;	/* allows logging control */
extern int comlog_enabled;	/* allows logging control */

void isoclog(char *process);	/* open log for process  */
void errlog(char *message);	/* forserious conditions */
void comlog(char *message);	/* for useful commentary */

/*
 *  Legal values for the IBEX_LOG environment variable
 */
#define IBEX_LOG_TRUE		"true"
#define IBEX_LOG_FALSE		"false"

#endif /* ISOCOPS_H */

/*
 * eof
 */
