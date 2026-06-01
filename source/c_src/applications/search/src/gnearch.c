/*
 * Support for replacing the PGPLOT graphics with gnuplot.
 * For the present, this is considered a second output path.
 *
 * After ever fit_peaks() we make a plot.
 * When we are done, we montage.
 */
#include "search.h"
#if BIGGER

/* return: a parsable file (1 or 0)? or a template required (-1)? */
int gnuconfile(int scancnt, char *gfile, gpconf *gpfp)
{
    if (scancnt > 4) {
        msg("-g arg '%%dx%%d:%%d:%%d' scanned %d items", 3, scancnt);
        return(1);
    } else if (scancnt == 0) {

    } /* else */
    msg("plots configured with %dx%d:%d", 2,
        gpfp->ncols, gpfp->nrows, gpfp->asqr);
    return(0);
}

/* parse the -g argument for CxR or a filename */
int gargparse(char *garg, gpconf *gpfp)
{
    int ii = sscanf(garg, "%dx%d:%d:%d",
        &gpfp->ncols, &gpfp->nrows, &gpfp->asqr, &gpfp->plot);
    /* in case the PGPLOT is specified in the environment */
    if (ii > 0 && ii < 5) {
        if (getenv("PGPLOT_DEV")) gpfp->plot = TRUE;
    } else {
        /* handler for the other cases */
        return(gnuconfile(ii, garg, gpfp));
    }
    if (gpfp->plot) msg("PGPLOT search plots will be made", 2);
    if (ii == 4) msg("Search gnuplots will be made with defaults", 2);
    return(0);
}

/* make the plot */
void gnusrchplot(int nout, srchsum *srchp, gpconf *gpfp)
{
}

/* make a montage and cleanup */
void gnufinish(gpconf *gpfp)
{


    free(gpfp->gplatt);
    free(gpfp->devp);
}

#endif /* BIGGER */
/*
 * eof vim:nospell
 */
