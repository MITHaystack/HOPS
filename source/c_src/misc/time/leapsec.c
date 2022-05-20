/*
 *  $Source: /usr/hete/repository/infra/ops/leapsec.c,v $
 *
 *  Leapsecond module.
 *
 *  Copyright (C) 1999-2020 by MIT.                  All rights reserved.
 *
 */

#include "htime.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef TAI_GPS_DELTA
#define TAI_GPS_DELTA	19/*seconds*/
#define JD_GPS_EPOCH	(2444239.5 + 5.0)/*days*/
#define JD_UNIX_EPOCH	(2440587.5)/*days*/
#endif /* TAI_GPS_DELTA */

/*
 *  A list of the instants in UTC with non-zero duration.
 *  This is only valid since 1972, before that, we give up.
 *
 *  This table is used only if IBEX_OPS/IBEX_DATACALSC is
 *  not defined and/or the tai-utc.dat file cannot be found.
 */
typedef struct leaptable LeapTableEntry;
static struct leaptable {
	HTLdbl	gps_start, gps_end, julian_day; int unix_secs;
} leapsecs[] = {

 /* GPS before   GPS after     Julian      Unix after  tai-utc calendar dateH*/
 { -252892810.0, -252892809.0, 2441317.5,  63072000 }, /*Jan  1 00:00:00 1972*/
 { -237168009.0, -237168008.0, 2441499.5,  78796800 }, /*Jul  1 00:00:00 1972*/
 { -221270408.0, -221270407.0, 2441683.5,  94694400 }, /*Jan  1 00:00:00 1973*/
 { -189734407.0, -189734406.0, 2442048.5, 126230400 }, /*Jan  1 00:00:00 1974*/
 { -158198406.0, -158198405.0, 2442413.5, 157766400 }, /*Jan  1 00:00:00 1975*/
 { -126662405.0, -126662404.0, 2442778.5, 189302400 }, /*Jan  1 00:00:00 1976*/
 {  -95040004.0,  -95040003.0, 2443144.5, 220924800 }, /*Jan  1 00:00:00 1977*/
 {  -63504003.0,  -63504002.0, 2443509.5, 252460800 }, /*Jan  1 00:00:00 1978*/
 {  -31968002.0,  -31968001.0, 2443874.5, 283996800 }, /*Jan  1 00:00:00 1979*/
 {    -432001.0,    -432000.0, 2444239.5, 315532800 }, /*Jan  1 00:00:00 1980*/
 {   46828800.0,   46828801.0, 2444786.5, 362793600 }, /*Jul  1 00:00:00 1981*/
 {   78364801.0,   78364802.0, 2445151.5, 394329600 }, /*Jul  1 00:00:00 1982*/
 {  109900802.0,  109900803.0, 2445516.5, 425865600 }, /*Jul  1 00:00:00 1983*/
 {  173059203.0,  173059204.0, 2446247.5, 489024000 }, /*Jul  1 00:00:00 1985*/
 {  252028804.0,  252028805.0, 2447161.5, 567993600 }, /*Jan  1 00:00:00 1988*/
 {  315187205.0,  315187206.0, 2447892.5, 631152000 }, /*Jan  1 00:00:00 1990*/
 {  346723206.0,  346723207.0, 2448257.5, 662688000 }, /*Jan  1 00:00:00 1991*/
 {  393984007.0,  393984008.0, 2448804.5, 709948800 }, /*Jul  1 00:00:00 1992*/
 {  425520008.0,  425520009.0, 2449169.5, 741484800 }, /*Jul  1 00:00:00 1993*/
 {  457056009.0,  457056010.0, 2449534.5, 773020800 }, /*Jul  1 00:00:00 1994*/
 {  504489610.0,  504489611.0, 2450083.5, 820454400 }, /*Jan  1 00:00:00 1996*/
 {  551750411.0,  551750412.0, 2450630.5, 867715200 }, /*Jul  1 00:00:00 1997*/
 {  599184012.0,  599184013.0, 2451179.5, 915148800 }, /*Jan  1 00:00:00 1999*/
 {  820108813.0,  820108814.0, 2453736.5,1136073600 }, /*Jan  1 00:00:00 2006*/
 {  914803214.0,  914803215.0, 2454832.5,1230768000 }, /*Jan  1 00:00:00 2009*/
 { 1025136015.0, 1025136016.0, 2456109.5,1341100800 }, /*Jul  1 00:00:00 2012*/
 { 1119744016.0, 1119744017.0, 2457204.5,1435708800 }, /*Jul  1 00:00:00 2015*/
 { 1167264017.0, 1167264018.0, 2457754.5,1483228800 }, /*Jan  1 00:00:00 2017*/
 /* GPS before   GPS after     Julian      Unix after  tai-utc calendar dateT*/

/*
 * when adding new entries, bump DEFAULT_LEAP_CNT, grab the Julian time
 * from tai-utc.dat, update the comment appropriately and fill out the
 * other three columns...e.g.
 * HOPS_JULIAN_EPOCH=UTC hops_time -q G 20081231_235959.000000
 * => 914803213.000000 (add one to get 235960 for GPS before
 * HOPS_JULIAN_EPOCH=UTC hops_time -q G 20090101_000000.000000
 * => 914803215.000000 (once entered this is GPS after)
 * HOPS_JULIAN_EPOCH=UTC hops_time -q K 20090101_000000.000000
 * => 1230768000 (once entered, this is Unix after)
 */

};
/* count the lines: grep 00:00:00 leapsec.c | grep -v count | wc -l */
#define DEFAULT_LEAP_CNT    28
#if DEFAULT_LEAP_CNT != 28
# error "The compiled leapsecs table is misconfigured."
#endif /* leap_cnt */

