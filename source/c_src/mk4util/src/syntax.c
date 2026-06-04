/************************************************************************/
/*									*/
/* This routine locates the main documentation file for this program	*/
/* (as defined in the external character string "progname"), and digs	*/
/* out the syntax section of that file, printing it on the screen.	*/
/*									*/
/*	Inputs:		progname (by extern)				*/
/*									*/
/*	Output:		screen output					*/
/*									*/
/* Created 13 December 1993 by CJL					*/
/*									*/
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mk4_util.h"
#include "hops_config.h"
#include "msg.h"

#define FALSE 0
#define TRUE  1

#define copyright1 "Copyright (c) 1992-2026 by Haystack Observatory,"
#define copyright2 "Massachusetts Institute of Technology."
#define copyright3 "This is free software; see the source Copyright file"
#define copyright4 "for conditions.  There is NO warranty; not even for"
#define copyright5 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."

void version(char *headurl)
    {
    msg ("Version: hops-%s (svn rev %d)", 3,
        PACKAGE_VERSION_SYNTAX, HOPS_SVN_REV);
    msg ("", 3);
    msg ("%s", 3, copyright1);
    msg ("%s", 3, copyright2);
    msg ("%s", 3, copyright3);
    msg ("%s", 3, copyright4);
    msg ("%s", 3, copyright5);
    msg ("%s", 3, headurl);
    msg ("", 3);
    }

void syntax(char *headurl)
    {
    char *evar, docdir[256], helpfile[512], line[256];
    int len, print;
    FILE *fp;
					/* Start with a blank line for appearance */
    msg ("", 3);
    /* User override of documentation location; */
    /* otherwise self-locate <prefix>/share/vhelp */
    /* relative to the install (see */
    /* hops_install_prefix()), with a legacy fallback */
    if ((evar = getenv ("PROGDOC")) != NULL)
	snprintf (docdir, sizeof(docdir), "%s", evar);
    else
	{
	const char *prefix = hops_install_prefix();
	if (prefix != NULL)
	    snprintf (docdir, sizeof(docdir), "%s/share/vhelp", prefix);
	else
	    snprintf (docdir, sizeof(docdir), "/usr/local/doc");
	}
    sprintf (helpfile, "%s/%s.doc", docdir, progname);
    msg ("Docfile is %s", 0, helpfile);

    if ((fp = fopen (helpfile, "r")) == NULL)
	msg ("Syntax error.  Could not open documentation file, sorry", 3);
    else
	{
					/* Keywords in file must be exactly right */
	print = FALSE;
	while (fgets (line, 250, fp) != NULL)
	    {
	    if (strncmp (line, "OPTION FLAGS:", 13) == 0) break;
	    if (strncmp (line, "SYNTAX:", 7) == 0) print = TRUE;
	    if (print)
		{
					/* Strip newline */
		len = strlen (line);
		line[len-1] = '\0';
		msg ("%s", 3, line);
		}
	    }
        fclose (fp);
	}

    version(headurl);
    return;
    }
