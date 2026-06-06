/*
 * Support for gnuplot splots
 *  general configuration options are in gspp->gpcopy
 *  details for this plot are in gspp
 * splot 'search-0.data'
 */
#if BIGGER
#include "search.h"
#include "splotemp.h"

/*
 * These options depend on the peak SNR value found
 * generally don't want too many contours or it gets confusing
 * gspp->cntr_specified is set nonzero if something was specified.
 */
void set_contour_options(gsplot *gspp)
{
    static char cblabel[MAX_TXT];
    if (gspp->cntr_specified > 0) {
        if (gspp->cntr_specified == 3) return;
        if (gspp->cntr_specified < 2)
            msg("You must specify both cntr levels and colorbar_label", 3);
    } else if (gspp->snr < 10.0) {  /* below 10 and you are desparate */
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
    } else {                        /* above 2500 is a joke */
        gspp->cntr_lowest = 500.0;
        gspp->cntr_increment = 1000.0;
    }
    snprintf(cblabel, MAX_TXT, "SNR: contours from %.1lf by %.1lf steps",
        gspp->cntr_lowest, gspp->cntr_increment);
    gspp->colorbar_label = cblabel;
}

/* if these were provided by the previous routine, clear them out */
void nuke_contour_options(gsplot *gspp)
{
    if (gspp->cntr_specified) return;
    gspp->cntr_lowest = gspp->cntr_increment = 0.0;
    gspp->colorbar_label = NULL;
}

/* report on what can be set in the gnuplot config file */
void show_gsplot_contours(FILE *fpg)
{
    fprintf(fpg,
        "#There are three options for the coutours that are automatically\n"
        "# set based on the SNR of the fringe found by search.  You may\n"
        "# override the choices by setting BOTH of these variables:\n"
        "cntr_lowest=<float>\n"
        "cntr_increment=<float>\n"
        "colorbar_label=<string>\n"
        "# Contours are placed at cntr_lowest + N*cntr_increment for\n"
        "# N=0,1,... such that the last contour is below the SNR found.\n"
        "# The label is 'SNR: contours from <...> by <...> steps' but\n"
        "# you can subsitute any text you like.  Note that these choices\n"
        "# will apply to ALL fringes.  So if you don't want that, leave\n"
        "# the default options in place and edit the .gnu command file.\n"
    );
}