static int leap_sec_debug = 0;
static int leap_cnt = sizeof(leapsecs)/sizeof(LeapTableEntry);
static int tai_utc_wanted = 1;

/*
 * A cache for lookups of GPS year starts, good until 2199.
 */
static struct { HTLdbl gps; int lsi; } gps_year_starts[200];
static void load_gps_year_starts(void);

/*
 * A hook for further processing once the leap table is in place.
 * It returns leap_cnt, which is the number of leap seconds, and
 * clears tai_utc_wanted so a reload never again recurs.
 */
static int
leap_cnt_loaded(void)
{
    tai_utc_wanted = 0;
    load_gps_year_starts();
    return(leap_cnt);
}


/*
 *  This routine loads the TAI-UTC database file
 *  if found on top of the default leapsecond data.
 */
static int
load_tai_utc(void)
{
	char	tai_utc[1024], *cal = getenv("IBEX_OPS");
	char	*caldir = getenv("IBEX_DATACALSC");
	FILE	*tai_file;
	HTLdbl	delta;
	int	i, mjd;

        assert(leap_cnt == DEFAULT_LEAP_CNT);
        if (leap_sec_debug) fprintf(stderr, "Leap Count %d\n", leap_cnt);

	if (Ht_tz_unset) {
                if (leap_sec_debug) fprintf(stderr,
                    "setting TZ to GMT\n", leap_cnt);
		putenv("TZ=GMT");
		Ht_tz_unset = 0;
	}

	if (!cal) {
		cal = getenv("HETE_OPS");
                if (leap_sec_debug) fprintf(stderr, "HETE_OPS is %s\n", cal);
		if (!cal) return(leap_cnt_loaded());
	}
	if (!caldir) caldir = Ht_DATACALSC;
	sprintf(tai_utc, TAI_UTC_DATA, cal, caldir);
        if (leap_sec_debug) fprintf(stderr, "tai_utc is %s\n", tai_utc);
	if (!(tai_file = fopen(tai_utc, "r"))) return(leap_cnt_loaded());
	leap_cnt = 0;

        if (leap_sec_debug) fprintf(stderr, "Reading TAI-UTC table\n");
	while (fgets(tai_utc, sizeof(tai_utc), tai_file)) {
		i = sscanf(tai_utc,
			" %*d %*s %*d =JD %"HLDL"f  TAI-UTC=  %"HLDL"f"
			" S + (MJD - %d",
			&leapsecs[leap_cnt].julian_day, &delta, &mjd);
		if (i != 3 || mjd != 41317) continue;
		leapsecs[leap_cnt].gps_end =
			leapsecs[leap_cnt].julian_day - JD_GPS_EPOCH;
		leapsecs[leap_cnt].gps_end *= 86400;
		leapsecs[leap_cnt].gps_end += (delta - TAI_GPS_DELTA);
		leapsecs[leap_cnt].gps_start =
			leapsecs[leap_cnt].gps_end - 1.0;
		leapsecs[leap_cnt].unix_secs =
			leapsecs[leap_cnt].julian_day - JD_UNIX_EPOCH;
		leapsecs[leap_cnt].unix_secs *= 86400;

		leap_cnt++;
	}

	return(leap_cnt_loaded());
}

