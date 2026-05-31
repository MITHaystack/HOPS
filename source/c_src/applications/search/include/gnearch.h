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
#endif /* BIGGER */
    int ncols, nrows, arat, plot;   /* [-g] cols x rows : arat ; plot? */
} gpconf;

#endif /* __gnearch_h__ */

/*
 * eof vim:nospell
 */
