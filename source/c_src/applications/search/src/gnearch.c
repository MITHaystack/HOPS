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
        return(gnuedits(gfile, gpfp));
    } /* else */
    msg("plots configured with %dx%d:%d", 2,
        gpfp->ncols, gpfp->nrows, gpfp->asqr);
    return(0);
}

/* parse the -g argument for CxR or a filename */
int gargparse(char *garg, gpconf *gpfp)
{
    int ii = sscanf(garg, "%dx%d:%d:%d:%d",
        &gpfp->ncols, &gpfp->nrows, &gpfp->asqr, &gpfp->pplt, &gpfp->gplt);
    /* in case the PGPLOT is specified in the environment */
    if (ii > 0 && ii < 6) {
        if (getenv("PGPLOT_DEV")) gpfp->pplt = TRUE;
    } else {
        /* handler for the other cases */
        return(gnuconfile(ii, garg, gpfp));
    }
    if (gpfp->pplt) msg("PGPLOT search plots will be made", 2);
    if (gpfp->gplt) {
        if (gpfp->gplatt) msg("Gnuplot search plots will be made", 2);
        if (gpfp->montage) msg("And a montage of all plots will be made", 2);
    }
    return(0);
}

/* make a montage and cleanup of various allocations */
void gnufinish(gpconf *gpfp)
{
    int ii;
    if (gpfp->gplt) {
        if (gpfp->montage && gpfp->density > 0) {
            if (gpfp->npdfs > 0) search_montage(gpfp);
            else msg("Have no pdfs to montage", 3);
        }
        msg("Freeing PDF plot filenames", 3);
        if (gpfp->gnupdfs) {
            for (ii = 0; ii < gpfp->npdfs; ii++)
                if (gpfp->gnupdfs[ii]) {
                    msg("Free %p", 3, gpfp->gnupdfs[ii] - 2*gpfp->patlen);
                    free(gpfp->gnupdfs[ii] - 2*gpfp->patlen);
                }
            free(gpfp->gnupdfs);
        }
        msg("Freeing gnuplot config file", 3);
        if (gpfp->gcfile) free(gpfp->gcfile);
        msg("Freeing gnuplot file pattern", 3);
        if (gpfp->gplatt) free(gpfp->gplatt);
    }
    msg("Freeing PGPLOT device", 3);
    if (gpfp->devp) free(gpfp->devp);
}

#endif /* BIGGER */
/*
 * eof vim:nospell
 */
