/*
 * Support for gnuplot splots
 *  general configuration options are in gspp->gpcopy
 *  details for this plot are in gspp
 * splot 'search-0.data'
 */
#if BIGGER
#include "search.h"
#include "splotemp.h"

void make_gnucmds(gsplot *gspp)
{
    FILE *gfp;
    if (!gspp->gfile || !gspp->gnpdf) {
        msg("No gnuplot command file or final PDF provided", 3);
        return;
    }
    if (!(gfp = fopen(gspp->gfile, "w"))) {
        perror("make_gnucmds:fopen");
        msg("Unable to open gnu command file %s", 3, gspp->gfile);
    }
    /* now create the various parts */
    fprintf(gfp, GNUPLOT_PDFILE,
        5.0, 8.0, "Sans", 14, gspp->gnpdf);
    fprintf(gfp, GNUPLOT_CONFIG,
        "NOTHING YET");
    fprintf(gfp, GNUPLOT_SPLOT,
        gspp->pfile);
    fprintf(gfp, GNUPLOT_CODA,
        "NO COMMENT");
    fclose(gfp);
}
#endif /* BIGGER */
/*
 * eof vim:nospell
 */
