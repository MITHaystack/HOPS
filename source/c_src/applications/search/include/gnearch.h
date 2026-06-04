/*
 * Support for replacing the PGPLOT graphics with gnuplot.
 * For the present, this is considered a second output path.
 */
#ifndef __gnearch_h__
#define __gnearch_h__

#if BIGGER
#include <math.h>
#define MAX_TXT 80
#endif /* BIGGER */

/* captures command-line options and gnuplot configuration options */
typedef struct gpconf {
#if BIGGER
    char *pdfile;                   /* [-d] pdf file if /pdf given */
    char *devp;                     /* [-d] dev name for pgplot */
    char *gcfile;                   /* gnuplot config filename */
    char *gplatt;                   /* gnuplot filename pattern */
    int patlen;                     /* strlen(gplatt) */
    int nprv;                       /* previous nout */
    int montage, density, npdfs;    /* make a montage w/ density */
    int gpdfs_alloc;                /* size of allocated gnupdfs */
    int nukegnu, nukedata;          /* unlink intermediates */
    char **gnupdfs;                 /* from npdfs output pdf files */
#endif /* BIGGER */
    int ncols, nrows, asqr;         /* [-g] cols x rows : asqr ; */
    int pplt, gplt;                 /* pgplot and gnuplot */
} gpconf;

#if BIGGER
/* captures the items used in the gnuplot graphic */
typedef struct gsplot {
    gpconf *gpcopy;                 /* convenience copy of ptr to gpconf */
    /* these three are in the same malloc blob */
    int fileno;                     /* counter for the filename */
    char *pfile;                    /* data file name */
    char *gfile;                    /* gnu cmd file name */
    char *gnpdf;                    /* resulting gnuplot pdf file */
    /* details for the gnu plot derived from data */
    char *frname;                   /* fringe name */
    char label[MAX_TXT];            /* scan-time, timestamp, pol title */
    char srcsnr[MAX_TXT];           /* source and peak SNR subtitle */
    char *source;                   /* source */
    double snr;                     /* SNR */
    int numrates, numdelays;        /* count of rates and delays */
    double ratesize, delaysize;     /* multipliers on screen dimensions */
    double min_rate, max_rate;      /* min and max rates (ps/s) */
    double min_delay, max_delay;    /* min and max delays (us) */
    double peak_rate, peak_delay;   /* peak of rate and delay */
    /* other details from somewhere else */
    int isosamples;                 /* for smooth contours */
    int bsplineorder;               /* ditto */
    int dgrid3dalgorder;            /* algorithm order value */
    double cntr_lowest;             /* lowest value contour level */
    double cntr_increment;          /* increment on contour levels */
    char *dgrid3dalgorithm;         /* dgrid3d algorithm name */
    char *colorbar_label;           /* label for the colorbar */
    char *palette_rgbformulae;      /* controls the colors via formulae */
    char *bgfieldname;              /* color name for snr<min background */
    double min_snr;                 /* lowest SNR level (e.g. 3.0) */
} gsplot;

/* functions in srch_montage.c */
extern void monty_runit(char *);
extern void search_montage(gpconf *);

/* functions in gnusrchplot.c */
extern void gnu_plot_doit(char *);
extern void save_gpdf(gsplot *);
extern void make_gnu_splot(gsplot *);
extern char *setup_gnu_filenames(int, gsplot *, gpconf *);
extern FILE *open_gdata_file(srchsum *, gpconf *, gsplot *);
extern void write_gplot_label(FILE *, fringesum *, gsplot *);
extern void write_gplot_info(FILE *, srchsum *, gsplot *);
extern void gnusrchplot(int, srchsum *, gpconf *);

/* functions in make_gnucmds.c, with templates in splotemp.h */
extern void set_gsplot_defaults(gsplot *gspp);
extern void make_gnucmds(gsplot *);

/* functions in gnedits.c */
extern int gnuexists(char *, int *);
extern int create_gnuconf(char *, gpconf *);
extern int puke(int, char *, char *, int);
extern int gnuparse(char *, gpconf *);
extern int gnuedits(char *, gpconf *);

/* functions in gnearch.c */
extern int gnuconfile(int, char *, gpconf *);
extern int gargparse(char *, gpconf *);
extern void gnufinish(gpconf *);
#endif /* BIGGER */

#endif /* __gnearch_h__ */
/*
 * eof vim:nospell
 */