/*
 *  Look up the unix seconds in the leapseconds table and do the math.
 */
GTime
Ht_UTC_to_GPS(UTime ut, HtErr *err)
{
	int	i = leap_cnt;
	GTime	gps;

	if (!err) err = &Ht_default_error;

	/* load leapsecond table if required */
	if (tai_utc_wanted) i = load_tai_utc();

	/* work backwards */
	while (i-- > 0) if (leapsecs[i].unix_secs <= ut.tv_sec) break;

	/* before the dawn of time is a lose */
	if (i < 0) {
		*err = (HTLdbl)TAI_GPS_DELTA;
		gps = (HTLdbl)ut.tv_sec
		    - ((JD_GPS_EPOCH - JD_UNIX_EPOCH) * 86400.0) - 9.0;
	} else {
		*err = (leapsecs[i].unix_secs == ut.tv_sec) ? 1.0 : 1e-6;
		gps = leapsecs[i].gps_end
		    + (ut.tv_sec - leapsecs[i].unix_secs);
	}

	gps += (1E-6)*(HTLdbl)ut.tv_usec;
	return(gps);
}

/*
 * Introduced to support ephemeris time calculations (eph_time.c)
 * all of which are relative to J2000.  An improved version of this
 * routine could remember the last query.
 */
GTime
Ht_leapsecs_since_J2000(GTime gt)
{
	int	i = leap_cnt;
	int	j;
	if (tai_utc_wanted) i = load_tai_utc();
	while (i-- > 0) if (leapsecs[i].gps_start <= gt) break;
	if (i < 0) return(0);
	j = leap_cnt;
	while (j-- > 0) if (leapsecs[j].gps_start <= J2000_EPOCH) break;
	return((HTLdbl)(i - j));
}

/*
 * Caches the time from year start where the leap second is
 * so that Ht_leapsecs_from_year_start_to_gps() is trivial.  We are
 * really only trying to support recent years (i.e. GPS era).
 *
 * (We assume only one per year which should hold this century.)
 * (We also assume leap_cnt has also been correctly assigned.)
 *
 * Ht_string_to_GPS() is not safe (depending how we got here).
 */
