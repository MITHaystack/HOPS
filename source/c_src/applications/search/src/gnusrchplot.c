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
    len = strlen(cmds) + strlen(GNUPLOT) + 16;
    syscmd = malloc(len);
    snprintf(syscmd, len-1, "%s %s", GNUPLOT, cmds);
    monty_runit(syscmd);
}

/* cache the data file name in gnupdfs reallocating as needed */
void save_gpdf(gsplot *gspp)
{
    size_t blob = MAX_BNO * sizeof(char *);
    gpconf *gc = gspp->gpcopy;
    if (!gc->gnupdfs) gc->gnupdfs = malloc(gc->gpdfs_alloc = blob);
    /* make sure there is an extra one for the last command */
    if ((gc->npdfs + 1)*sizeof(char *) == gc->gpdfs_alloc)
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
    gspp->gpcopy = gpfp;                /* convenience ptr */
    pfile = calloc(4*(gpfp->patlen + 1), 1);
    if (!pfile) { perror("gnusrchplot:malloc"); return(NULL); }

    gspp->pfile = pfile;
    snprintf(gspp->pfile, gpfp->patlen, gpfp->gplatt, fno);
    strcat(gspp->pfile, ".data");
    msg("splot data file is %s (%p) ALLOC", 1, gspp->pfile, gspp->pfile);

    gspp->gfile = pfile + 1*gpfp->patlen;
    gspp->gfile[-1] = 0;                 /* ensure null-termination */
    snprintf(gspp->gfile, gpfp->patlen, gpfp->gplatt, fno);
    strcat(gspp->gfile, ".gnu");
    msg("splot gnu cmd file is %s (%p)", 1, gspp->gfile, gspp->gfile);

    gspp->gnpdf = pfile + 2*gpfp->patlen;
    gspp->gnpdf[-1] = 0;                 /* ensure null-termination */
    snprintf(gspp->gnpdf, gpfp->patlen, gpfp->gplatt, fno);
    strcat(gspp->gnpdf, ".pdf");
    gspp->gnpdf[gpfp->patlen-1] = 0;     /* ensure null-termination */
    msg("splot gnu pdf file is %s (%p)", 1, gspp->gnpdf, gspp->gnpdf);
    return(pfile);
}

/* open the data file and sort out aspect ratio */
FILE *open_gdata_file(srchsum *sb, gpconf *gpfp, gsplot *gspp)
{
    int size;
    FILE *ofp;

    /* open the file */
    ofp = fopen(gspp->pfile, "w");
    if (!ofp) { perror("open_gdata_file:fopen"); return(NULL); }
    fprintf(ofp, "#\n# search plot for %s\n", gspp->frname);
    fprintf(ofp, "#'%s'\n#\n", gspp->pfile);

    /* rate and delay are size multipliers on nominal size */
    size = sb->nrate;
    if (sb->ndelay > size) size = sb->ndelay;
    gspp->ratesize = 1.0 * ((double)sb->nrate / (double)size);
    gspp->delaysize = 1.0 * ((double)sb->ndelay / (double)size);
    if (gpfp->asqr) gspp->delaysize = gspp->ratesize = 1.0;
    fprintf(ofp, "# rate,delay size: %.3f,%.3f\n",
        gspp->ratesize, gspp->delaysize);
    return(ofp);
}

/* construct the plot title/label */
void write_gplot_label(FILE *ofp, fringesum *frdt, gsplot *gspp)
{
    int year, day, hour, min, sec;
    int_to_time (frdt->time_tag, &year, &day, &hour, &min, &sec);
    snprintf(gspp->label, sizeof(gspp->label),
        "%04d-%03d-%02d%02d%02d/%s.%c.%d.%s pol %2s",
        1900+year, day, hour, min, sec, frdt->baseline, frdt->freq_code,
        frdt->extent_no, frdt->root_id, frdt->polarization);
    /* this appears as the plot overtitle */
    fprintf(ofp, "# fringe: %s\n", gspp->label);
}

/* write number of items into the header of the file */
void write_gplot_info(FILE *ofp, srchsum *sb, gsplot *gspp)
{
    fringesum *frdt = sb->datum;
    /* this appears as the plot undertitle */
    snprintf(gspp->srcsnr, sizeof(gspp->srcsnr),
        "Source: %s  SNR: %.2f",
        (gspp->source = frdt->source), (gspp->snr = frdt->snr));
    fprintf(ofp, "# %s\n", gspp->srcsnr);
    fprintf(ofp, "# resid rate: (ps/s) %f .. %f\n",
        (gspp->min_rate = sb->min_rate), (gspp->max_rate = sb->max_rate));
    fprintf(ofp, "# resid delay: (us) %f .. %f\n",
        (gspp->min_delay = sb->min_delay), (gspp->max_delay = sb->max_delay));
    fprintf(ofp, "# peak rate %5.2f\n", (gspp->peak_rate=frdt->delay_rate));
    fprintf(ofp, "# peak delay %5.2f\n", (gspp->peak_delay = frdt->mbdelay));
    fprintf(ofp, "# num rates %d\n", (gspp->numrates = sb->nrate));
    fprintf(ofp, "# num delays %d\n", (gspp->numdelays = sb->ndelay));
}

/* follow the plot_srchdata() logic with one plot per baseline */
void gnusrchplot(int nout, srchsum *srchp, gpconf *gpfp)
{
    int base, nbase = 0, fileno = gpfp->nprv, ii, jj;
    FILE *ofp;
    srchsum *sb;
    fringesum *frdt;
    gsplot gsp, *gspp;

    msg("gnusrchplot with config %s", 2, gpfp->gsp_gcfile);
    if (gpfp->gsp_gcfile) {
        /* use the values from the config file */
        gspp = gpfp->gsp_gcfile;
        msg("Using options from %s for gnuplot plots", 2, gpfp->gcfile);
    } else {
        /* these are independent of the data input */
        set_gsplot_defaults((gspp = &gsp));
        msg("Using defaults for gnuplot plots", 2);
    }

    while (srchp[nbase].datum != NULL) nbase++;
    if (nbase != nout) {
        msg("The # baselines found != # written (%d != %d)", 3, nbase, nout);
        return;
    }
    for (base=0; base<nbase; base++, fileno++) {
        sb = srchp + base;
        frdt = sb->datum;
        /* setup of gnuplot filenames and dig out fringe name */
        if (!setup_gnu_filenames(fileno, gspp, gpfp)) return;
        gspp->frname = fringename(frdt);
        if ((sb->nrate == 1) || (sb->ndelay == 1)) {
            msg("Cannot plot 1-D grid for %s", 2, gspp->frname);
            continue;
        }
        /* open the data file and share some of info in header */
        if (!(ofp = open_gdata_file(sb, gpfp, gspp))) continue;
        write_gplot_label(ofp, sb->datum, gspp);
        write_gplot_info(ofp, sb, gspp);
        /* gnuplot help splot datafile example */
        for (ii = 0; ii < sb->nrate;  ii++)
            for (jj = 0; jj < sb->ndelay; jj++)
                fprintf(ofp, "%d %d %5.2f\n", ii, jj, sb->snr[ii][jj]);
        fputs("#\n# eodata\n#\n", ofp);
        fclose(ofp);
        make_gnu_splot(gspp);
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
