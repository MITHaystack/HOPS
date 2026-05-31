/************************************************************************/
/*                                                                      */
/* Deals with everything on the command line.                           */
/*                                                                      */
/*      Inputs:         argc, argv      command line args               */
/*                                                                      */
/*      Output:         fpout           output file pointer             */
/*                      display         For graphical output            */
/*                      return value    0=OK, 1=BAD                     */
/*                                                                      */
/* Created October 5 1995 by CJL                                        */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cpgplot.h"
#include "search.h"

#include <getopt.h>

#if BIGGER
/* for readability in -x and -d argument parsing below */
#define XD_BARFAGE do {\
    msg("Only one of -x and -d may be given", 3);\
    return(205);\
    } while(0)

/* this routine modifies the -d argument if /pdf is found
 * so that pgplot only sees the part it understands */
char *pdfixer(char *optarg, char **display)
    {
        char *df = NULL, *slash, *co;
        co = malloc(strlen(optarg) + MAX_TXT);
        if (!co) { perror("pdfixer:malloc1"); return(NULL); }
        strcpy(co, optarg);
        msg("Convert optarg '%s' to '%s'", 0, optarg, (*display = co));
        slash = strrchr(co, '/');
        /* are we dealing with .../something/pdf */
        if (slash[1] == 'p' && slash[2] == 'd' && slash[3] == 'f')
            {
            *slash = 0; /* optarg name/ps/pdf -> name/ps */
            msg("into an optarg %s", 0, co);
            df = malloc(strlen(co) + MAX_TXT);
            if (!df) { perror("pdfixer:malloc2"); free(co); return(NULL); }
            strcpy(df, co);
            slash = strrchr(df, '/');
            msg("slash is %s", 0, slash);
            if (!slash) { free(df); free(co); return(NULL); }
            *slash = 0;
            msg("Output converts to %s", 0, df);
            return(df);
            }
        return(NULL);
    }
#endif /* BIGGER */

int
parse_cmdline (int argc, char **argv, FILE **fpout, int *plot, int *sqp,
    gpconf *gpcp)
    {
#if BIGGER 
#else /* BIGGER */
    static char device[1000];
#endif /* BIGGER */
    static char outfile[1000];
    int ii, c, nxsub = 2, nysub = 2;
    char *display = NULL;
    extern char *optarg;
    extern int optind, msglev;
                                        /* Defaults */
    *fpout = stdout;
    *plot = FALSE;
    *sqp = FALSE;
                                        /* parse command line and read in */
                                        /* filename */
    while ((c = getopt (argc, argv, "d:g:m:o:x")) != -1)
        {
        switch (c)
            {
#if BIGGER
            /* handle -x and -d cases together, similar to cohfit */
            case 'd':   /* wedge in a ps2pdf fix; pdfile is malloc'd and used
                         * by the caller to do the ps to pdf conversion, any
                         * /pdf construct is deleted from optarg */
                if (display) XD_BARFAGE;
                msg("-d option with %s", 0, optarg);
                gpcp->pdfile = pdfixer(optarg, &display);
                if (gpcp->pdfile) msg("PDFile is %s", 0, gpcp->pdfile);
                *plot = TRUE;
            case 'x':
                if (c == 'x' && display) XD_BARFAGE;
                else if (!display) display = "/XW";
                gpcp->devp = malloc((ii = strlen(display)) + MAX_TXT);
                if (!gpcp->devp) { perror("parse-d:malloc"); return(ENOMEM); }
                strncpy(gpcp->devp, display, ii);
                msg("Device is %s, plot = %d", 0, gpcp->devp, gpcp->pdfile);
                *plot = TRUE;
                break;
#else /* BIGGER */
            case 'd':                   /* File away the display string */
                strncpy (device, optarg, sizeof(device));
                *plot = TRUE;
                break;

            case 'x':                   /* short for -d /xw */
                strcpy (device, "/XW");
                *plot = TRUE;
                break;
#endif /* BIGGER */
#if BIGGER
            case 'g':                   /* Specify the gridding */
                if (3 == sscanf(optarg, "%dx%d:%d", &nxsub, &nysub, sqp))
                    {
                    if (getenv("PGPLOT_DEV")) *plot = TRUE;
                    }
                else
                    {
                    nxsub = nysub = 2;
                    *sqp = 0;
                    }
                break;
#else /* BIGGER */
#endif /* BIGGER */
            case 'm':                   /* Verbosity control */
                if (sscanf (optarg, "%d", &msglev) != 1)
                    {
                    msg ("Invalid -m flag argument '%s'", 3, optarg);
                    msg ("Message level remains at %d", 3, msglev);
                    }
                if (msglev > 3) msglev = 3;
                if (msglev < -3) msglev = -3;
                break;

            case 'o':                   /* Override default to stdout */
                strncpy (outfile, optarg, sizeof(outfile));
                if ((*fpout = fopen (outfile, "w")) == NULL)
                    {
                    msg ("Could not open output file '%s'", 3, outfile);
                    return (1);
                    }
                break;

            case '?':
            default:
#if BIGGER
                /* as set in search.c for search */
#else /* BIGGER */
                /* override for help for soirch */
                msg("The -g option is not allowed in soirch", 3);
                msg("(soirch is the original version of search)", 3);
                set_progname("search");
#endif /* BIGGER */
                syntax("search/parse_cmdline.c");
                return (1);
            }
        }
                                        /* Input files on command line are */
                                        /* handled in main routine. */
                                        /* synchronize with gnuplot */
    gpcp->ncols = nxsub;
    gpcp->nrows = nysub;
    gpcp->arat = *sqp;
    gpcp->plot = *plot;
                                        /* Open plot device */
    if (*plot)
        {
#if BIGGER
        if (cpgbeg (0, gpcp->devp, 1, 1) != 1)
            {
            msg ("Could not open pgplot device '%s', abort.", 3, gpcp->devp);
            exit (1);
            }
        cpgsubp (nxsub, nysub);
#else /* BIGGER */
        if (cpgbeg (0, device, 1, 1) != 1)
            {
            msg ("Could not open pgplot device '%s', abort.", 3, device);
            exit (1);
            }
        cpgsubp (2, 2);
#endif /* BIGGER */
        msg("pgplot initialized with %dx%d subplots", 1, nxsub, nysub);
        }

    return (0);
    }

/*
 * eof vim: nospell
 */