static void
load_gps_year_starts(void)
{
    char start[50];
    int yy, lsi = 0;
    HTLdbl gps;
    /* zero out the pre-GPS era */
    for (yy = 0; yy < 13; yy++) {
        gps_year_starts[yy].gps = 0.0;
        gps_year_starts[yy].lsi = lsi;
    }
    /* load the nonzero entries */
    while (yy < 200) {
        sprintf(start, "%04d0101_000000", 1900 + yy);
        /* gps = Ht_string_to_GPS(start, 1); */
        gps = Ht_UTC_to_GPS(pass_str2ut(start),0);

        if (gps > leapsecs[lsi].gps_start) {
            while (lsi < (leap_cnt-1) &&
                   gps > leapsecs[lsi+1].gps_start) lsi++;
        }
        gps_year_starts[yy].gps = gps;
        gps_year_starts[yy].lsi = lsi;

        if (lsi > 0 && lsi < leap_cnt-1) assert(
            (leapsecs[lsi].gps_end <= gps) &&
            (leapsecs[lsi+1].gps_start > gps)
        );
        yy++;
    }
}
HTLdbl Ht_leapsecs_from_year_start_to_gps(int year, GTime gps)
{
    HTLdbl extra = 0.0;
    int yy = year - 1900, lsi = gps_year_starts[yy].lsi;
    while (lsi < leap_cnt-1 && gps > leapsecs[lsi+1].gps_end) {
        lsi++;
        extra += 1.0;
    }
    return(extra);
}

/*
 * Returns the number of leap seconds inserted
 * from GPS EPOCH (1980) to HOPS_JULIAN_EPOCH.
 */
int
Ht_julian_epoch(void)
{
	static int  retv = -999;
	int	    i = leap_cnt, j;
	char	    *je;
	GTime	    gt;
	if (retv != -999) return(retv);
	if (tai_utc_wanted) i = load_tai_utc();
	je = getenv("HOPS_JULIAN_EPOCH");
	if      (!je)                gt = 0;    /* GPS epoch */
	else if (!strcmp(je, "GPS")) gt = 0;
	else if (!strcmp(je, "UTC")) return(retv = Ht_JULIAN_EPOCH_IS_UTC);
	else                         gt = Ht_string_to_GPS(je, 0);
	while (i-- > 0) if (leapsecs[i].gps_start <= gt) break;
	if (i < 0) return(0);
	j = leap_cnt;
	while (j-- > 0) if (leapsecs[j].gps_start <= 0) break;
	retv = i - j;
	return(retv);
}

UTime
Ht_GPS_to_UTC(GTime gt, HtErr *err)
{
	int	i = leap_cnt;
	UTime	utc;

	if (!err) err = &Ht_default_error;

	/* load leapsecond table if required */
	if (tai_utc_wanted) i = load_tai_utc();

	while (i-- > 0) if (leapsecs[i].gps_start <= gt) break;

	/* before the dawn of time is a lose */
	if (i < 0) {
		*err = (HTLdbl)TAI_GPS_DELTA;
		utc.tv_sec = (int)gt
			   + ((int)(JD_GPS_EPOCH - JD_UNIX_EPOCH) * 86400) + 9;
	} else {
		*err = 1e-6;
		if (gt < leapsecs[i].gps_end) {
			gt += 1.0;
			*err += 1.0;
		}
		gt -= leapsecs[i].gps_end;
		utc.tv_sec = leapsecs[i].unix_secs + (int)gt;
	}

	gt -= (int)gt;
	utc.tv_usec = (int)(1E6 * gt + 0.5);
	while (utc.tv_usec < 0) {
		utc.tv_usec += 1000000;
		utc.tv_sec -= 1;
	}
	while (utc.tv_usec >= 1000000) {
		utc.tv_usec -= 1000000;
		utc.tv_sec += 1;
	}
	return(utc);
}

/*
 *  Borrowed from Numerical Recipes.
 *
 *  Ht_julday returns the Julian Day Number which begins at Noon of
 *  the calendar date specified by month mm, day id, and year iyyy,
 *  which are integer variables.  The year after 1 BC (-1) is 1 AD (1).
 */
    
#define IGREG (15+31L*(10+12L*1582)) /* Gregorian calendar adopted */

int
Ht_julday(int mm, int id, int iyyy)
{
	int jul,ja,jy,jm;
	if (iyyy == 0) {
		fputs("Ht_julday: there is no year zero.\n", stderr);
		exit(1);
	}
	if (iyyy < 0) ++iyyy;
	if (mm > 2) {
		jy=iyyy;
		jm=mm+1;
	} else {
		jy=iyyy-1;
		jm=mm+13;
	}
	jul = (int)(floor(365.25*jy)+floor(30.6001*jm)+id+1720995);
	if (id+31L*(mm+12L*iyyy) >= IGREG) {
		ja=0.01*jy;
		jul += 2-ja+(int) (0.25*ja);
	}
	return jul;
}