/* there are too many options for the contours -- these are data independent */
void set_basic_contour_options(gsplot *gspp)
{
    gspp->isosamples = 100;
    gspp->dgrid3dalgorithm = "qnorm";
    gspp->dgrid3dalgorder = 2;
    gspp->bsplineorder = 6;
}
void show_basic_contour_options(FILE *fpg, gsplot *gspp)
{
    fprintf(fpg,
        "#The following options control the interpretation of the data\n"
        "# in the search output grid and control how contours are drawn.\n"
        "#This option supplies the interpolation of contours in the grid\n"
        "# and the number of samples on the curve.\n"
        "isosamples=%d\n"
        "dgrid3dalgorithm=%s\n"
        "dgrid3dalgorder=%d\n"
        "bsplineorder=%d\n"
        "#\n", gspp->isosamples, gspp->dgrid3dalgorithm,
        gspp->dgrid3dalgorder, gspp->bsplineorder);
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
void show_colormap_palette(FILE *fpg, gsplot *gspp)
{
    fprintf(fpg,
        "#The following options control the colormap used.  Gnuplot uses\n"
        "# a few named colormaps and many specified as rgbformulae (a trio\n"
        "# of integers) that control ranges of red, green and blue values.\n"
        "# Thus in all cases, a 'type' and 'options are needed.  For each\n"
        "# of these a background color is needed for the area of the plot\n"
        "# that falls below the colorbar minimum. Thus you may set these:\n"
        "palette_type=<string>\n"
        "palette_options=<string>\n"
        "bgfieldname=<string>\n"
        "#\n");
    fprintf(fpg,
        "#The following types are recognized and automatically supply a\n"
        "# plausible background color.  For the two without options, you\n"
        "# must supply it as whitespace to prevent parsing errors.\n"
        "# palette_type palette_options                   bgfieldname\n"
        "# -------------------------------------------------------------\n"
        " cubehelix     start 0.5 cycles 0.5 saturation 1 gray20\n"
        " viridis                                         dark-violet\n"
        " gray                                            black\n"
        " rgbformulae   23,28,3                           dark-spring-green\n"
        " rgbformulae   7,5,15                            gray20\n"
        " rgbformulae   3,11,6                            dark-spring-green\n"
        " rgbformulae   21,22,23                          gray20\n"
        " rgbformulae   30,31,32                          navy\n"
        " rgbformulae   33,13,10                          navy\n"
        " rgbformulae   34,35,36                          gray20\n"
        " rgbformulae   31,-11,32                         dark-spring-green\n"
        "#Finally, you can use two strings for type and option that will\n"
        "# be acceptable to the 'set palette ... ... ' command\n"
        "#The default map is:\n"
        "palette_type=%s\n"
        "palette_options=%s\n"
        "bgfieldname=%s\n"
        "#\n", gspp->palette_type, gspp->palette_options, gspp->bgfieldname);
}

/* set values for a number of pdfcairo options */
void set_pdfcairo_options(gsplot *gspp)
{
    gspp->pdfsizeinch = 7.0;
    gspp->labelfont = "Sans";
    gspp->titlefont = "Sans-Bold";
    gspp->labelfontsize = 14;
    gspp->titlefontscale = 1.30;
    gspp->snrminfrac = 0.6;
    gspp->peakradius = 0.25;
    gspp->peakcolorname = "orange";
    gspp->peaktransparency = 0.6;
}
void show_pdfcairo_options(FILE *fpg, gsplot *gspp)
{
    fprintf(fpg,
        "#The following options allow adjustment of some aspects of\n"
        "# the plot.  The pdfcairo terminal type is used and a size\n"
        "# in inches is required.  Fonts for the axis labels and title\n"
        "# may be changed, along with the point size of both.  (The title\n"
        "# point size is specified as a multipler of the label size.)\n"
        "pdfsizeinch=%lf\n"
        "labelfont=%s\n"
        "titlefont=%s\n"
        "labelfontsize=%d\n"
        "titlefontscale=%lf\n"
        "#\n", gspp->pdfsizeinch, gspp->labelfont, gspp->titlefont,
        gspp->labelfontsize, gspp->titlefontscale);
    fprintf(fpg,
        "#The colorbar covers the SNR range from some minimum to the\n"
        "# SNR at the peak found by the search algorithm.  The minimum\n"
        "# is expressed as a multiplier on the lowest contour level.\n"
        "snrminfrac=%lf\n"
        "#\n", gspp->snrminfrac);
    fprintf(fpg,
        "#The peak SNR is marked in the plot by a circle with a radius\n"
        "# specified as a fraction of the grid spacing.  You may specify\n"
        "# the color and a degree of transparency on a scale 0 (solid to\n"
        "# 1 (invisible).\n"
        "peakradius=%lf\n"
        "peakcolorname=%s\n"
        "peaktransparency=%lf\n"
        "#\n", gspp->peakradius, gspp->peakcolorname, gspp->peaktransparency);
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
    memset(gspp, 0, sizeof(gsplot));
    set_colormap_palette(gspp);
    set_pdfcairo_options(gspp);
    set_basic_contour_options(gspp);
    set_rate_unit_sf("ps/s", gspp);
    set_delay_unit_sf("ns", gspp);
}

/* report on what can be set in the gnuplot config file */
void show_gsplot_defaults(FILE *fpg)
{
    gsplot gsp_dummy;
    fprintf(fpg,
        "#There are many options that may be used in gnuplot 3d plots.\n"
        "# We have made some general choices that mirror the original\n"
        "# search output via PGPLOT.  Options are provided to make some\n"
        "# adjustments to this plan.  However, the data file and gnuplot\n"
        "# commands are normally left on disk so you may experiment.\n"
        "# For details, use the 'help' capability of gnuplot.\n"
        "#\n");
    set_gsplot_defaults(&gsp_dummy);
    show_basic_contour_options(fpg, &gsp_dummy);
    show_colormap_palette(fpg, &gsp_dummy);
    show_pdfcairo_options(fpg, &gsp_dummy);
    show_gsplot_contours(fpg);
}

/* Check to see if the line matches a legal edit.  If so, try to parse
 * it, and if it parses correctly, set gpfp->gsp_gcfile to point to the
 * internal staging area here.  Later, when the plots are made, this
 * pointer can be checked.  The following macro is for readability: */
int is_gsplot_option(char *line, int lno, int err, gpconf *gpfp)
{
#define RETURN_PUKE(N,E,LINE,LN,VAR,ERR) do {\
    if (N == E) {\
        if (!(gpfp->gsp_gcfile)) gpfp->gsp_gcfile = ggp;\
        return(1);\
    }\
    msg("Only scanned %d of %d on line %d for variable '%s'", 3, N,E,LN,VAR);\
    msg("'%s'", 3, LINE);\
    return(ERR); } while(0)
    static gsplot gsp_gcfile_private;
    static char colorbar_label[MAX_TXT];
    static char dgrid3dalgorithm[MAX_TXT];
    static char palette_type[MAX_TXT];
    static char palette_options[MAX_TXT];
    static char bgfieldname[MAX_TXT];
    static char labelfont[MAX_TXT];
    static char titlefont[MAX_TXT];
    static char peakcolorname[MAX_TXT];
    gsplot *ggp = &gsp_gcfile_private;
    int ncs;
    /* load up all defaults to make life easier in make_gnucmds() */
    if (gsp_gcfile_private.fileno == 0) {
        set_gsplot_defaults(ggp);
        gsp_gcfile_private.fileno = -1;
        msg("Using options from %s for gnuplot plots", 2, gpfp->gcfile);
    }
    /* test the line and connect to ggp if there is input */
    if (!strncmp(line, "cntr_lowest=", 12)) {
        ncs = sscanf(line, "cntr_lowest=%lf", &ggp->cntr_lowest);
        if (ncs == 1) ggp->cntr_specified ++;
        RETURN_PUKE(ncs, 1, line, lno, "cntr_lowest", err);
    } else if (!strncmp(line, "cntr_increment=", 15)) {
        ncs = sscanf(line, "cntr_increment=%lf", &ggp->cntr_increment);
        if (ncs == 1) ggp->cntr_specified ++;
        RETURN_PUKE(ncs, 1, line, lno, "cntr_increment", err);
    } else if (!strncmp(line, "colorbar_label=", 15)) {
        ncs = sscanf(line, "colorbar_label=%s", colorbar_label);
        if (ncs == 1) {
            ggp->cntr_specified ++;
            ggp->colorbar_label=colorbar_label;
        }
        RETURN_PUKE(ncs, 1, line, lno, "colorbar_label", err);
    } else if (!strncmp(line, "isosamples=", 11)) {
        ncs = sscanf(line, "isosamples=%d", &ggp->isosamples);
        RETURN_PUKE(ncs, 1, line, lno, "isosamples", err);
    } else if (!strncmp(line, "dgrid3dalgorithm=", 17)) {
        ncs = sscanf(line, "dgrid3dalgorithm=%s", dgrid3dalgorithm);
        if (ncs == 1) ggp->dgrid3dalgorithm = dgrid3dalgorithm;
        RETURN_PUKE(ncs, 1, line, lno, "dgrid3dalgorithm", err);
    } else if (!strncmp(line, "dgrid3dalgorder=", 16)) {
        ncs = sscanf(line, "dgrid3dalgorder=%d", &ggp->dgrid3dalgorder);
        RETURN_PUKE(ncs, 1, line, lno, "dgrid3dalgorder", err);
    } else if (!strncmp(line, "bsplineorder=", 13)) {
        ncs = sscanf(line, "bsplineorder=%d", &ggp->bsplineorder);
        RETURN_PUKE(ncs, 1, line, lno, "bsplineorder", err);
    } else if (!strncmp(line, "palette_type=", 13)) {
        ncs = sscanf(line, "palette_type=%s", palette_type);
        if (ncs == 1) ggp->palette_type = palette_type;
        RETURN_PUKE(ncs, 1, line, lno, "palette_type", err);
    } else if (!strncmp(line, "palette_options=", 13)) {
        ncs = sscanf(line, "palette_options=%s", palette_options);
        if (ncs == 1) ggp->palette_options = palette_options;
        RETURN_PUKE(ncs, 1, line, lno, "palette_options", err);
    } else if (!strncmp(line, "bgfieldname=", 12)) {
        ncs = sscanf(line, "bgfieldname=%s", bgfieldname);
        if (ncs == 1) ggp->bgfieldname = bgfieldname;
        RETURN_PUKE(ncs, 1, line, lno, "bgfieldname", err);
    } else if (!strncmp(line, "pdfsizeinch=", 12)) {
        ncs = sscanf(line, "pdfsizeinch=%d", &ggp->pdfsizeinch);
        RETURN_PUKE(ncs, 1, line, lno, "pdfsizeinch", err);
    } else if (!strncmp(line, "labelfont=", 10)) {
        ncs = sscanf(line, "labelfont=%s", labelfont);
        if (ncs == 1) ggp->labelfont = labelfont;
        RETURN_PUKE(ncs, 1, line, lno, "labelfont", err);
    } else if (!strncmp(line, "titlefont=", 10)) {
        ncs = sscanf(line, "titlefont=%s", titlefont);
        if (ncs == 1) ggp->titlefont = titlefont;
        RETURN_PUKE(ncs, 1, line, lno, "titlefont", err);
    } else if (!strncmp(line, "labelfontsize=", 14)) {
        ncs = sscanf(line, "labelfontsize=%d", &ggp->labelfontsize);
        RETURN_PUKE(ncs, 1, line, lno, "labelfontsize", err);
    } else if (!strncmp(line, "titlefontscale=", 15)) {
        ncs = sscanf(line, "titlefontscale=%lf", &ggp->titlefontscale);
        RETURN_PUKE(ncs, 1, line, lno, "titlefontscale", err);
    } else if (!strncmp(line, "snrminfrac=", 11)) {
        ncs = sscanf(line, "snrminfrac=%lf", &ggp->snrminfrac);
        RETURN_PUKE(ncs, 1, line, lno, "snrminfrac", err);
    } else if (!strncmp(line, "peakradius=", 11)) {
        ncs = sscanf(line, "peakradius=%lf", &ggp->peakradius);
        RETURN_PUKE(ncs, 1, line, lno, "peakradius", err);
    } else if (!strncmp(line, "peakcolorname=", 14)) {
        ncs = sscanf(line, "peakcolorname=%s", peakcolorname);
        if (ncs == 1) ggp->peakcolorname = peakcolorname;
        RETURN_PUKE(ncs, 1, line, lno, "peakcolorname", err);
    } else if (!strncmp(line, "peaktransparency=", 17)) {
        ncs = sscanf(line, "peaktransparency=%lf", &ggp->peaktransparency);
        RETURN_PUKE(ncs, 1, line, lno, "peaktransparency", err);
    }
    return(0);                      /* not our problem here */
}
#undef RETURN_PUKE

