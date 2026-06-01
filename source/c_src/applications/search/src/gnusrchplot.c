/*
 * This is called to make one PDF using gnuplot
 * after the corresponding PGPLOT subplot.
 *
 * Filenames are saved in gnupdfs[] with the count in npdfs and the
 * allocated number bumped in MAX_BNO increments.  Note that gnupdfs
 * ultimately holds the pdf filenames, but the data and gnu names are
 * related, so we allocate 3x what we need and hold .data, .gnu and
 * .pdf in slots [0,1,2]*patlen (which holds some slop), i.e.
 *   (patt).data @ gpfp->gnupdfs[ii] + 0*gpfp->patlen
 *   (patt).gnu  @ gpfp->gnupdfs[ii] + 1*gpfp->patlen
 *   (patt).pdf  @ gpfp->gnupdfs[ii] + 2*gpfp->patlen
 */
#include "search.h"
#if BIGGER

/* a helper that builds the gnuplot command */
void gnu_plot_doit(char *cmds)
{
    int len = strlen(cmds) + strlen(GNUPLOT) + 16;
    char *syscmd = malloc(len);
    snprintf(syscmd, len-1, "%s %s", GNUPLOT, cmds);
    monty_runit(syscmd);
}

/* cache the data file name in gnupdfs reallocating as needed */
void save_gpdf(char *pfile, gpconf *gpfp)
{
    if (!gpfp->gnupdfs) gpfp->gnupdfs = malloc(gpfp->gpdfs_alloc = MAX_BNO);
    if (gpfp->npdfs == gpfp->gpdfs_alloc)
        gpfp->gnupdfs = realloc(gpfp->gnupdfs, gpfp->gpdfs_alloc += MAX_BNO);
    gpfp->gnupdfs[gpfp->npdfs] = pfile;
    (gpfp->npdfs)++;
}

/* follow the plot_srchdata() logic with one plot per baseline */
void gnusrchplot(int nout, srchsum *srchp, gpconf *gpfp)
{
    int base, nbase = 0, fileno = gpfp->nprv, size, ii, jj;
    int year, day, hour, min, sec;
    double ratesize, delaysize;
    char *pfile, *frname, label[MAX_TXT];
    FILE *ofp;
    srchsum *sb;
    fringesum *frdt;

    while (srchp[nbase].datum != NULL) nbase++;
    if (nbase != nout) {
        msg("The # baselines found != # written (%d != %d)", 3, nbase, nout);
        return;
    }
    for (base=0; base<nbase; base++, fileno++) {
        sb = srchp + base;
        frdt = sb->datum;
        pfile = malloc(gpfp->patlen + 10);
        if (!pfile) { perror("gnusrchplot:malloc"); return; }
        snprintf(pfile, 3*(gpfp->patlen), gpfp->gplatt, fileno);
        strcat(pfile, ".data");
        ofp = fopen(pfile, "w");
        frname = fringename(frdt);
        if ((sb->nrate == 1) || (sb->ndelay == 1)) {
            msg("Cannot plot 1-D grid for %s", 2, frname);
            continue;
        }
        /* Make cells square to indicate true search space represented */
        size = sb->nrate;
        if (sb->ndelay > size) size = sb->ndelay;
        ratesize = 0.7 * ((double)sb->nrate / (double)size);
        delaysize = 0.7 * ((double)sb->ndelay / (double)size);
        /* or force it to be a square plot using 0.7 of the plot area */
        if (gpfp->asqr) delaysize = ratesize = 0.7;
        fprintf(ofp, "#'%s'\n", pfile);
        fprintf(ofp, "#\n# search plot for %s\n#\n", frname);
        fprintf(ofp, "# rate,delay size: %.3f,%.3f\n", ratesize, delaysize);
        int_to_time (frdt->time_tag, &year, &day, &hour, &min, &sec);
        snprintf(label, MAX_TXT, "%04d-%03d-%02d%02d%02d/%s.%c.%d.%s pol %2s",
            1900+year, day, hour, min, sec, frdt->baseline, frdt->freq_code,
            frdt->extent_no, frdt->root_id, frdt->polarization);
        fprintf(ofp, "# fringe: %s\n", label);
        fprintf(ofp, "# source: %s SNR: %5.2f\n", frdt->source, frdt->snr);
        fprintf(ofp, "# resid rate: (ps/s) %f .. %f\n",
            sb->min_rate, sb->max_rate);
        fprintf(ofp, "# resid delay: (ps/s) %f .. %f\n",
            sb->min_delay, sb->max_delay);
        fprintf(ofp, "# peak rate %5.2f\n", frdt->delay_rate);
        fprintf(ofp, "# peak delay %5.2f\n", frdt->mbdelay);
        /* gnuplot help splot datafile example */
        for (ii = 0; ii < sb->ndelay; ii++) {
            for (jj=0; jj<sb->nrate; jj++) {
                fprintf(ofp, "%d %d %5.2f\n", ii, jj, sb->snr[jj][ii]);
            }
        }
        fputs("#\n# eodata\n#\n", ofp);
        save_gpdf(pfile, gpfp);
        fclose(ofp);
    }
    /* sanity check the counters */
    gpfp->nprv += nout;
    if (gpfp->nprv != fileno) {
        msg("Warning, file counter fail: %d != %d", 3, gpfp->nprv, fileno);
    }
}
#endif /* BIGGER */
/*
 * eof vim:nospell
 */
