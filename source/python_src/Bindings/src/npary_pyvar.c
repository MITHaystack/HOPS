/*
 * Additional SWIG wrapper demo source
 * (c) Massachusetts Institute of Technology, 2020
 * The contents of the package Copyright statement apply here.
 *
 * This example is fully contrived, but you want such a file when
 * the functions you have (from lower levels) have an interface that
 * is somewhat awkward to pass through the SWIG numpy rules.
 *
 * Note that we don't have any good way of catching malloc errors,
 * so we'll merely hope that an empty response will be enough for
 * the caller to understand there is an issue.  Better code should
 * handle this....
 */

#include "npary_pyvar.h"

void describe_one_vis(MyVis *mv, char **buf)
{
    *buf = malloc(80);
    if (buf) snprintf(*buf, 80, "%10.10s [%g,%g] ... (%d)",
        mv->name, mv->amps[0], mv->phas[0], mv->nvis);
}

void describe_all_vis(MyVis *mv, long num, char **buf)
{
    char *bf1, *bf2;
    describe_one_vis(mv+0, &bf1);
    describe_one_vis(mv+num-1, &bf2);
    *buf = malloc(80*3);
    if (buf && bf1 && bf2)
        snprintf(*buf, 80*3, "%-.40s\n...%d...\n%-.40s", bf1, num, bf2);
    if (bf1) free(bf1);
    if (bf2) free(bf2);
}

/* other support functions */

/*
 * eof
 */
