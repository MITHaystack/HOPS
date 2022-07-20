/*
 * Simple program to generate tai-utc.dat from CDFLeapSeconds.txt
 * as a backup to the NAVY (maia) going offline.  It takes precisely
 * two arguments (cdf version and output to maia format).
 *
 * gbc Dec 2019
 */

#include <stdio.h>
#include <string.h>

/* poor man's jd calculator just to keep it local */
/* centuries are generally not leap years, but 2000 is */
double hack_jd(int year, int imon, int dom)
{
    static int dbmn[12] = { 0, 31, 31+28, 31+28+31, 31+28+31+30,
        31+28+31+30+31, 31+28+31+30+31+30, 31+28+31+30+31+30+31,
        31+28+31+30+31+30+31+31, 31+28+31+30+31+30+31+31+30,
        31+28+31+30+31+30+31+31+30+31, 31+28+31+30+31+30+31+31+30+31+30 };
    static int dbml[12] = { 0, 31, 31+29, 31+29+31, 31+29+31+30,
        31+29+31+30+31, 31+29+31+30+31+30, 31+29+31+30+31+30+31,
        31+29+31+30+31+30+31+31, 31+29+31+30+31+30+31+31+30,
        31+29+31+30+31+30+31+31+30+31, 31+29+31+30+31+30+31+31+30+31+30 };
    double jd = 2437300.5;  /* 1961 JAN 1, from the tai-utc.dat table */
    int *dbm = (year % 4 == 0) ? dbml : dbmn;
    jd += (double)dbm[imon] + (double)(dom-1);
    jd += 365.0*(double)(year - 1961);
    jd += 1.0*(double)((year - 1961) / 4);
    /* which is good until 2100--not my problem */
    return(jd);
}

#define IFMT \
"%d%d%d%lf%lf%lf"
#define OFMT1 \
" %4d %3s %2d =JD %7.1lf  TAI-UTC=%12.7lf S + (MJD - %#6.0lf) X %8.6lf S\n"
#define OFMT2 \
" %4d %3s %2d =JD %7.1lf  TAI-UTC=%12.7lf S + (MJD - %#6.0lf) X %9.7lfS\n"
#define OFMT3 \
" %4d %3s %2d =JD %7.1lf  TAI-UTC=  %-10.1lf S + (MJD - %#5.0lf) X %-9.1lfS\n"

int convert(char *in, char *ou)
{
    static char line[200];
    static char mtb[12][5] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                               "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
    FILE *fin, *fou;
    char *fmt;
    int year, dom, imon, nn;
    double jd, taiutc, mjdo, srate;

    if (!(fin = fopen(in, "r"))) { perror("opening input file"); return(1); }
    if (!(fou = fopen(ou, "w"))) { perror("opening output file"); return(2); }

    while (fgets(line, sizeof(line), fin)) {
        if (line[0] == ';') continue;       /* skip comments */
        nn = sscanf(line, IFMT, &year, &imon, &dom, &taiutc, &mjdo, &srate);
        if (nn != 6) {                      /* verify full read */
            fprintf(stderr, "partical scan %d != 6\n", nn);
            return(12);
        }
        if (imon > 12 || imon < 1) {        /* check on month */
            fprintf(stderr, "illegal month %d\n", imon);
            return(11);
        }
        imon--;
        /* address some fortran formatting peculiarities */
        if (year == 1960) continue;
        if (year == 1962 || year == 1963) fmt = OFMT2;
        else if (year < 1970) fmt = OFMT1;
        else fmt = OFMT3;
        if (year > 1970) mjdo = 41317.;
        jd = hack_jd(year, imon, dom);
        sprintf(line, fmt, year, mtb[imon], dom, jd, taiutc, mjdo, srate);
        fputs(line, fou);
    }

    if (fclose(fin)) { perror("closing input"); return(3); }
    if (fclose(fou)) { perror("closing output"); return(4); }

    return(0);
}

int main(int argc, char **argv)
{
    char *input, *output;

    if (argc > 1 && !strcmp(argv[1], "--help")) {
        fprintf(stderr, "Usage: %s cdf-file output-file\n", argv[0]);
        return(0);
    }
    if (argc > 1 && !strcmp(argv[1], "--version")) {
        fprintf(stderr, "this is the only version\n", argv[0]);
        return(0);
    }
    if (argc != 3) {
        fprintf(stderr, "Usage: %s cdf-file output-file\n", argv[0]);
        return(1);
    }

    input = argv[1];
    output = argv[2];
    return(convert(input, output));
}

/*
 * eof
 */
