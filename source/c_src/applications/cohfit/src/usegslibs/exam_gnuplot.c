/*
 * Output a file of data formatted for handling by gnuplot
 * Per base-pol data is captured in exdata->edata[bno]
 * and we here finish up for a scan_boundary.  The gnuplot
 * commands are defined in the template file with defined
 * strings:
 *   GNUPLOT_PDFILE     -- pdfcairo setup
 *   GNUPLOT_PERBNO     -- per-file choices
 *   GNUPLOT_SETTINGS   -- standard configuration choices
 *   GNUPLOT_TWOPANEL   -- setup for a SNR under AMP 2-panel plot
 *   GNUPLOT_CODA       -- and end of file marker
 * Each has an accompanying *_BASE define with the approximate
 * size of the string, should allocation be needed.  The first
 * three have %-items for printf().  The others are not configurable.
 *  fringe title
 *  data file
 *  xrange: min and max (%lf)
 *  amprange: min and max (%lf)
 *  snrrange: min and max (%lf)
 *  amp and snr coherence time estimates (%lf)
 *
 * The gnuplot(1) program itself is provided as a Makefile
 * #define if the program was found at configuration time.
 * If the program is missing, the files are still generated
 * (for some other processing) but no pdf is made.  (Technically,
 * if missing, it will be set to /bin/false, but that is a detail.)
 *
 * montage -tile 2x2 -geometry +2+2 -density 300 fff-dets-*pdf ff-dets.pdf
 * montage -tile 2x2 -geometry +2+2 -density 150 fff-dets-*pdf ff-dets.pdf
 *
 * Should use global max x-y on all plots rather than local ones.
 * Should limit SNR and AMP > 0
 */
#include <stdlib.h>
#include <string.h>
#include "cohfit.h"
#include "exam_gnuplot.h"

/* another helper */
void exam_runit(char *cmd)
{
    int rc = system(cmd);
    if (rc) {
        msg("system(%s) failed (%d)", 3, cmd, rc);
        perror("exam_plot:system");
    } else {
        msg("system(%s) successful", 0, cmd);
    }
    free(cmd);
}

/* for readability below */
#define EXAM_RETURN(R,C,S) do {\
        msg("Unable to montage: need %d only have %d chars", 3, R, C);\
        msg("CMD so far: %s", 3, S);\
    return;} while(0)

void exam_montage(char *epat, int elen, int bbn, char *pdf[bbn],
    int density, int rnc[2], int tbbn)
{
    int cmdlen = (bbn+1) * (elen + 8) + strlen(MONTAGE) + 64, nc = 0, bb;
    char *montyc = malloc(cmdlen), *mtpdfo = malloc(elen + 32), *pc;

    if (!montyc) { perror("exam_montage:malloc:1"); return; }
    if (!mtpdfo) { perror("exam_montage:malloc:2"); return; }
    pc = strchr(epat, '%');     /* expect -%<integer>d */
    *--pc = 0;
    nc = snprintf(mtpdfo, elen + 32, " %s-%d.montage.pdf", epat, tbbn);
    *pc = '-';                  /* restore it to be clean */
    if (nc >= elen + 32) EXAM_RETURN(nc, elen+32, mtpdfo);

    /* the basic montage command */
    nc = snprintf(montyc, cmdlen,
        "%s -tile %dx%d -geometry +2+2 -density %d ",
        MONTAGE, rnc[1], rnc[0], density);
    if (--nc >= cmdlen) EXAM_RETURN(nc, cmdlen, montyc);

    /* pile on the file names */
    for (bb = 0; bb < bbn; bb++) {
        nc += snprintf(montyc + nc, strlen(pdf[bb]) + 2, " %s ", pdf[bb]);
        if (--nc >= cmdlen) EXAM_RETURN(nc, cmdlen, montyc);
        msg("%s %d == %d?", 0, montyc, nc, strlen(montyc));
    }

    /* and finally add the output file */
    nc += snprintf(montyc + nc, elen + 16, "%s", mtpdfo);
    if (nc >= cmdlen) EXAM_RETURN(nc, cmdlen, montyc);
    msg("Montage command len %d < limit %d is:", 1, nc, cmdlen);
    exam_runit(montyc);
}

