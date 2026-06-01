/*
 * This handles the configuration file in support of the
 * plethora of gnuplot options.  It also carries the 4
 * options from the original PGPLOT implementation:
 *  ncols, nrows, asqr, plot.
 *
 * The general logic follows exam_edits.c of cohfit
 * with some pruning.
 */
#include "search.h"
#include <sys/stat.h>

/* see if the file exists; return 1 if found else the errno */
int gnuexists(char *gfile, int *serp)
{
    struct stat sb;
    if ((stat(gfile, &sb) < 0) && ((*serp = errno) == ENOENT)) return(0);
    if (*serp) {                    /* this cannot not end very well */
        msg("Some error to fix with file %s:", 3, gfile);
        msg("  %s", 3, strerror(*serp));
        return(-1);
    }
    return(1);                      /* file exists and is e.g. readable */
}

/* create the template config file returning -1 if ok >0 otherwise */
int create_gnuconf(char *gfile, gpconf *gpfp)
{
    return(1);                      /* not created */
}

/* parse the config file returning 0 if ok, >0 otherwise */
int gnuparse(char *gfile, gpconf *gpfp)
{
    return(1);                      /* not parsed */
}


/* entry from gnuconfile(), returning
 *   -1 if a template was generated
 *    0 if a file was found and correctly parsed
 *    1 if there was a parsing error
 */
int gnuedits(char *garg, gpconf *gpfp)
{
    int staterr = 0;
    if (1 == gnuexists(garg, &staterr)) {
        msg("Found gnuplot config file '%s'", 2, garg);
        return(gnuparse(garg, gpfp));
    } else {
        if (ENOENT != staterr) return(staterr);
    }
    msg("Creating gnuplot config file '%s'", 2, garg);
    return(create_gnuconf(garg, gpfp));
}

/*
 * eof vim:nospell
 */