/* construct the .gnu file from template and options */
void make_gnucmds(gsplot *gspp)
{
    FILE *gfp;
    double ngrid;

    if (!gspp->gfile || !gspp->gnpdf) {
        msg("No gnuplot command file or final PDF provided", 3);
        return;
    }
    if (!(gfp = fopen(gspp->gfile, "w"))) {
        perror("make_gnucmds:fopen");
        msg("Unable to open gnu command file %s", 3, gspp->gfile);
    }
    /* these options depends on peak SNR found */
    set_contour_options(gspp);

    /* now create the various parts */
    fprintf(gfp, GNUPLOT_PDFILE,
        gspp->pdfsizeinch, gspp->ratesize, gspp->delaysize,
        gspp->labelfont, gspp->labelfontsize,
        gspp->label, gspp->srcsnr, gspp->titlefont,
        (int)round(gspp->labelfontsize * gspp->titlefontscale));
    fprintf(gfp, GNUPLOT_CONFIG,
        /* setup options for data and contouring */
        gspp->isosamples, gspp->numrates, gspp->numdelays,
        gspp->dgrid3dalgorithm, gspp->dgrid3dalgorder,
        gspp->bsplineorder, gspp->cntr_specified,
        gspp->cntr_lowest, gspp->cntr_increment,
        /* linear scaling function( indices to rate and delay values */
        gspp->rate_sf, X_COORD_SHIFT, gspp->min_rate,
        gspp->peak_rate, gspp->max_rate, gspp->numrates-1,
        gspp->delay_sf, Y_COORD_SHIFT, gspp->min_delay,
        gspp->peak_delay, gspp->max_delay, gspp->numdelays-1,
        /* ranges and labels */
        gspp->numrates-1, gspp->rate_unit,
        gspp->peak_rate*gspp->rate_sf, gspp->rate_unit,
        gspp->numdelays-1, gspp->delay_unit,
        gspp->peak_delay*gspp->delay_sf, gspp->delay_unit,
        /* zrange and cb label and formula */
        (gspp->cntr_lowest)*gspp->snrminfrac, gspp->snr,
        gspp->colorbar_label, gspp->palette_type, gspp->palette_options);
    /* take the smaller number of grid points to scale peak circle */
    ngrid = (double)gspp->numrates;
    if ((double)gspp->numdelays > ngrid) ngrid = (double)gspp->numdelays;
    fprintf(gfp, GNUPLOT_SPLOT,
        gspp->gnpdf, gspp->bgfieldname,
        gspp->peak_rate, gspp->peak_delay, gspp->peakradius/ngrid,
        gspp->peakcolorname, gspp->peaktransparency, gspp->pfile);
    fclose(gfp);

    /* since next fringe may require other choices */
    nuke_contour_options(gspp);
}
#endif /* BIGGER */
/*
 * eof vim:nospell
 */
