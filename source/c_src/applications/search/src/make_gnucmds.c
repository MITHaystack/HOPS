/*
 * Support for gnuplot splots
 *  general configuration options are in gspp->gpcopy
 *  details for this plot are in gspp
 * splot 'search-0.data'
 */
#if BIGGER
#include "search.h"
#include "splotemp.h"

/* there are too many options for the contours -- these are data independent */
void set_basic_contour_options(gsplot *gspp)
{
    gspp->isosamples = 100;
    gspp->dgrid3dalgorithm = "qnorm";
    gspp->dgrid3dalgorder = 2;
    gspp->bsplineorder = 6;
}

/* For palette type and options that we have tested, set a bg color
 * otherwise at least make sure 3 strings are passed.  We tested:
 *   gspp->palette_type = "cubehelix";
 *   gspp->palette_options = "start 0.5 cycles 0.5 saturation 1";
 *   gspp->palette_type = "viridis";
 *   gspp->palette_options = " ";
 *   gspp->palette_type = "rgbformulae";
 *   gspp->palette_options = "7,5,15";
 *   gspp->palette_type = "rgbformulae";
 *   gspp->palette_options = "3,11,6";
 *   gspp->palette_type = "rgbformulae";
 *   gspp->palette_options = "21,22,23";
 *   gspp->palette_type = "rgbformulae";
 *   gspp->palette_options = "33,13,10";
 *   gspp->palette_type = "rgbformulae";
 *   gspp->palette_options = "30,31,32";
 *   gspp->palette_type = "rgbformulae";
 *   gspp->palette_options = "34,35,36";
 *   gspp->palette_type = "rgbformulae";
 *   gspp->palette_options = "31,-11,32";
 *   gspp->palette_type = "gray";
 *   gspp->palette_options = " ";
 */
/* We've reduced it to a set of formulas and a background object color;
 * this macro conditions the choice on the formula trio */
#define PALETTE(TYPE,OPTS,BGCLR) do {\
    if (gspp->palette_type && \
        !strcmp(gspp->palette_type, TYPE) &&\
        !strcmp(gspp->palette_options,OPTS)) {\
        gspp->bgfieldname = BGCLR; return; } } while(0);
void set_colormap_palette(gsplot *gspp)
{
    /* these two have different options */
    PALETTE("cubehelix", "start 0.5 cycles 0.5 saturation 1", "gray20");
    PALETTE("viridis", " ", "dark-violet");
    PALETTE("gray", " ", "black");
    /* ocean (green-blue-white) */
    PALETTE("rgbformulae", "23,28,3", "dark-spring-green");
    /* gnuplot default (black-blue-red-yellow) */
    PALETTE("rgbformulae", "7,5,15", "gray20"); /* gnuplot default */
    /* green-red-violet */
    PALETTE("rgbformulae", "3,11,6", "dark-spring-green");
    /* hot (black-red-yellow-white) */
    PALETTE("rgbformulae", "21,22,23", "gray20");
    /* black-blue-violet-yellow-white */
    PALETTE("rgbformulae", "30,31,32", "navy");
    /* rainbow (blue-green-yellow-red) */
    PALETTE("rgbformulae", "33,13,10", "navy");
    /* AFM hot (black-red-yellow-white) */
    PALETTE("rgbformulae", "34,35,36", "gray20");
    /* demonstrates inverting color */
    PALETTE("rgbformulae", "31,-11,32", "dark-spring-green");
    /* force something that works */
    if (!gspp->palette_type) {
        gspp->palette_type = "rgbformulae";
        gspp->palette_options = "23,28,3";
        gspp->bgfieldname = "dark-spring-green";
    }
}

