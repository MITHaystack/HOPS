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
    char *gplatt;                   /* gnuplot filename pattern */
    int nprv;                       /* previous nout */
#endif /* BIGGER */
    int ncols, nrows, asqr, plot;   /* [-g] cols x rows : asqr ; plot? */
} gpconf;

extern int gnuexists(char *, int *);
extern int create_gnuconf(char *, gpconf *);
extern int gnuparse(char *, gpconf *);
extern int gnuedits(char *, gpconf *);
extern int gnuconfile(int, char *, gpconf *);
extern int gargparse(char *, gpconf *);
extern void gnusrchplt(int, srchsum *, gpconf *);
extern void gnufinish(gpconf *);

#endif /* __gnearch_h__ */
/*
 * eof vim:nospell
 */
