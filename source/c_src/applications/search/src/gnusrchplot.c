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
 * see setup_gnu_filenames() for allocation.  These are freed at
 * the end in search.c:gnufinish()
 */
#include "search.h"
#if BIGGER

/* a helper that builds the gnuplot command
 * monty_runit() is pulled from srch_montage.c */
void gnu_plot_doit(char *cmds)
{
    int len;
    char *syscmd;
    if (!cmds) return;
    syscmd = malloc(len);
    len = strlen(cmds) + strlen(GNUPLOT) + 16;
    snprintf(syscmd, len-1, "%s %s", GNUPLOT, cmds);
    monty_runit(syscmd);
}

/* cache the data file name in gnupdfs reallocating as needed */
void save_gpdf(gsplot *gspp)
{
    size_t blob = MAX_BNO * sizeof(char *);
    gpconf *gc = gspp->gpcopy;
    if (!gc->gnupdfs) gc->gnupdfs = malloc(gc->gpdfs_alloc = blob);
    if (gc->npdfs == gc->gpdfs_alloc)
        gc->gnupdfs = realloc(gc->gnupdfs, gc->gpdfs_alloc += blob);
    gc->gnupdfs[gc->npdfs] = gspp->gnpdf;
    /* make sure the next one is nulled just in case npdfs is wrong */
    gc->gnupdfs[++(gc->npdfs)] = NULL;
}

/* create .gnu file, run gnuplot and save the resultant PDF file */
void make_gnu_splot(gsplot *gspp)
{
    /* build the splot commands in gspp->gfile */
    make_gnucmds(gspp);
    /* and do the dance */
    gnu_plot_doit(gspp->gfile);
    /* if all of that worked, save the result and maybe data and cmds */
    if (gspp->gnpdf) save_gpdf(gspp);
    if (gspp->gfile && gspp->gpcopy->nukegnu)  unlink(gspp->gfile);
    if (gspp->pfile && gspp->gpcopy->nukedata) unlink(gspp->pfile);
}

/* set up the .data, .gnu and .pdf filenames in pfile allocation */
char *setup_gnu_filenames(int fno, gsplot *gspp, gpconf *gpfp)
{
    char *pfile;
    msg("setup_gnu_filenames: pattern is %s length is %d", 0,
        gpfp->gplatt, gpfp->patlen);
    memset(gspp, 0, sizeof(gsplot));
    gspp->gpcopy = gpfp;                /* convenience ptr */
    pfile = calloc(3*(gpfp->patlen) + 10, 1);
    if (!pfile) { perror("gnusrchplot:malloc"); return(NULL); }

    gspp->pfile = pfile;
    snprintf(gspp->pfile, gpfp->patlen, gpfp->gplatt, fno);
    strcat(gspp->pfile, ".data");
    msg("splot data file is %s (%p) ALLOC", 0, gspp->pfile, gspp->pfile);

    gspp->gfile = pfile + 1*gpfp->patlen;
    gspp->gfile[-1] = 0;                 /* ensure null-termination */
    snprintf(gspp->gfile, gpfp->patlen, gpfp->gplatt, fno);
    strcat(gspp->gfile, ".gnu");
    msg("splot gnu cmd file is %s (%p)", 0, gspp->gfile, gspp->gfile);

    gspp->gnpdf = pfile + 2*gpfp->patlen;
    gspp->gnpdf[-1] = 0;                 /* ensure null-termination */
    snprintf(gspp->gnpdf, gpfp->patlen, gpfp->gplatt, fno);
    strcat(gspp->gnpdf, ".pdf");
    gspp->gnpdf[gpfp->patlen-1] = 0;     /* ensure null-termination */
    msg("splot gnu pdf file is %s (%p)", 0, gspp->gnpdf, gspp->gnpdf);
    return(pfile);
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
    gsplot gsp;

    while (srchp[nbase].datum != NULL) nbase++;
    if (nbase != nout) {
        msg("The # baselines found != # written (%d != %d)", 3, nbase, nout);
        return;
    }
    for (base=0; base<nbase; base++, fileno++) {
        sb = srchp + base;
        frdt = sb->datum;
        /* setup of gnuplot filenames */
        if (!(pfile = setup_gnu_filenames(fileno, &gsp, gpfp))) return;
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
        fprintf(ofp, "#\n# search plot for %s\n#\n",
            (gsp.frname = frname));
        fprintf(ofp, "# rate,delay size: %.3f,%.3f\n", ratesize, delaysize);
        int_to_time (frdt->time_tag, &year, &day, &hour, &min, &sec);
        snprintf(label, MAX_TXT, "%04d-%03d-%02d%02d%02d/%s.%c.%d.%s pol %2s",
            1900+year, day, hour, min, sec, frdt->baseline, frdt->freq_code,
            frdt->extent_no, frdt->root_id, frdt->polarization);
        fprintf(ofp, "# fringe: %s\n", (gsp.label = label));
        fprintf(ofp, "# source: %s SNR: %5.2f\n",
            (gsp.source = frdt->source), (gsp.snr = frdt->snr));
        fprintf(ofp, "# resid rate: (ps/s) %f .. %f\n",
            (gsp.min_rate = sb->min_rate), (gsp.max_rate = sb->max_rate));
        fprintf(ofp, "# resid delay: (us) %f .. %f\n",
            (gsp.min_delay = sb->min_delay), (gsp.max_delay = sb->max_delay));
        fprintf(ofp, "# peak rate %5.2f\n", (gsp.peak_rate=frdt->delay_rate));
        fprintf(ofp, "# peak delay %5.2f\n", (gsp.peak_delay = frdt->mbdelay));
        /* gnuplot help splot datafile example */
        for (ii = 0; ii < sb->ndelay; ii++) {
            for (jj=0; jj<sb->nrate; jj++) {
                fprintf(ofp, "%d %d %5.2f\n", ii, jj, sb->snr[jj][ii]);
            }
        }
        fputs("#\n# eodata\n#\n", ofp);
        fclose(ofp);
        make_gnu_splot(&gsp);
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