/*
 * If the JULIAN_EPOCH_IS_UTC, we work this out from the leap second table.
 * Note, in this case, leap seconds aren't handled correctly.
 */
GTime
Ht_julian_to_GPS_via_UTC(HTLdbl julian)
{
    int	    ii = leap_cnt;
    GTime   gps;
    while (ii-- > 0) if (julian >= leapsecs[ii].julian_day) break;
    julian -= leapsecs[ii].julian_day;	/* days from UT epoch */
    julian *= 86400.0;			/* secs from UT epoch */
    /* not enough precision */
    julian = rint(julian*20000.0) * (1.0/20000.0);
    gps = leapsecs[ii].gps_end;		/* corresponding GPS */
    gps += julian;
    return(gps);
}
HTLdbl
Ht_GPS_to_julian_via_UTC(GTime gps)
{
    int	    ii = leap_cnt;
    HTLdbl  julian;
    while (ii-- > 0) if (gps >= leapsecs[ii].gps_end) break;
    gps -= leapsecs[ii].gps_end;	/* secs from UT epoch */
    gps /= 86400.0;			/* days from UT epoch */
    julian = leapsecs[ii].julian_day;
    julian += gps;
    return(julian);
}

/*
 * for historical reasons this was so named, but julday->julian
 * is available as a sane synonym.  The rint() is necessary since
 * GTime doesn't hold all the bits, so we round it out predictably,
 * dropping a few tens of milliseconds.
 */
GTime
Ht_julday_to_GPS(HTLdbl julian)
{
	int je = Ht_julian_epoch();
	if (je == Ht_JULIAN_EPOCH_IS_UTC)
	    return(Ht_julian_to_GPS_via_UTC(julian));
	julian -= JD_GPS_EPOCH;		/* days in the GPS epoch */
	julian *= 86400.0;		/* secs in the GPS epoch */
	julian += je;
	/* not enough precision */
	julian = rint(julian*20000.0) * (1.0/20000.0);
	return(julian);
}
GTime
Ht_julian_to_GPS(HTLdbl julian)
{
	return(Ht_julday_to_GPS(julian));
}
HTLdbl
Ht_GPS_to_julian(GTime gps)
{
	int je = Ht_julian_epoch();
	if (je == Ht_JULIAN_EPOCH_IS_UTC)
	    return(Ht_GPS_to_julian_via_UTC(gps));
	gps -= je;
	gps /= 86400.0;			/* days in the gps epoch */
	gps += JD_GPS_EPOCH;		/* days in the gps epoch */
	return(gps);
}

/*
 * The modified versions just shift by the high digits....
 */
GTime
Ht_modjul_to_GPS(HTLdbl mjd)
{
	/* MJD == JD - 2400000.5 */
	mjd += 2400000.5;		/* to julian days */
	return(Ht_julday_to_GPS(mjd));
}
HTLdbl
Ht_GPS_to_modjul(GTime gps)
{
	/* MJD == JD - 2400000.5 */
	gps = Ht_GPS_to_julian(gps);	/* to julian days */
	return(gps - 2400000.5);
}

#undef IGREG

/*
 * set to 1 in the Makefile for leap_sec_chk checking
 * left undefined for the library implementation
 */
#ifndef CALCULATE_DEFAULT_TABLE
#define CALCULATE_DEFAULT_TABLE 0
#endif /* CALCULATE_DEFAULT_TABLE */

#if CALCULATE_DEFAULT_TABLE