/* set the delay and rate unit/scale factors */
void set_rate_unit_sf(char *unit, gsplot *gspp)
{
    if (!strcmp(unit, "ps/s")) {
        gspp->rate_unit = "ps/s";   /* search code supplies this */
        gspp->rate_sf = 1.0;
    } /* else other options as desired */
}
void set_delay_unit_sf(char *unit, gsplot *gspp)
{
    if (!strcmp(unit, "us")) {
        gspp->delay_unit = "us";    /* search code supplies this */
        gspp->delay_sf = 1.0;
    } else if (!strcmp(unit, "ns")) {
        gspp->delay_unit = "ns";
        gspp->delay_sf = 1e3;
    } /* else other options as desired */
}

/* items currently hardwired but in principal adjustable */
void set_gsplot_defaults(gsplot *gspp)
{
    set_colormap_palette(gspp);
    set_basic_contour_options(gspp);
    set_rate_unit_sf("ps/s", gspp);
    set_delay_unit_sf("ns", gspp);
}

/*
 * These options depend on the peak SNR value found
 * generally don't want too many contours or it gets confusing
 */
void set_contour_options(gsplot *gspp)
{
    static char cblabel[MAX_TXT];
    if (gspp->snr < 10.0) { /* below 10 and you are desparate */
        gspp->cntr_lowest = 3.0;
        gspp->cntr_increment = 1.0;
    } else if (gspp->snr < 50.0) {
        gspp->cntr_lowest = 5.0;
        gspp->cntr_increment = 10.0;
    } else if (gspp->snr < 100) {
        gspp->cntr_lowest = 10.0;
        gspp->cntr_increment = 20.0;
    } else if (gspp->snr < 250) {
        gspp->cntr_lowest = 25.0;
        gspp->cntr_increment = 50.0;
    } else if (gspp->snr < 500) {
        gspp->cntr_lowest = 50.0;
        gspp->cntr_increment = 100.0;
    } else if (gspp->snr < 1000) {
        gspp->cntr_lowest = 100.0;
        gspp->cntr_increment = 200.0;
    } else if (gspp->snr < 2500) {
        gspp->cntr_lowest = 250.0;
        gspp->cntr_increment = 500.0;
    } else {            /* above 2500 and you are just messing around */
        gspp->cntr_lowest = 500.0;
        gspp->cntr_increment = 1000.0;
    }
    snprintf(cblabel, MAX_TXT, "SNR: contours from %.1lf by %.1lf steps",
        gspp->cntr_lowest, gspp->cntr_increment);
    gspp->colorbar_label = cblabel;
}

/* construct the .gnu file from template and options */
void make_gnucmds(gsplot *gspp)
{
    FILE *gfp;

    /* these are independent of the data input */
    set_gsplot_defaults(gspp);
    /* these options depends on peak SNR found */
    set_contour_options(gspp);

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
        /* setup options for data and contouring */
        gspp->isosamples, gspp->numrates, gspp->numdelays,
        gspp->dgrid3dalgorithm, gspp->dgrid3dalgorder,
        gspp->bsplineorder, gspp->cntr_lowest, gspp->cntr_increment,
        /* linear scaling function( indices to rate and delay values */
        gspp->rate_sf, gspp->min_rate,
        gspp->peak_rate, gspp->max_rate, gspp->numrates-1,
        gspp->delay_sf, gspp->min_delay,
        gspp->peak_delay, gspp->max_delay, gspp->numdelays-1,
        /* ranges and labels */
        gspp->numrates-1, gspp->rate_unit,
        gspp->peak_rate*gspp->rate_sf, gspp->rate_unit,
        gspp->numdelays-1, gspp->delay_unit,
        gspp->peak_delay*gspp->delay_sf, gspp->delay_unit,
        /* zrange and cb label and formula */
        (gspp->cntr_lowest)*0.6, gspp->snr,
        gspp->colorbar_label, gspp->palette_type, gspp->palette_options);
    fprintf(gfp, GNUPLOT_SPLOT,
        gspp->gnpdf, gspp->bgfieldname, gspp->pfile);
    fclose(gfp);
}
#endif /* BIGGER */
/*
 * eof vim:nospell
 */
