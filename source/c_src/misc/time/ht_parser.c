/*
 * $Id: ht_parser.c 2739 2019-04-12 19:30:02Z barrettj $
 *
 * Support for command line arguments of hops_time
 */

#include "hops_time.h"

/*
 * Some globals
 */
int	    echo = 0;		/* echo query time, joined with replacement */
int	    replace = 0;	/* replace query time in input string */
int	    statf = 0;		/* stat the file for the time */
int	    qtype = 0;		/* asserted query type (UNIX clock) */
char	    *query_arg = "Kdefault";
int	    rtype = 0;		/* asserted report type (UNIX clock) */
char	    *report_arg = "Kdefault";
char	    *joiner = " == ";	/* joiner string */
int         gibber = 0;         /* let gibberish be nonfatal */

/*
 * basic command line help:  -h o option
 */
static void option_help(void)
{
    printf("where the options are:\n"
	"  -v            verbose, may be repeated for more\n"
	"  -h o|f|d|e|a  help on options/formats/details/examples/all\n"
	"  -o[=]<string> set origin for relative time references\n"
	"  -d U<string>  provide rel. times (Unit U) from this origin\n"
	"  -i <type>     specify input type (bypasses heuristics)\n"
	"  -q <type>     specify output type for time conversion\n"
        "  -a            provides the time in all known formats\n"
	"  -f            query time is stat.mtime of files\n"
	"  -e            echo query argument\n"
	"  -g            let gibberish be non fatal\n"
        "  -t <time>     report the internal time classification\n"
	"  -r            replace query with conversion and echo\n"
	"  -j <string>   supply an alternate join string ( == )\n"
	"\n"
	"This tool converts time references from one format\n"
	"to another.  The time(s) are supplied as command-line\n"
	"arguments, or else from the first item on every line\n"
	"of arguments discovered to be a file (use STDIN for\n"
	"the standard input), or the file modification time if\n"
	"the -f option was used.  The -e, -r and -j options modify\n"
	"the basic behavior of converting the time query into\n"
	"the format requested by the -q option.\n"
	"\n"
	"Environment variables affect some answers:\n"
	"  HOPS_VDIF_EPOCH    which semester for Vdif origin (0..63)\n"
	"  HOPS_JULIAN_EPOCH  affects treatment of julian times:\n"
	"    GPS -- julian times tied to GPS epoch (default)\n"
	"    UTC -- julian times tied to UTC calendar\n"
	"\n"
    );
}

/*
 * help with the various time formats: -h f option
 * this should be coordinated with Ht_rtype_to_style()
 */
static void format_help(void)
{
    Ht_format_help();
}

static void chatty_help(void)
{
    printf(
        "The -a option is most useful with -v to get the types.\n"
        "Use -- to separate the time arguments from the - arguments.\n"
        "This is particularly useful when you want, e.g. -2d (two days ago).\n"
        "Or you can send it in via the standard input:\n"
        "$ hops_time  -qR -ds20130801_000000 -- -28d\n"
        "+71629.344216\n"
        "$ echo -28d | hops_time  -qR -ds20130801_000000 STDIN\n"
        "+71745.607701\n"
        "\n"
    );
    printf(
        "The -o option may include an = to bypass the heuristics\n"
        "to convert the argument to a GPS time.  Normally, the current\n"
        "time is used for the time origin.\n"
        "\n"
    );
    printf(
        "Relative times use a unit U which can be any of:\n"
        "  u   microseconds\n"
        "  s   seconds\n"
        "  ms  milliseconds\n"
        "  m   minutes\n"
        "  h   hours\n"
        "  d   days\n"
        "  i   sideral month (27.32.. days)\n"
        "  n   synodic month (29.53.. days)\n"
        "  y   year (365.25 days)\n"
    );
    printf(
        "The code is most robust in the GPS epoch (since 1980),\n"
        "but it does (sometimes) work fine at earlier times:\n"
        "  $ hops_time -qG 1980/006.00000000\n"
        "  0.000000\n"
        "  $ hops_time -qM 1970/001.00000000\n"
        "  40586.9998958334\n"
        "  $ hops_time -qK 1970/001.00000000\n"
        "  0\n"
        "\n"
        "The big problem here is the heuristics to figure out how\n"
        "to convert some random expression into a time are only tuned\n"
        "up to the late 90's and early 21st century.\n"
    );
}

/* ./hops_time -d s2010/243.63694048 -v -a 2010/243.00000000 */
static void example_help(void)
{
    printf(
        "So, for example...\n"
        "\n"
        "  $ hops_time -qM 2010/243.00000000\n"
        "  55439.0001736111  # plus some leap seconds\n"
        "  $ HOPS_JULIAN_EPOCH=UTC hops_time -qM 2010/243.00000000\n"
        "  55439.0000000000  # exactly\n"
        "\n"
        "  $ hops_time -d d56500.9998958334 -qR 56510.9998958334\n"
        "  +10.000000        # is 10 day from then\n"
        "  $ hops_time -o 56533.8223736817 -qM +10d\n"
        "  56543.8223736817  # 10 days from then\n"
        "\n"
    );
}

static void all_help(void)
{
    option_help();
    format_help();
    chatty_help();
    example_help();
}


/*
 * Provide various forms of help
 * Returns 1 (this is good command help) otherwise 0.
 */
int dump_formats(char *oa)
{
    switch (*oa) {
    case 'o': option_help();	break;
    case 'f': format_help();	break;
    case 'd': chatty_help();	break;
    case 'e': example_help();	break;
    case 'a': all_help();	break;
    default: return(fprintf(stderr, "no help on %s\n", oa),0);
    }
    return(1);
}

int set_origin(char *arg)
{
    while (isspace(*arg)) arg++;
    if (*arg == '+' || *arg == '-') return(1);
    Ht_gps_origin = (*arg == '=')
		  ? atof(++arg)
		  : Ht_string_to_GPS(arg, 0);
    if (verb>1)
    {
#ifdef USE_LONG_DOUBLE
        fprintf(stderr, "Ht_gps_origin set to %Lf\n", Ht_gps_origin);
#else
        fprintf(stderr, "Ht_gps_origin set to %lf\n", Ht_gps_origin);
#endif
    }
    return(0);
}
int delta_setup(char *arg)
{
    return(Ht_delta_setup(arg, arg+1));
}
int input_setup(char *oa)
{
    qtype = Ht_qtype_to_style(oa);
    query_arg = oa;
    if (qtype == Ht_DS_NUMB)
	return(fprintf(stderr, "Unknown query type %s\n", oa));
    if (verb>1) fprintf(stderr, "qtype now %d\n", qtype);
    return(0);
}
int query_setup(char *oa)
{
    rtype = Ht_rtype_to_style(oa);
    report_arg = oa;
    if (rtype == Ht_DS_NUMB)
	return(fprintf(stderr, "Unknown report type %s\n", oa));
    if (verb>1) fprintf(stderr, "rtype now %d\n", rtype);
    return(0);
}

/*
 * eof
 */
