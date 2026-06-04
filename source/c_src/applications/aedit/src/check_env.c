/************************************************************************/
/*									*/
/* At present, set only the extern which specifies the aedit help file	*/
/* directory								*/
/*									*/
/*	Inputs:								*/
/*									*/
/*	Output:								*/
/*									*/
/* Created ?? by CJL							*/
/*									*/
/************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mk4_util.h"
					/* Declare this global, to be */
					/* initialized here, but used */
					/* throughout program */
char ahelpdir[200];

void check_env(void)
    {					/* Legacy default value */
    static char *helpdef = "/usr/local/lib/ahelp";
    char *dummy;
    const char *prefix;

					/* User override; otherwise self-locate */
					/* <prefix>/share/vhelp/aedit relative to the */
					/* install (see hops_install_prefix()), with a */
					/* legacy fallback */
    if ((dummy = getenv ("AHELP")) != NULL) strcpy (ahelpdir, dummy);
    else if ((prefix = hops_install_prefix()) != NULL)
	snprintf (ahelpdir, sizeof(ahelpdir), "%s/share/vhelp/aedit", prefix);
    else strcpy (ahelpdir, helpdef);

    return;
    }
