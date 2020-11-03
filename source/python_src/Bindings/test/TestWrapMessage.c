/*
 * Test program for message wrapper
 * (c) Massachusetts Institute of Technology, 2020
 *
 * This program is for initial testing...
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "wrap_msg.h"

#define differ(A,B)   ((A)==(B) ? 0 : 1)

static struct myopts {
    int     verb;       /* canonical verbosity */
    char    *name;      /* effective program name */
    int     skip;       /* automated test */
    int     nada;
} opt = {
    .verb = 0,
    .name = "TestWrapMessage",
    .skip = 0,
    .nada = 0
};
static int cmdhelp = 0;
static int option_checks(void)
{
    /* a place to check for pilot errors */
    if (opt.verb) printf("verbosity is %d, name is %s\n", opt.verb, opt.name);
    return(0);
}
static int usage(char *name)
{
    printf("Usage: %s [options]\n", name);
        printf("\nwhere the options are:\n\n"
        "  -v               verbose, may be repeated for more\n"
        "  -n <string>      name of the test program\n"
        "  -s               skip the automated test\n"
        "\n"
    );
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
    int c;
    if (version(argv)) return(1);
    while ((c = getopt(argc, argv, "vn:s")) != -1)
    switch(c) {
    case 'v': opt.verb++;                       break;
    case 'n': opt.name = optarg;                break;
    case 's': opt.skip++;                       break;
    default: return(1);
    }
    if ((c = option_checks())) return(c);
    return(0);
}
static int cmdline(int *argc, char ***argv)
{
    int     x = options(*argc, *argv);
    *argc -= optind;
    *argv += optind;
    return(x);
}

/* allow the user to try it */
int commandlinetest(void)
{
    char *nm;
    int ml, err = 0;
    set_wrap_progname(opt.name);
    nm = get_wrap_progname();
    if (opt.verb) printf("set progname to %s\n", nm);
    err += strcmp(nm, opt.name);

    set_wrap_msglevel(3 - opt.verb);
    ml = get_wrap_msglevel();
    err += differ( (3 - opt.verb), ml );

    wrap_message(opt.verb, 6, "This is most severe 6");
    wrap_message(opt.verb, 5, "This is less severe 5");
    wrap_message(opt.verb, 4, "This is less severe 4");
    wrap_message(opt.verb, 3, "This is less severe 3");
    wrap_message(opt.verb, 2, "This is less severe 2");
    wrap_message(opt.verb, 1, "This is less severe 1");
    wrap_message(opt.verb, 0, "This is least severe 0");

    return(err);
}

int msgleveltest(int ml)
{
    char key[40], *kp;
    int err = 0, level;
    snprintf(key, sizeof(key), "msglev=%+d", ml);
    set_wrap_progname(key);
    kp = get_wrap_progname();
    err += strcmp(key, kp);

    set_wrap_msglevel(ml);
    level = get_wrap_msglevel();
    if (ml >= -3 && ml <= 3) err += differ(ml, level);
    else if (ml > 3) err += differ(3, level);
    else if (ml < -3) err += differ(-3, level);

    printf("--------------------------------------------- %d\n", opt.verb);
    for (level = -3; level < 4; level++)
        wrap_msg("Testing code level %d", level, level);

    return(err);
}

/* main entry */
int main(int argc, char **argv)
{
    int err = 0;
    int ml;
    if (cmdline(&argc, &argv)) return(!cmdhelp);

    err += commandlinetest();
    if (opt.skip == 0) {
        for (opt.verb = 0; opt.verb < 6; opt.verb++)
            for (ml = -4; ml < 4; ml++) 
                err += msgleveltest(ml);
    }

    return(err);
}

/*
 * eof
 */
