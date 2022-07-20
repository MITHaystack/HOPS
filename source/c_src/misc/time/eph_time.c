/*
 * $Id: eph_time.c 1231 2008-02-20 01:01:12Z gbc $
 *
 * Routines to support ephemeris time calculation relative to J2000
 *
 * TDT - GPS == 51.184 s (TDT_GPS_DELTA)
 * TAI - GPS == 19.000 s (TAI_GPS_DELTA)
 * TDB - TDT =  K * sin(E(t))
 *              t is secs from J2000 epoch (TDB)
 * E(t) = M + EB * sin(M)
 * M(t) = M0 + M1 * t
 */

#include <math.h>
#include <strings.h>

#include "htime.h"

#define KK_TDB_TDT	0.001657
#define EB_TDB_TDT	0.016710
#define M0_TDB_TDT	6.239996
#define M1_TDB_TDT	1.99096871e-07

#ifndef J2000_EPOCH
#define J2000_EPOCH	630763200.000000
#endif /* J2000_EPOCH */

HTLdbl
Ht_GPS_to_TDT(HTLdbl gps)
{
	return(gps + TDT_GPS_DELTA - J2000_EPOCH);
}
HTLdbl
Ht_TDT_to_GPS(HTLdbl tdt)
{
	return(tdt - TDT_GPS_DELTA + J2000_EPOCH);
}

HTLdbl
Ht_GPS_to_TAI(HTLdbl gps)
{
	return(gps + TAI_GPS_DELTA - J2000_EPOCH);
}
HTLdbl
Ht_TAI_to_GPS(HTLdbl tai)
{
	return(tai - TAI_GPS_DELTA + J2000_EPOCH);
}

HTLdbl
e_tdb_tdt(HTLdbl t)
{
	HTLdbl	m = M0_TDB_TDT + M1_TDB_TDT*t;
	return(m + EB_TDB_TDT * sin(m));
}

/* this is approximate, but overkill */
HTLdbl
Ht_TDT_to_TDB(HTLdbl tdt)
{
	HTLdbl	tdb = tdt + KK_TDB_TDT * sin(e_tdb_tdt(tdt));
	return(tdt + KK_TDB_TDT * sin(e_tdb_tdt(tdb)));
}
/* this is correct */
HTLdbl
Ht_TDB_to_TDT(HTLdbl tdb)
{
	return(tdb - KK_TDB_TDT * sin(e_tdb_tdt(tdb)));
}

/*
 * Return an ephemeris type
 */
int
Ht_ephem_type(char *arg)
{
    if      (!strcasecmp(arg, "gps"))       return(SPICE_ET_TYPE_GPS);
    else if (!strcasecmp(arg, "tai"))       return(SPICE_ET_TYPE_TAI);
    else if (!strcasecmp(arg, "tdt"))       return(SPICE_ET_TYPE_TDT);
    else if (!strcasecmp(arg, "tdb"))       return(SPICE_ET_TYPE_TDB);
    else if (!strcasecmp(arg, "utc"))       return(SPICE_ET_TYPE_UTC);
    else                                    return(SPICE_ET_TYPE_GPS);
}

/*
 * this might be useful
 */
char
*Ht_convert_et_to_gps(char *arg, int eph_type)
{
    static char     buf[4096];
    HTLdbl          et = atof(arg);
    switch (eph_type) {
    case 4:
    default: case 0:                                break;
    case 1: et = Ht_TAI_to_GPS(et);                 break;
    case 2: et = Ht_TDT_to_GPS(et);                 break;
    case 3: et = Ht_TDT_to_GPS(Ht_TDB_to_TDT(et));  break;
    }
#ifdef USE_LONG_DOUBLE
    snprintf(buf, 4096, "%Lf\n", et);
#else
    snprintf(buf, 4096, "%lf\n", et);
#endif
    return(buf);
}

/*
 * This routine loads a buffer with something some of the spice
 * routines will read. I think.
 */
void Ht_spice_sprintf(char *b, char *tla, HTLdbl t)
{
    char        *dp;
    int         p;
    /* [Thu] Jun 13 05:47:00[.000000] UTC 1996 */
    /*       4   8  11       19       27  31   */
    dp = Ht_GPS_to_string(t, Ht_DS_DATE);
    for (p = 31; p < 35; p++) *b++ = dp[p];
    *b++ = ' ';
    for (p =  4; p < 23; p++) *b++ = dp[p];
    *b++ = ' ';
    for (p =  0; p <  3; p++) *b++ = *tla++;
    *b = '\n';
}

/*
 * code fragments...which need to get coupled to the C/SPICE type.

    "The -C allows you to convert a time into the ephemeris\n"
    "time (relative to J2000) as specified by the -c flag.\n"
    "To convert an ephemeris time into something else, you\n"
    "need to use the -x flag flag that, and also use the -c\n"
    "flag to indicate whether it is tdb/tdt/tai (anything\n"
    "else treats it as a gps time). E.g.\n"
    "\n"
    "  ibex_time -x -c tdt -E 277688705.184000\n"
    "   2008/10:19:11:44:00\n"
    "  ibex_time -c tdt -C 2008/10:19:11:44:00\n"
    "   277688705.184000\n"
    "\n"
    "The -O (oh) option prints out a date-like thing such as\n"
    "is required by some of the spice tools; usually requiring\n"
    "that you use -c with tdt, tdb or tai.\n"

 * need to define types and add more plumbing....
 */
#define ORIGINAL_TIMEIS_CODE 0
#if ORIGINAL_TIMEIS_CODE

    case 'O':
	gps = Ht_string_to_GPS(when, &terror);
	switch (eph_type) {
	case 1:  tla = "TAI"; gps += TAI_GPS_DELTA; break;
	case 2:  tla = "TDT"; gps += TDT_GPS_DELTA; break;
	case 3:  tla = "TDB";
	    gps = Ht_TDT_to_TDB(Ht_GPS_to_TDT(gps))
		+ SPICE_ET_ORIGIN;		    break;
	default: tla = "WTF";			    break;
	}
	spice_printf(buf, tla, gps);
	break;

    case 'C':
	gps = Ht_string_to_GPS(when, &terror);
	/* get leapsecs since j2000 */
	switch (eph_type) {
	default: case 0:                                 break;
	case 1: gps = Ht_GPS_to_TAI(gps);                break;
	case 2: gps = Ht_GPS_to_TDT(gps);                break;
	case 3: gps = Ht_TDT_to_TDB(Ht_GPS_to_TDT(gps)); break;
	case 4: gps = Ht_leapsecs_since_J2000(gps);
		gps += TDT_GPS_DELTA + 13.0;		 break;
	}
#ifdef USE_LONG_DOUBLE
	sprintf(buf, "%Lf\n", gps);
#else
	sprintf(buf, "%lf\n", gps);
#endif
	break;

#endif /* ORIGINAL_TIMEIS_CODE */

/*
 * eof
 */