static void
test_UTC(int t)
{
	int	i, j;
	UTime	utc, ut2;
	GTime	gps;
	HtErr	err;
        char    utc0[80], utc1[80], gpsx[80];

        if (leap_sec_debug) fprintf(stderr, "test_UTC(%d)\n", t);
	for (i = -2; i < 3; i++) for (j = 0; j < 1000000; j += 333333) {
		utc.tv_sec = t + i;
		utc.tv_usec = j;
                strcpy(utc0, Ht_UTC_to_string(utc, Ht_DS_ISOT));
		gps = Ht_UTC_to_GPS(utc, &err);
                strcpy(gpsx, Ht_GPS_to_string(gps, Ht_DS_ISOT));
		ut2 = Ht_GPS_to_UTC(gps, &err);
                strcpy(utc1, Ht_UTC_to_string(ut2, Ht_DS_ISOT));
                fprintf(stdout, "At %s -> %s\n-> %s\n", utc0, gpsx, utc1);
		fprintf(stdout, "% 10d.%06d0 >%18.7"HLDL"f [%7.1"HLDL"e] ",
			(int)utc.tv_sec, (int)utc.tv_usec, gps, err);
		fprintf(stdout, ">% 10d.%06d0 [%7.1"HLDL"e]\n",
			(int)ut2.tv_sec, (int)ut2.tv_usec, err);
	}
}

static void
test_GPS(HTLdbl t)
{
	int	i, j;
	UTime	utc;
	GTime	gps;
	HtErr	err;

        if (leap_sec_debug) fprintf(stderr, "test_GPS(%"HLDL"f)\n", t);
	for (i = -2; i < 3; i++) for (j = 0; j < 1000000; j += 333333) {
		gps = t + (HTLdbl)i + (1e-6)*(HTLdbl)j;
		utc = Ht_GPS_to_UTC(gps, &err);
		fprintf(stdout, "%18.7"HLDL"f >% 10d.%06d0 [%7.1"HLDL"e] ",
			gps, (int)utc.tv_sec, (int)utc.tv_usec, err);
		gps = Ht_UTC_to_GPS(utc, &err);
		fprintf(stdout, ">%18.7"HLDL"f [%7.1"HLDL"e]\n",
			gps, err);
	}
}

#include <time.h>
#include <sys/time.h>

void
dump_leaps(void)
{
	int	j;

        if (leap_sec_debug) fprintf(stderr, "Dumping Leap Seconds\n");
	for (j = 0; j < leap_cnt; j++) {
		fprintf(stdout,
		    (leapsecs[j].unix_secs > 1000000000)
		    ?  " {% 13.1"HLDL"f,% 13.1"HLDL"f,% 9.1"HLDL"f,%10d }, "
                        "/*%20.20s*/\n"
		    :  " {% 13.1"HLDL"f,% 13.1"HLDL"f,% 9.1"HLDL"f, %9d }, "
                        "/*%20.20s*/\n"
		    ,
			leapsecs[j].gps_start,
			leapsecs[j].gps_end,
			leapsecs[j].julian_day,
			leapsecs[j].unix_secs,
			asctime(gmtime((time_t*)&leapsecs[j].unix_secs))+4);
	}
}

void
dump_julian(void)
{
        if (leap_sec_debug) fprintf(stderr, "Dumping Julian Days\n");
	fprintf(stdout, "JD %d JAN 1 = %.1f\n", 1970, Ht_julday(1,1,1970)-.5);
	fprintf(stdout, "JD %d JAN 1 = %.1f\n", 1972, Ht_julday(1,1,1970)-.5);
	fprintf(stdout, "JD %d JAN 1 = %.1f\n", 1980, Ht_julday(1,1,1980)-.5);
	fprintf(stdout, "JD %d JAN 6 = %.1f\n", 1980, Ht_julday(1,6,1980)-.5);
	fprintf(stdout, "JD %d JAN 1 = %.1f\n", 1990, Ht_julday(1,1,1980)-.5);
	fprintf(stdout, "JD %d JAN 1 = %.1f\n", 1999, Ht_julday(1,1,1999)-.5);
	fprintf(stdout, "JD %d JAN 1 = %.1f\n", 2000, Ht_julday(1,1,2000)-.5);
	fprintf(stdout, "JD %d JAN 1 = %.1f\n", 2010, Ht_julday(1,1,2010)-.5);
	fprintf(stdout, "JD %d JAN 1 = %.1f\n", 2020, Ht_julday(1,1,2020)-.5);
}

