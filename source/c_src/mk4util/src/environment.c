/************************************************************************/
/*                                                                      */
/* Mechanism for modifying the behaviour of various programs in a       */
/* uniform way.  In each program, one uses an extern declaration to     */
/* access the various environment variables the user may have set.      */
/* This routine should be called early by any program whose behaviour   */
/* is under environment variable control.  Any variable which has       */
/* potential or actual application to more than one program should be   */
/* processed here.                                                      */
/*                                                                      */
/*      Inputs:                                                         */
/*                                                                      */
/*      Output:                                                         */
/*                                                                      */
/* Created ?? by CJL                                                    */
/* Modified for UTIL library use 931018 by CJL                          */
/* Added a few Mk4-specific directories 930323 CJL                      */
/*                                                                      */
/************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE   /* for dladdr() */
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#ifdef HOPS_VEX_TEXT_SHARE_DIR
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif /* HOPS_VEX_TEXT_SHARE_DIR */
#include "mk4_util.h"

                                        /* Declare these global, to be */
                                        /* initialized here, but used */
                                        /* throughout program */
char datadir[200];
char scheddir[200];
char afiledir[200];
char textdir[1000];
char sysvexdir[200];
char taskdir[200];
char bindir[200];
char tmpdir[200];

// /correlator no longer exists, thus this entire file is full of junk.
// PFORMAT_TEXT_DIR does not appear to have every been defined in HOPS3
// Here at least we set things so that fourfit will run without an install
// or later, after an install.  If the Makefile supplies both variables,
// then pre-install, the vex/text source directory can supply the file
// and after the install, the installed destination should be used.
static char *textdef(void)
{
#ifdef HOPS_VEX_TEXT_SRC_DIR
    /* pre-install: use the in-tree vex/text source directory */
    static char *textdefault = HOPS_VEX_TEXT_SRC_DIR;
#else /* HOPS_VEX_TEXT_SRC_DIR */
    /* Resolve <install_prefix>/share/text at runtime from the on-disk location
       of this shared library (libmk4util.so), via dladdr(). This keeps the
       absolute install path out of the binary (reproducible builds) and makes the
       install relocatable. This is the C analogue of the C++ tree's
       MHO_DirectoryInterface::GetHopsInstallPrefix(). */
    static char textbuf[1024];
    char *textdefault = "/correlator/prog/text"; /* legacy fallback */
    Dl_info info;
    if (dladdr((void *)textdef, &info) && info.dli_fname != NULL)
    {
        char *resolved = realpath(info.dli_fname, NULL); /* <prefix>/lib/libmk4util.so */
        if (resolved != NULL)
        {
            char *p = strrchr(resolved, '/'); /* strip /libmk4util.so */
            if (p != NULL)
            {
                *p = '\0';
                p = strrchr(resolved, '/');   /* strip /lib */
                if (p != NULL)
                {
                    *p = '\0';                /* now <prefix> */
                    snprintf(textbuf, sizeof(textbuf), "%s/share/text", resolved);
                    textdefault = textbuf;
                }
            }
            free(resolved);
        }
    }
#endif /* HOPS_VEX_TEXT_SRC_DIR */

#ifdef HOPS_VEX_TEXT_SHARE_DIR
    {
        struct stat sb;
        /* see if the directory exists, if so, use it */
        if (!stat(HOPS_VEX_TEXT_SHARE_DIR, &sb))
            return(HOPS_VEX_TEXT_SHARE_DIR);
    }
#endif /* HOPS_VEX_TEXT_SHARE_DIR */
    return(textdefault);
}

void
environment(void)
    {                                   /* Default values */
    static char *datadef = "/correlator/data";
    static char *scheddef = "/correlator/schedules";
    static char *afiledef = "/correlator/afiles";
    static char *sysvexdef = "/correlator/sysvex";
    static char *taskdef = "/correlator/task";
    static char *bindef = "/correlator/prog/bin/hppa";
    static char *tmpdef = "/correlator/tmp";
    char *dummy;

    if ((dummy = getenv ("DATADIR")) != NULL) strcpy (datadir, dummy);
    else if ((dummy = getenv ("CORDATA")) != NULL) strcpy (datadir, dummy);
    else strcpy (datadir, datadef);

    if ((dummy = getenv ("SCHEDDIR")) != NULL) strcpy (scheddir, dummy);
    else strcpy (scheddir, scheddef);

    if ((dummy = getenv ("AFILEDIR")) != NULL) strcpy (afiledir, dummy);
    else strcpy (afiledir, afiledef);

    // introducing a better name and a testing fallback to vex source
    if ((dummy = getenv ("HOPS_VEX_DIR")) != NULL) strcpy (textdir, dummy);
    else if ((dummy = getenv ("TEXT")) != NULL) strcpy (textdir, dummy);
    else strcpy (textdir, textdef());

    if ((dummy = getenv ("SYSVEX")) != NULL) strcpy (sysvexdir, dummy);
    else strcpy (sysvexdir, sysvexdef);

    if ((dummy = getenv ("TASK")) != NULL) strcpy (taskdir, dummy);
    else strcpy (taskdir, taskdef);

    if ((dummy = getenv ("BIN")) != NULL) strcpy (bindir, dummy);
    else strcpy (bindir, bindef);

    if ((dummy = getenv ("TMP")) != NULL) strcpy (tmpdir, dummy);
    else strcpy (tmpdir, tmpdef);
    return;
    }
