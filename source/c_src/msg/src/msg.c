#include "msg.h"

void set_progname(char* local_progname)
{
    int i, len;
    for(i=0;i<256;i++){progname[i] = '\0';}
    len = strlen(local_progname);
    if(len < 256)
    {
        strncpy(progname, local_progname, len);
    }
    else 
    {
        //truncate to 256
        strncpy(progname, local_progname, len);
    }
}


void set_msglev(int lev)
{
    msglev = lev;
}

					/* To understand this, check stdarg man pages */
void msg (const char *string, 
          int level, 
          ...)	

    {
    // extern int msglev;
    // extern char progname[];
    va_list args;

    if (level >= msglev)
        {
        va_start (args, level);
        if (*progname) fprintf (stderr, "%s: ", progname);
        vfprintf (stderr, string, args);
        putc ('\n', stderr);
        va_end (args);
        fflush (stderr);
        }
    }