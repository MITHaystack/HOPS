/*
 * Support for replacing the PGPLOT graphics with gnuplot.
 * For the present, this is considered a second output path.
 */
#ifndef __gnearch_h__
#define __gnearch_h__

#if BIGGER
#define MAX_TXT 80
#endif /* BIGGER */

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
    char **gnupdfs;                 /* from npdfs output pdf files */
    int savegnu, savedata;          /* don't unlink intermediates */
#endif /* BIGGER */
    int ncols, nrows, asqr;         /* [-g] cols x rows : asqr ; */
    int pplt, gplt;                 /* pgplot and gnuplot */
} gpconf;

#if BIGGER
extern void monty_runit(char *cmd);
extern void search_montage(gpconf *);
extern void gnu_plot_doit(char *cmds);
extern void save_gpdf(char *, gpconf *);
extern void gnusrchplt(int, srchsum *, gpconf *);

extern int gnuexists(char *, int *);
extern int create_gnuconf(char *, gpconf *);
extern int gnuparse(char *, gpconf *);
extern int gnuedits(char *, gpconf *);
extern int gnuconfile(int, char *, gpconf *);
extern int gargparse(char *, gpconf *);
extern void gnufinish(gpconf *);
#endif /* BIGGER */

#endif /* __gnearch_h__ */
/*
 * eof vim:nospell
 */
