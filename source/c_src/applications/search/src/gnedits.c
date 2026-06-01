/*
 * This handles the configuration file in support of the
 * plethora of gnuplot options.  It also carries the 4
 * options from the original PGPLOT implementation:
 *  ncols, nrows, asqr, plot.
 *
 * The general logic follows exam_edits.c of cohfit
 */
#include "search.h"


/* entry from gnuconfile(), returning
 *   -1 if a template was generated
 *    0 if a file was found and correctly parsed
 *    1 if there was a parsing error
 */
int gnuedits(char *garg, gpconf *gpfp)
{

}

/*
 * eof vim:nospell
 */
