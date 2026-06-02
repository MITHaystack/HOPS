/*
 * This is called to make a montage of PDFs produced
 * by gnusrchplot().  The code is similar to cohfit:exam_montage().
 * We use a very sloppy upper bound on the command to be executed.
 */
#include "search.h"
#if BIGGER

/* a helper used both for gnuplots and for montage */
void monty_runit(char *cmd)
{
    int rc = system(cmd);
    msg("Ran system('%s') = %d", 1, cmd, rc);
    if (rc) {
        msg("system(%s) failed (%d)", 3, cmd, rc);
        perror("monty_runit:system");
    } else {
        msg("system(%s) successful", 0, cmd);
    }
    free(cmd);
}

/* for readability below */
#define MONTY_RETURN(R,C,S) do {\
        msg("Unable to montage: need %d only have %d chars", 3, R, C);\
        msg("CMD so far: %s", 3, S); return; } while(0)

void search_montage(gpconf *gpfp)
{
    int patlen = gpfp->patlen, nc, pp;
    int cmdlen = (gpfp->npdfs+1)*(patlen + 16) + strlen(MONTAGE) + MAX_TXT;
    char *montyc = malloc(cmdlen), *mtpdfo = malloc(patlen + 32), *pc;
    if (!montyc) { perror("search_montage:malloc:1"); return; }
    if (!mtpdfo) { perror("search_montage:malloc:2"); return; }
    msg("Making the montage", 2);
    pc = strchr(gpfp->gplatt, '%');         /* expect -%<integer>d */
    *--pc = 0;
    /* create the output filename */
    nc = snprintf(mtpdfo, patlen + 32,
        " %s-%d.montage.pdf", gpfp->gplatt, gpfp->npdfs + 1);
    *pc = '-';                              /* restore it to be clean */
    if (nc > patlen + 32) MONTY_RETURN(nc, patlen+32, mtpdfo);
    msg("montage output is %s", 1, mtpdfo);

    /* start with the basic montage command */
    nc = snprintf(montyc, cmdlen,
        "%s -tile %dx%d -geometry +2+2 -density %d ",
        MONTAGE, gpfp->ncols, gpfp->nrows, gpfp->density);
    if (--nc >= cmdlen) MONTY_RETURN(nc, cmdlen, montyc);

    /* pile on the file names */
    for (pp = 0; pp < gpfp->npdfs; pp++) {
        msg("adding #%d %s (%p)", 1, pp, gpfp->gnupdfs[pp], gpfp->gnupdfs[pp]);
        nc += snprintf(montyc + nc, strlen(gpfp->gnupdfs[pp]) + 2,
            " %s ", gpfp->gnupdfs[pp]);
        if (--nc >= cmdlen) MONTY_RETURN(nc, cmdlen, montyc);
        msg("%s %d == %d?", 1, montyc, nc, strlen(montyc));
    }

    /* and finally add the output file */
    nc += snprintf(montyc + nc, patlen + 16, "%s", mtpdfo);
    if (nc >= cmdlen) MONTY_RETURN(nc, cmdlen, montyc);
    msg("Montage command len %d < limit %d is:", 1, nc, cmdlen);
    msg("Final montage file name: %s", 2, mtpdfo);
    free(mtpdfo);
    monty_runit(montyc);
}

#endif /* BIGGER */
/*
 * eof vim:nospell
 */
