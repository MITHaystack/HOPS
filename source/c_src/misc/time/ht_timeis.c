/*
 * $Id: ht_timeis.c 2911 2019-12-23 17:24:02Z gbc $
 *
 * Handle a time argument and do the dance.
 */

#include <ctype.h>
#include <string.h>
#include <sys/stat.h>

#include "hops_time.h"

static char *do_timeis_buf = 0;

/*
 * Handler for things that best laundered through UTC.
 * Either do_timeis_buf is loaded with the answer,
 * or the answer is output to ofp.
 */
static void via_UTC(char *arg)
{
    static char	    temp_buf[8999];
    int		    n = strlen(arg) - 1;
    UTime	    utc = Ht_string_to_UTC(arg,0);
    if (arg[n] == '\n') arg[n] = '\0';
    if (replace) {
	snprintf(temp_buf, sizeof(temp_buf), Ht_UTC_to_string(utc, rtype));
	do_timeis_buf = temp_buf;
    } else {
	fprintf(ofp, "%s\n", Ht_UTC_to_string(utc, rtype));
    }
}

/*
 * Handler for things that best laundered through GPS.
 * Either do_timeis_buf is loaded with the answer,
 * or the answer is output to ofp.
 */
static void via_GPS(char *arg)
{
    static char	    temp_buf[8999];
    int		    n = strlen(arg) - 1;
    GTime	    gps = Ht_string_to_GPS(arg,0);
    if (arg[n] == '\n') arg[n] = '\0';
    if (replace) {
	snprintf(temp_buf, sizeof(temp_buf), Ht_GPS_to_string(gps, rtype));
	do_timeis_buf = temp_buf;
    } else {
	fprintf(ofp, "%s\n", Ht_GPS_to_string(gps, rtype));
    }
}

/*
 * If a replacement was called for, merge the argument with do_timeis_buf
 */
static void output_replacement(char *arg)
{
    char *x = do_timeis_buf;
    arg += Ht_string_min_len;
    while (*arg && !isspace(*arg)) ++arg;
    while (x && *x && *x != '\n') x++;
    if (*x == '\n') *x = 0;
    /* output the replacement */
    fputs(do_timeis_buf, ofp);
    fputs(" ", ofp);
    /* output the rest of the input */
    fputs(arg, ofp);
    fputs("\n", ofp);
}

/*
 * Check if this is something we are going to make sense of...
 */
static int gibberish(char *when)
{
    static char last[200] = "+1y";
    int	t;
    if (when) {
	t = Ht_string_type(when);
        if (gibber && (t == Ht_STR_NTYP))
            t = Ht_string_type(strcpy(when,last));
	if (t == Ht_STR_NTYP) {
	    fprintf(stderr, "Invalid time request: %s [%d]\n", when, t);
	    return(1);
	}
    } else {
	fprintf(stderr, "Null time request.\n");
	return(1);
    }
    // strncpy(last, when, sizeof(last));
    return(0);
}

/*
 * Echo the argument followed by the joiner.  The converted value follows.
 */
static void echo_prefix(char *when)
{
    int	n = strlen(when) - 1;
    fprintf(ofp, "%s%s", when, joiner);
}

/*
 * Convert the time, returning nonzero on errors.
 * Here we funnel the input string to something that
 * works through either GTime or UTime which saves on
 * some silly back and forth conversions....
 */
static int do_time_is(char *ta)
{
    if (verb>2) fprintf(stderr, "do_time_is (%d): %s\n", rtype, ta);
    if (gibberish(ta)) return(1);
    if (echo) echo_prefix(ta);
    switch (*report_arg) {
    /* calendar types */
    case 'B': case 'D': case 'E': case 'H': case 'I': case 'J':
    case 'K': case 'M': case 'N': case 'P': case 'Q':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'F':
	via_UTC(ta);
	break;
    /* epoch types */
    case 'C': case 'G': case 'R': case 'S':
	via_GPS(ta);
	break;
    default:
	return(fprintf(stderr,
	    "Unknown report type %s style %d\n", report_arg, rtype));
    }
    if (replace) output_replacement(ta);
    return(0);
}

/*
 * Get the mtime from a file.
 */
static int statfile(char *ta)
{
    struct stat	    stb;
    UTime	    utc;
    static char	    mtime[50];
    if (verb>2) fprintf(stderr, "SF: %s\n", ta);
    if (stat(ta, &stb)) return(strcmp(ta, "STDIN"));
    if (verb>2) fprintf(stderr, "SF: %s exists\n", ta);
    if (statf) {
	utc.tv_sec = stb.st_mtime;
	utc.tv_usec = 500000;	    /* in the middle */
	strcpy(mtime, Ht_UTC_to_string(utc, Ht_DS_SECS));
	if (verb>2) fprintf(stderr, "STAT: %s\n", mtime);
	if (echo) echo_prefix(ta);
	return(do_time_is(mtime) || statf);
    }
    return(0);
}

/*
 * Arrange to read queries from a file, or from a stat of the file.
 */
static int file_input(char *ta)
{
    static char	    line[4096];
    char	    *s;
    FILE	    *fin;
    if (verb>2) fprintf(stderr, "Arg: file_input(%s)\n", ta);
    if (statfile(ta)) return(statf);    /* not a file or handled */
    fin = (strcmp(ta, "STDIN")) ? fopen(ta, "r") : stdin;
    if (!fin) return(1);
    if (verb>2) fprintf(stderr, "Arg: file_input(%s)[%d]\n", ta, fileno(fin));

    while (fgets(line, sizeof(line), fin)) {
	if (line[0] == '#') {
	    fputs(line, ofp);	/* just echo comments */
	    continue;
	}
	line[strlen(line)-1] = '\0';	/* nuke newline */
	if (verb>3) fprintf(stderr, "While: %s\n", line);
	if (do_time_is(line)) break;
    }

    fclose(fin);
    return(2);
}

/*
 * Do the file processing if it's a file, otherwise do the dance.
 */
int time_argument(char *ta)
{
    if (verb>2) fprintf(stderr, "Arg: %s\n", ta);
    if (!file_input(ta)) return(do_time_is(ta));
    return(0);
}

/*
 * Dump out the time in "all" formats
 */
int all_arguments(char *ta)
{
    static char	*rtys[] = {
	"B", "Burst",
//	"C", "SPICE",
	"D", "Date", "DateA",
	"E", "Maestro__", "Maestro::", "Maestro..",
	"F", "File__", "File___",
	"G", "GPS",
	"H", "HOPS",
	"I", "ISOusz", "ISOmsz", "ISOsz", "ISOusnz", "ISOmsnz", "ISOsnz",
	"J", "Julian",
	"K", "Clock",
	"M", "MJD",
	"N", "Plan",
	"P", "PIGMI", "Plog",
	"Q", "YDF",
	"R",
	"S", "MET",
	"U", "UNIX",
	"V", "SVNnz", "SVNz", "SVNusnz", "SVNus",
	"W", "Weeks",
	"X", "Vex", "Vdif", "Vrdbe",
	"Y", "YDS"
    };
    static int nrtys = sizeof(rtys)/sizeof(char*);
    int	ii;
    if (verb>2) fprintf(stderr, "All: %s\n", ta);
    for (ii = 0; ii < nrtys; ii++) {
	if (query_setup(rtys[ii])) return(1);
	if (verb>0) fprintf(ofp, "%10.10s ", rtys[ii]);
	if (do_time_is(ta)) return(2);
    }
    return(0);
}

/*
 *
 */