/* a helper that runs system() on the commandfile */
void exam_plot(char *cmds)
{
    int len = strlen(cmds) + strlen(GNUPLOT) + 16;
    char *syscmd = malloc(len);
    snprintf(syscmd, len-1, "%s %s", GNUPLOT, cmds);
    exam_runit(syscmd);
}

/* update limits with the per-plot values and set key locations */
void exam_custom(edatum *edp, double limits[6], char *ksnr, char *kamp)
{
    /* xrange: min and max (%lf) */
    limits[0] = edp->lenmin;
    limits[1] = edp->lenmax;
    /* amprange: min and max (%lf) */
    limits[2] = edp->ampmin;
    limits[3] = edp->ampmax;
    /* snrrange: min and max (%lf) */
    limits[4] = edp->snrmin;
    limits[5] = edp->snrmax;
    if (ksnr) ksnr = "inside bottom left vertical reverse Left";
    if (kamp) kamp = "inside bottom left vertical reverse Left";
}

/* helper function to establish global plot limits */
void exam_setlimits(examdata *exdp, 
    double limits[6], char *ksnr[], char *kamp[])
{
    int bb;
    char *kybot = "inside bottom left vertical reverse Left";
    char *kytop = "inside top right vertical Right";
    edatum *edp;
    double midamp, midsnr, gmamp, gmsnr;

    /* first establish limits */
    for (bb = 0; bb < exdp->nbno; bb++) {
        edp = &exdp->edata[bb];
        if (bb == 0) {              /* first edp establishes values */
            exam_custom(edp, limits, NULL, NULL);
        } else {                    /* otherwise expand as needed */
            if (edp->lenmin < limits[0]) limits[0] = edp->lenmin;
            if (edp->lenmax > limits[1]) limits[1] = edp->lenmax;
            if (edp->ampmin < limits[2]) limits[2] = edp->ampmin;
            if (edp->ampmax > limits[3]) limits[3] = edp->ampmax;
            if (edp->snrmin < limits[4]) limits[4] = edp->snrmin;
            if (edp->snrmax > limits[5]) limits[5] = edp->snrmax;
        }
    }
    if (exdp->customlimits) return;

    gmamp = (limits[2] + limits[3]) / 2.0;
    gmsnr = (limits[4] + limits[5]) / 2.0;
    for (bb = 0; bb < exdp->nbno; bb++) {
        edp = &exdp->edata[bb];
        midamp = (edp->ampmin + edp->ampmax) / 2.0;
        midsnr = (edp->snrmin + edp->snrmax) / 2.0;
        ksnr[bb] = midsnr < gmsnr ? kytop : kybot;
        kamp[bb] = midamp < gmamp ? kytop : kybot;
    }
}

