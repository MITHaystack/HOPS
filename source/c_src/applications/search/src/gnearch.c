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
        if (gpfp->gnupdfs) {
            for (ii = 0; ii < gpfp->npdfs; ii++)
                if (gpfp->gnupdfs[ii]) {
                    /* gpfp->gnupdfs[ii] points to .pdf */
                    void *pfile, *gfile, *gnpdf;
                    pfile = gpfp->gnupdfs[ii] - 2*gpfp->patlen;
                    gfile = gpfp->gnupdfs[ii] - 1*gpfp->patlen;
                    gnpdf = gpfp->gnupdfs[ii];
                    msg("Free '%s' (%p) '%s' (%p) '%s' (%p)", 1,
                        (char *)pfile, pfile, (char *)gfile, gfile,
                        (char *)gnpdf, gnpdf);
                    /* pfile was allocated */
                    free(pfile);
                }
            free(gpfp->gnupdfs);
        }
        if (gpfp->gcfile) {
            msg("Freeing gnuplot config file", 1);
            free(gpfp->gcfile);
        }
        if (gpfp->gplatt) {
            msg("Freeing gnuplot file pattern", 1);
            free(gpfp->gplatt);
        }
    }
    msg("Freeing PGPLOT device", 1);
    if (gpfp->devp) free(gpfp->devp);
}

#endif /* BIGGER */
/*
 * eof vim:nospell
 */
