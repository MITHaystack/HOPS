/*
 * Support for gnuplot splots
 *  general configuration options are in gspp->gpcopy
 *  details for this plot are in gspp
 * splot 'search-0.data'
 */
#if BIGGER
#include "search.h"
#include "splotemp.h"

/* items currently hardwired */
void set_gsplot_defaults(gsplot *gspp)
{
    gspp->isosamples = 100;
    gspp->dgrid3dalgorithm = "qnorm";
    gspp->dgrid3dalgorder = 2;
    gspp->bsplineorder = 6;
    /* FIXME: need to adjust based on min/max */
    gspp->cntr_lowest = 5.0;
    gspp->cntr_increment = 10.0;
    gspp->min_snr = 3.0;
    gspp->bgfieldname = "gray20";
    gspp->colorbar_label = "SNR: FIXME contour levels";
    gspp->palette_rgbformulae = "23,28,3";
    /* FIXME */
    gspp->rate_unit = "ps/s";
    gspp->delay_unit = "ns";    /* us for sf 1.0 */
    gspp->rate_sf = 1.0;
    gspp->delay_sf = 1e3;
}

void make_gnucmds(gsplot *gspp)
{
    FILE *gfp;

    /* set hardwired parameters */
    set_gsplot_defaults(gspp);

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
        7.0, gspp->ratesize, gspp->delaysize, "Sans", 14,
        gspp->label, gspp->srcsnr, "SansBold", (int)round(14 * 1.15));
    fprintf(gfp, GNUPLOT_CONFIG,
        gspp->isosamples, gspp->numrates, gspp->numdelays,
        gspp->dgrid3dalgorithm, gspp->dgrid3dalgorder,
        gspp->bsplineorder, gspp->cntr_lowest, gspp->cntr_increment,
        gspp->rate_sf,
        gspp->min_rate, gspp->peak_rate, gspp->max_rate, gspp->numrates-1,
        gspp->delay_sf,
        gspp->min_delay, gspp->peak_delay, gspp->max_delay, gspp->numdelays-1,
        gspp->numrates-1, gspp->peak_rate*gspp->rate_sf, gspp->rate_unit,
        gspp->numdelays-1, gspp->peak_delay*gspp->delay_sf, gspp->delay_unit,
        gspp->min_snr, gspp->snr,
        gspp->colorbar_label, gspp->palette_rgbformulae);
    fprintf(gfp, GNUPLOT_SPLOT,
        gspp->gnpdf, gspp->bgfieldname, gspp->pfile);
    fclose(gfp);
}
#endif /* BIGGER */
/*
 * eof vim:nospell
 */