/* swap format to indicate best amp fit */
void exam_bestamp(char *ltit[], int best)
{
    switch (best) {
    case FITOPT_NDX_PS:
        ltit[FITOPT_NDX_PS] = "lt 2 tit '{/:Bold plateau-slope}'";
        ltit[FITOPT_NDX_PO] = "lt 3 tit 'plateau-only'";
        ltit[FITOPT_NDX_SO] = "lt 4 tit 'slope-only'";
        ltit[FITOPT_NDX_3PT] = "lt 6 tit 'parabolic'";
        ltit[FITOPT_NDX_CBS] = "lt 7 tit '{/:Bold cubic spline}'";
        break;
    case FITOPT_NDX_PO:
        ltit[FITOPT_NDX_PS] = "lt 3 tit 'plateau-slope'";
        ltit[FITOPT_NDX_PO] = "lt 2 tit '{/:Bold plateau-only}'";
        ltit[FITOPT_NDX_SO] = "lt 4 tit 'slope-only'";
        ltit[FITOPT_NDX_3PT] = "lt 6 tit 'parabolic'";
        ltit[FITOPT_NDX_CBS] = "lt 7 tit '{/:Bold cubic spline}'";
        break;
    case FITOPT_NDX_SO:
        ltit[FITOPT_NDX_PS] = "lt 4 tit 'plateau-slope'";
        ltit[FITOPT_NDX_PO] = "lt 3 tit 'plateau-only'";
        ltit[FITOPT_NDX_SO] = "lt 2 tit '{/:Bold slope-only}'";
        ltit[FITOPT_NDX_3PT] = "lt 6 tit 'parabolic'";
        ltit[FITOPT_NDX_CBS] = "lt 7 tit '{/:Bold cubic spline}'";
        break;
    default:
        msg("Developer error in exam_bestamp() best = %d", 3, best);
        exit(1);
    }
}

/* populate the label and object details */
void exam_labobj(edatum *edp, labobj *snr, labobj *abp, labobj *ach,
    double lim[6])
{
    /* for starters */
    snr->fc = "dark-red";       snr->dens = 0.1;
    abp->fc = "forest-green";   abp->dens = 0.1;
    ach->fc = "navy";           ach->dens = 0.1;

    /* rectangles from left,bottom to right,top */
    snr->top = edp->snrmax;
    snr->lft = edp->snr_cotime - 0.5;
    snr->rgt = edp->snr_cotime + 0.5;
    snr->bot = edp->snrmin;
    abp->top = edp->ampmax;
    abp->lft = edp->breakpoint - 0.5;
    abp->rgt = edp->breakpoint + 0.5;
    abp->bot = edp->ampmin;
    ach->top = edp->ampmax;
    ach->lft = edp->ampl_cotime - 0.5;
    ach->rgt = edp->ampl_cotime + 0.5;
    ach->bot = edp->ampmin;

    /* labels are near a rect. corner: bp is right just, others are left;
     * the y axis is upper or lower depending on where the data is in plot */
    {
        double pltamp = (lim[2] + lim[3]) / 2.0;
        double pltsnr = (lim[4] + lim[5]) / 2.0;
        double midamp = (edp->ampmin + edp->ampmax) / 2.0;
        double midsnr = (edp->snrmin + edp->snrmax) / 2.0;
        snr->x = snr->rgt * 1.05;
        snr->y = (midsnr < pltsnr) ? snr->top : snr->bot;
        abp->x = abp->lft / 1.05;
        abp->y = (midamp < pltamp) ? abp->top : abp->bot;
        ach->x = ach->rgt * 1.05;
        ach->y = (midamp < pltamp) ? ach->top : ach->bot;
    }
}