void
dump_clocks(void)
{
	unsigned long	x = 0;
        int             n = 0;

        if (leap_sec_debug) fprintf(stderr, "Dumping UNIX Clocks\n");
	fprintf(stdout, "%08x = %s",(int)x, asctime(gmtime(&x)));
	for (x = 0x10000000; x < 0xFF000000 && n < 18; x += 0x10000000, n++)
		fprintf(stdout, "%x = %s",(unsigned)x,asctime(gmtime(&x)));
}

void
dump_epochs(void)
{
	long x = 0, z;
	HTLdbl y = 0.0;
	int i;

        if (leap_sec_debug) fprintf(stderr, "Dumping HETE Epochs\n");
	for (i = 0; i < 7*4; i++) {
		x = (int)(y / 1000000.0);
		z = (int)( y / 4294967296.0 + .5 );
		printf("%04x %04x 0000 0000 = %s",
			(unsigned)(z >> 16 | EPOCH_UNIX(0)),
			(unsigned)(z & 0xffff),
			asctime(gmtime((&x))));
		y += 281474976710656. / 4.;
	}
}

int
main(int argc, char **argv)
{
        char *lsd = getenv("LEAP_SEC_DEBUG");
	int dw = 0;

        leap_sec_debug = (lsd) ? atoi(lsd) : 0;
        if (leap_sec_debug) fprintf(stderr, "Leap Second Debugging Enabled\n");
	load_tai_utc();

	if (argc == 1) {
		dump_leaps();
		return(0);
	}

	if (argc == 2) {
	    if (atoi(argv[1]) & 0x01) dump_leaps();
	    if (atoi(argv[1]) & 0x02) dump_clocks();
	    if (atoi(argv[1]) & 0x04) dump_epochs();
	    if (atoi(argv[1]) & 0x08) dump_julian();
            if (atoi(argv[1]) & 0x10) {
		test_UTC(63072000);
		test_UTC(315532800 + 5 * 86400);
		test_UTC(915148800);
            }
            if (atoi(argv[1]) & 0x20) {
		test_GPS(-252892810.0);
		test_GPS(-432001.0 + 5. * 86400.);
		test_GPS(599184012.0);
            }
            return(0);
	}

        if (leap_sec_debug) fprintf(stderr, "Dumping Digital Watch Epochs\n");
	while (argc-- > 0) {
		int yy, mm, dd, hh, nn, ss;
		HTLdbl seconds, gps, err;
		UTime ut;
		char *date;

		sscanf(*++argv, "%d/%d/%d", &yy, &mm, &dd);	argc--;
		sscanf(*++argv, "%d:%d:%d", &hh, &nn, &ss);	argc--;
		seconds = (Ht_julday(mm, dd, yy) -.5 - JD_UNIX_EPOCH) * 86400;
		seconds += hh*3600 + nn*60 + ss + 0.5;
		ut.tv_sec = (int)seconds;
		ut.tv_usec = 0;
		gps = Ht_UTC_to_GPS(ut, &err);
		date = asctime(gmtime(&ut.tv_sec));
		date[24] = 0;
		fprintf(stdout,
			"{ %.1"HLDL"f, 0.0, { EPOCH_WATCH(0x%1x), 0, 0, 0 },\n"
			"  3.0, 1.0, 1.0, 0, 0, \"DW-%1X Reset: %s UTC\" }\n",
			gps, dw, dw, date+4);
		dw++;
	}
	return(0);
}

#endif /* CALCULATE_DEFAULT_TABLE */

/*
 * eof
 */
