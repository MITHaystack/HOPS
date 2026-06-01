/*
 * This handles the configuration file in support of the
 * plethora of gnuplot options.  It also carries the 4
 * options from the original PGPLOT implementation:
 *  ncols, nrows, asqr, plot.
 *
 * The general logic follows exam_edits.c of cohfit
 * with some pruning.
 */
#include "search.h"
#if BIGGER
#include <sys/stat.h>

/* see if the file exists; return 1 if found else the errno */
int gnuexists(char *gfile, int *serp)
{
    struct stat sb;
    if ((stat(gfile, &sb) < 0) && ((*serp = errno) == ENOENT)) return(0);
    if (*serp) {                    /* this cannot not end very well */
        msg("Some error to fix with file %s:", 3, gfile);
        msg("  %s", 3, strerror(*serp));
        return(-1);
    }
    return(1);                      /* file exists and is e.g. readable */
}

/* create the template config file returning -1 if ok >0 otherwise */
int create_gnuconf(char *gfile, gpconf *gpfp)
{
    FILE *fpg = fopen(gfile, "w");
    if (!fpg) {
        perror("create_gnuconf:fopen(w)");
        return(199);                /* errno values are < 133 today */
    }
    fprintf(fpg,
        "#search configuration file -- this line is mandatory\n"
        "#\n"
        "#Lines that begin with # are comments and\n"
        "# in any case lines must have < %d characters.\n"
        "#\n"
        "#Parameters are set with something= ... where\n"
        "# there should be no space before the '=' character\n"
        "# and an exact match to 'something' is required.\n"
        "# There should be only one such assignment per line.\n"
        "#\n"
        "#Values are either integer or floating point and are\n"
        "# correspondingly scanned by %%d or %%f or %%lf.  Booleans\n"
        "# are expressed as 0 for FALSE and nonzero for TRUE.\n"
        "#\n"
        "#The following line sets the number of rows x columns\n"
        "# : aspect ratio (square if nonzero) for both the PGPLOT\n"
        "# as well as the montage:\n"
        "CxR:A= %dx%d:%d\n"
        "#\n", MAX_TXT, gpfp->ncols, gpfp->nrows, gpfp->asqr);

    /* other things */

    fprintf(fpg, "#\n# eoc\n#\n"); 
    fclose(fpg);
    msg("Created graphic config file '%s'", 2, gfile);
    return(-1);
}

/* private support to print msg and exit with some uniq number */
int puke(int lno, char *wye, char *line, int errval)
{
    msg(wye, 3, lno);
    msg("'%s'", 3, line);
    return(errval);
}

/* parse the config file returning 0 if ok, >0 otherwise */
int gnuparse(char *gfile, gpconf *gpfp)
{
    FILE *fpg = fopen(gfile, "r");
    char line[MAX_TXT + 10];
    int lno = 0, ncs;
    if (!fpg) {
        perror("gnuparse:fopen(r)");
        return(200);    /* errno values are < 133 today */
    }
    while (++lno && fgets(line, MAX_TXT, fpg)) {
        line[strlen(line)-1] = 0;                   /* stomp newline */
        fprintf(stdout, "Line '%s'\n", line);
        if (1 == lno && strcmp(line,
            "#search configuration file -- this line is mandatory")) {
            msg("Config file %s missing mandatory first line", 3, gfile);
            return(201);
        } else if (1 == lno) {
            continue;                               /* do nothing */
        } else if (line[0] == '#') {
            continue;                               /* do nothing */
        } else if (!strncmp(line, "CxR:A=", 6)) {
            ncs = sscanf(line, "CxR:A=%dx%d:%d",
                &gpfp->ncols, &gpfp->nrows, &gpfp->asqr);
            if (3 == ncs) continue;
            return(puke(lno, "Line %d did not parse properly:", line, 202));
        } else {
            return(puke(lno, "Line %d is beyond the pale:", line, 254));
        }
    }
    gpfp->gcfile = malloc(strlen(gfile) + 2);
    if (!gpfp->gcfile) { perror("gnuparse:malloc"); return(253); }
    strcpy(gpfp->gcfile, gfile);
    return(0);
}

/* entry from gnuconfile(), returning
 *   -1 if a template was generated
 *    0 if a file was found and correctly parsed
 *    1 if there was a parsing error
 */
int gnuedits(char *garg, gpconf *gpfp)
{
    int staterr = 0;
    if (1 == gnuexists(garg, &staterr)) {
        msg("Found gnuplot config file '%s'", 2, garg);
        return(gnuparse(garg, gpfp));
    } else {
        if (ENOENT != staterr) return(staterr);
    }
    msg("Creating gnuplot config file '%s'", 2, garg);
    return(create_gnuconf(garg, gpfp));
}

#endif /* BIGGER */
/*
 * eof vim:nospell
 */
