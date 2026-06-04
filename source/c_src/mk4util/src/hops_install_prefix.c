/************************************************************************/
/*                                                                      */
/* Resolve the HOPS install prefix at runtime from the on-disk location */
/* of this shared library (libmk4util.so), via dladdr(). This keeps the */
/* absolute install path out of the compiled binary (needed for         */
/* reproducible builds) and makes the install relocatable. It is the C  */
/* analogue of the C++ tree's                                           */
/* MHO_DirectoryInterface::GetHopsInstallPrefix().                      */
/* HOPS3 programs use it to locate their installed resources            */
/* (e.g. share/vhelp, share/text) relative to their own install.        */
/*                                                                      */
/*      Inputs:         (none)                                          */
/*                                                                      */
/*      Output:         pointer to a static buffer holding <prefix>,    */
/*                      or NULL if the prefix could not be determined.  */
/*                      The buffer is overwritten on each call.         */
/*                                                                      */
/************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE   /* for dladdr() */
#endif
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "mk4_util.h"

const char *hops_install_prefix(void)
    {
    static char prefixbuf[1024];
    Dl_info info;
    char *resolved, *p;

                                        /* locate the shared library that */
                                        /* contains this function */
    if (!dladdr((void *)hops_install_prefix, &info) || info.dli_fname == NULL)
        return NULL;

                                        /* extract <prefix>/lib/libmk4util.so */
    resolved = realpath(info.dli_fname, NULL);
    if (resolved == NULL)
        return NULL;

    p = strrchr(resolved, '/');         /* strip /libmk4util.so */
    if (p != NULL)
        {
        *p = '\0';
        p = strrchr(resolved, '/');     /* strip /lib */
        if (p != NULL)
            {
            *p = '\0';                  /* now <prefix> */
            snprintf(prefixbuf, sizeof(prefixbuf), "%s", resolved);
            free(resolved);
            return prefixbuf;
            }
        }
    free(resolved);
    return NULL;
    }
