/*
 * $Id: hops_time.c 2739 2019-04-12 19:30:02Z barrettj $
 *
 * Main routine for yet another swiss-army knife time utility.
 */

#include <unistd.h>

#include "hops_time.h"

/*
 * Global options
 */
int	    verb = 0;
int	    all = 0;
FILE	    *ofp = 0;

/*
 * Boilerplate
 */
static int cmdhelp = 0;
static int usage(char *name)
{
    printf("Usage: %s [options] [times]\n", name);
    dump_formats("o");
    return(cmdhelp = 1);
}
static int version(char **argv)
{
    if (!argv[1]) return(0);
    if (!strcmp(argv[1], "--help"))    return(usage(argv[0]));
    if ( strcmp(argv[1], "--version")) return(0);
    printf(__FILE__ "\t[" __DATE__ " " __TIME__ "]\n");
    return(cmdhelp = 1);
}
static int options(int argc, char **argv)
{
    int	    c;
    if (version(argv)) return(1);
    while ((c = getopt(argc, argv, "vh:o:d:i:q:fegrj:at:")) != -1) switch(c) {
    case 'v': verb++;						break;
    case 'h': return((cmdhelp = dump_formats(optarg)));
    case 'o': if (set_origin(optarg)) return(2);		break;
    case 'd': if (delta_setup(optarg)) return(3);		break;
    case 'i': if (input_setup(optarg)) return(4);		break;
    case 'q': if (query_setup(optarg)) return(5);		break;
    case 'f': statf++;						break;
    case 'e': echo++;						break;
    case 'g': gibber++;						break;
    case 'r': replace++;					break;
    case 'j': joiner = optarg;					break;
    case 't': c = Ht_string_type(optarg); return(printf("%08x\n", c));
    case 'a': all++;						break;
    default:							return(7);
    }
    return(0);
}
static int
cmdline(int *argc, char ***argv)
{
    int	    x = options(*argc, *argv);
    *argc -= optind;
    *argv += optind;
    return(x);
}

/*
 * Main Entry.
 */
int main(int argc, char **argv)
{
    int		errs = 0;

    /* no need to do otherwise */
    ofp = stdout;

    /* basic command line parsing */
    if (cmdline(&argc, &argv)) return(!cmdhelp);

    /* for clarity */
    if (verb>1 && strcmp(query_arg, "Kdefault"))
        fprintf(stderr, "input query type is UNIX (K) by default\n");
    if (verb>1 && strcmp(report_arg, "Kdefault"))
        fprintf(stderr, "output report type is UNIX (K) by default\n");

    /* process the remaining arguments */
    if (all)
	while (argc-- && !errs) errs += all_arguments(*argv++);
    else
	while (argc-- && !errs) errs += time_argument(*argv++);

    fclose(ofp);
    return(- errs);
}

/*
 * eof
 */
