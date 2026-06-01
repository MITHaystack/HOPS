/*
 * includes for search
 */
#ifndef __search_h__
#define __search_h__

#define FALSE 0
#define TRUE  1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "adata.h"
#include "mk4_util.h"
#include "mk4_afio.h"

#if BIGGER
#define MAX_BNO 45                      /* 10 telescopes */
#define MAX_NRATE 1500                  /* generous maximums */
#define MAX_NDELAY 1500
#else /* BIGGER */
#define MAX_BNO 66                      /* 12 telescopes */
#define MAX_NRATE 300                   /* Reasonable maximums */
#define MAX_NDELAY 50
#endif /* BIGGER */

#if (MAX_BNO * MAX_NRATE * MAX_NDELAY * 4 * 2) > 4294967296/2
# error "MAX_BNO * MAX_NRATE * MAX_NDELAY is too big"
#endif

typedef struct
    {
    int         order;
    int         lastorder;
    int         keyval;
    fringesum   fdata;
    } avg_data;

typedef struct srchsummary
    {
    fringesum   *datum;                 /* Data structure ptr for this scan */
    int         nd;
    fringesum   *darray[MAX_NRATE * MAX_NDELAY];
    int         nrate;
    int         ndelay;
    float       min_rate;
    float       max_rate;
    float       min_delay;
    float       max_delay;
    float       snr[MAX_NRATE][MAX_NDELAY];
    } srchsum;

extern int space;
extern int optind;
extern int read_data (avg_data** data, char* filename, int* navg);
extern int sort_data (avg_data* data, int navg);
extern void clear_srchdata (srchsum srchdata[]);
extern int fit_peaks (srchsum *srchdata);
extern int write_srchdata (srchsum *srchdata, FILE* fpout);

#include "gnearch.h"

extern int parse_cmdline (int, char **, FILE **, gpconf *);
extern void plot_srchdata (srchsum srchdata[], int square);
extern int fill_grids_orig (srchsum *srchdata);
extern int fill_grids (srchsum *srchdata);

#endif /* __search_h__ */
/*
 * eof vim: nospell
 */