/* this creates .gnu and makes the pdfs */
void exam_gnuplot(examdata *exdp)
{
    int bb, bbn, lw[8];
    char *pdfbno[exdp->nbno], *gnucmds = NULL, *dt[8], *rgb[8];
    char *keysnr[exdp->nbno], *keyamp[exdp->nbno], *ltit[NFITOPT];
    double limits[6];
    edatum *edp;
    labobj snr, abp, ach;
    FILE *fpg;
    /* exit quietly if no data to work with */
    if (!exdp->epat || 0 == exdp->elen || 0 == exdp->nbno) return;
    msg("", 1);
    msg("exam_gnuplot on '%s' (%d) with %d files: %s %s", 1,
        exdp->epat, exdp->elen, exdp->nbno,
        exdp->gnuplot ? GNUPLOT : "disabled",
        exdp->montage ? MONTAGE : "disabled");

    /* default colors */
    rgb[0] = "ignored";       rgb[1] = "medium-blue";
    rgb[2] = "dark-violet";   rgb[3] = "olive";
    rgb[4] = "orange-red";    rgb[5] = "navy";
    rgb[6] = "light-red";     rgb[7] = "forest-green";
    /* defaults for lw, dt, lt and title that have ps and cbs as winners */
    lw[0] = lw[1] = lw[5] = lw[6] = 1;
    lw[2] = lw[3] = lw[4] = 2;
    lw[7] = 3;
    dt[0] = dt[1] = dt[2] = dt[5] = dt[7] = "solid";
    dt[3] = "'.._'";
    dt[4] = "'...'";
    dt[6] = "'---'";
    
    /* determine the global limits for this scan group */
    exam_setlimits(exdp, limits, keysnr, keyamp);

    /* generate a gnu command file */
    for (bb = 0; bb < exdp->nbno; bb++) {
        edp = &exdp->edata[bb];             /* convenience pointer */
        gnucmds = malloc(strlen(edp->examfile) + 16);
        if (!gnucmds) { perror("examgnuplot:malloc:1"); return; }
        pdfbno[bb] = malloc(strlen(edp->examfile) + 16);
        if (!pdfbno[bb]) { perror("examgnuplot:malloc:2"); return; }
        sprintf(gnucmds, "%s.gnu", edp->examfile);
        sprintf(pdfbno[bb], "%s.pdf", edp->examfile);
        fpg = fopen(gnucmds, "w");
        /* see exam_gnuplot.h for argument dispositions */
        fprintf(fpg, GNUPLOT_PDFILE, 5.0, 6.0, "Sans", 14, pdfbno[bb]);
        if (exdp->customlimits)
            exam_custom(edp, limits, keysnr[bb], keyamp[bb]);
        fprintf(fpg, GNUPLOT_PERBNO,
            edp->frlabel, edp->examfile,
            limits[0], limits[1], limits[2], limits[3], limits[4], limits[5],
            edp->ampl_cotime, edp->snr_cotime);
        exam_labobj(edp, &snr, &abp, &ach, limits);
        fprintf(fpg, GNUPLOT_SETTINGS,
            lw[1], dt[1], rgb[1], lw[2], dt[2], rgb[2], lw[3], dt[3], rgb[3],
            lw[4], dt[4], rgb[4], lw[5], dt[5], rgb[5], lw[6], dt[6], rgb[6],
            lw[7], dt[7], rgb[7]);
        exam_bestamp(ltit, edp->bestndx);
        fprintf(fpg, GNUPLOT_TWOPANEL,
            keysnr[bb],
            edp->snr_cotime, snr.x, snr.y, snr.fc,
            snr.lft,snr.bot,snr.rgt,snr.top, snr.fc, snr.dens, snr.fc,
            ltit[FITOPT_NDX_3PT], ltit[FITOPT_NDX_CBS],
            keyamp[bb], ((edp->bestndx == FITOPT_NDX_PS) ? 1 : 0),
            edp->breakpoint, abp.x, abp.y, abp.fc,
            abp.lft,abp.bot,abp.rgt,abp.top, abp.fc, abp.dens, abp.fc,
            edp->ampl_cotime, ach.x, ach.y, ach.fc, 
            ach.lft,ach.bot,ach.rgt,ach.top, ach.fc, ach.dens, ach.fc,
            ltit[FITOPT_NDX_PS], ltit[FITOPT_NDX_PO], ltit[FITOPT_NDX_SO]);
        fputs(GNUPLOT_CODA, fpg);
        fclose(fpg);
        if (exdp->gnuplot) exam_plot(gnucmds);
        free(gnucmds);
    }
    bbn = bb;
    if (exdp->gnuplot && exdp->montage)
        exam_montage(exdp->epat, exdp->elen, bbn, pdfbno,
            exdp->density, exdp->rnc, exdp->pbno + exdp->nbno);

    // summary output file

    for (bb = 0; bb < bbn; bb++)
        if (pdfbno[bb]) free(pdfbno[bb]);
}

/*
 * eof vim:nospell
 */
