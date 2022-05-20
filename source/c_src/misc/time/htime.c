/*
 *  $Id: htime.c 4184 2009-03-02 20:32:47Z gbc $
 *
 *  HETE time library
 *
 *  Copyright (C) 1998 by MIT.			All rights reserved.
 *
 *  Quick port from HETE-1:  BEWARE EPOCH!
 *
 *  In particular, the UTime timeval scheme dies on the epoch bits,
 *  So ultimately, those routines go away in favor of HTLdbls.
 *
 *  NO-leap smarts or databases yet....
 *  IN particular meaning of Life is yet undefined.
 *
 *  Older HETE-1 stuff precedes newer HETE-2 stuff so that some
 *  day the former may be static'd or deleted as appropriate.
 *
 *  Hacked for IBEX.
 */

#include <assert.h>

#include "htime.h"
#include <ctype.h>

/* all globals are defined in lifetime.c */

static UTime		unix_str2ut(char *s);
static UTime		date_str2ut(char *s, int *ptyp);
static GTime		rltv_str2ut(char *s, HTLdbl *err);

static HTime		hex_str2ht(char *s, char *fmt);

static char		*gtime(UTime *ut, int alt);
static char		*sget_day(char *s, struct tm *tm);
static char		*sget_mon(char *s, struct tm *tm);

//upper limit on number of weeks count, this is ~157 years
#define MAX_WEEKS 8192

/*
 * HOPS rootcode functions:
 * Original:
 *   [a-z]x6 == # of 4-sec periods since midnight Jan 1 1979,
 *   (HOPS_ROOT_EPOCH) chosen to cover the VLBI era more or less
 *   unambiguosly until zzzzzz at 2018-02-26T15:45:0[0-3]
 * Modified:
 *   [0-9A-Z]x6 == # of 1-sec periods since HOPS_ROOT_BREAK
 * s points to a string to parse
 * date points to a place for the result
 */
static UTime hopsrcold_str2ut(char *s)
{
    UTime   ut;
    ut.tv_usec = 0;
    ut.tv_sec = 0;
    ut.tv_sec += (*s++ - 'a') * 26*26*26*26*26;
    ut.tv_sec += (*s++ - 'a') * 26*26*26*26;
    ut.tv_sec += (*s++ - 'a') * 26*26*26;
    ut.tv_sec += (*s++ - 'a') * 26*26;
    ut.tv_sec += (*s++ - 'a') * 26;
    ut.tv_sec += (*s++ - 'a');
    ut.tv_sec *= 4;	/* 4-sec periods */
    ut.tv_sec += HOPS_ROOT_EPOCH + 2;
    return(ut);
}
static void ut2hopsrcold_str(UTime ut, char *date)
{
    int	m, u = ut.tv_sec;
    if (ut.tv_usec > 500000) u++;
    u -= HOPS_ROOT_EPOCH;
    u >>= 2;		/* 4-sec periods */
    m  = u / (26*26*26*26*26);
    u -= m * (26*26*26*26*26);
    *date++ = (char)('a' + m);
    m  = u / (26*26*26*26);
    u -= m * (26*26*26*26);
    *date++ = (char)('a' + m);
    m  = u / (26*26*26);
    u -= m * (26*26*26);
    *date++ = (char)('a' + m);
    m  = u / (26*26);
    u -= m * (26*26);
    *date++ = (char)('a' + m);
    m  = u / (26);
    u -= m * (26);
    *date++ = (char)('a' + m);
    *date++ = (char)('a' + u);
    *date = 0;
}
static UTime hopsrcnew_str2ut(char *s)
{
    UTime   ut;
    ut.tv_usec = 0;
    ut.tv_sec = HOPS_ROOT_BREAK;
    ut.tv_sec += (time_t) strtoul(s, 0, 36);
    return(ut);
}
static void ut2hopsrcnew_str(UTime ut, char *date)
{
    long m, u = ut.tv_sec;
    if (ut.tv_usec > 500000) u++;
    u -= HOPS_ROOT_BREAK;
    m = u / (36*36*36*36*36);
    u -= m * (36*36*36*36*36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    m = u / (36*36*36*36);
    u -= m * (36*36*36*36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    m = u / (36*36*36);
    u -= m * (36*36*36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    m = u / (36*36);
    u -= m * (36*36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    m = u / (36);
    u -= m * (36);
    *date++ = (m < 10) ? (char)('0' + m) : (char)('A' + m - 10);
    *date++ = (u < 10) ? (char)('0' + u) : (char)('A' + u - 10);
    *date = 0;
}
static UTime hopsrc_str2ut(char *s)
{
    int ii, new = 0;
    for (ii = 0; ii < 6; ii++)
        if (isdigit(s[ii]) || isupper(s[ii])) {
            new = 1;
            break;
        }
    if (new) return(hopsrcnew_str2ut(s));
    else     return(hopsrcold_str2ut(s));
}
static void ut2hopsrc_str(UTime ut, char *date)
{
    if (ut.tv_sec < HOPS_ROOT_BREAK) return(ut2hopsrcold_str(ut, date));
    else                             return(ut2hopsrcnew_str(ut, date));
}


/*
 *  Fill in tm_wday element; returns ptr to next part if recognized.
 */
static char *
sget_day(char *s, struct tm *tm)
{
	static char	*dw[] = { "Sun", "Mon", "Tue", "Wed",
				  "Thu", "Fri", "Sat" };
	register int	i;

	for (i = 0; i < 7; i++)
		if (!strncmp(s, dw[i], 3)) {
			tm->tm_wday = i;
			return(s+4);
		}
	return(s);
}

/*
 *  Fill in tm_mon element; returns ptr to next part if recognized.
 */
static char *
sget_mon(char *s, struct tm *tm)
{
	static char	*dm[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
				  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	register int	i;

	for (i = 0; i < 12; i++)
		if (!strncmp(s, dm[i], 3)) {
			tm->tm_mon = i;
			return(s+4);
		}
	return(s);
}

/*
 *  Convert seconds to GPS
 */
static GTime
sec_str2gps(char *s)
{
	GTime	gps;
#ifdef USE_LONG_DOUBLE
	sscanf(s, "%Lf", &gps);
#else
	sscanf(s, "%lf", &gps);
#endif
	return(gps);
}

/*
 *  Convert weeks and seconds to GPS
 */
static GTime
wks_str2gps(char *s)
{
	GTime	gps;
	int	wks;

#ifdef USE_LONG_DOUBLE
    sscanf(s, "%d%*c%Lf", &wks, &gps);
#else
	sscanf(s, "%d%*c%lf", &wks, &gps);
#endif

	gps += wks * (86400. * 7.);
	return(gps);
}

/*
 *  Convert Year and Secs UTC to GPS
 *  ( GRBYYMMDD+SECONDS )
 */
static GTime
grb_str2gps(char *s)
{
	static char	pass[22];
	static UTime	ut;
	static GTime	gps, secs;
	extern double	atof();

	strncpy(pass, s, 6);
	pass[6] = '_';
	pass[7] = pass[8] = pass[9] = pass[10] = pass[11] = pass[12] = '0';
	pass[13] = '.';
	pass[14] = pass[15] = pass[16] = pass[17] = pass[18] = pass[19] = '0';
	ut = pass_str2ut(pass);

	gps = Ht_UTC_to_GPS(ut, 0);
	secs = atof(s+7);
	gps += secs;

	return(gps);
}

/*
 *  Convert a (modified) julian day to GPS
 */
static GTime
jul_str2gps(char *s)
{
	HTLdbl	day;
#ifdef USE_LONG_DOUBLE
	sscanf(s, "%Lf", &day);
#else
	sscanf(s, "%lf", &day);
#endif
	if (day >   50000 && day < 70000) return(Ht_modjul_to_GPS(day));
	else                              return(Ht_julday_to_GPS(day));
}

/*
 *  Convert doy/sod to GPS
 *  Jan 1 is day 1.
 */
static GTime
sod_str2gps(char *s, int type)
{
	static char	eve[40];
	int		y, d;
	HTLdbl		secs, gps;
#ifdef USE_LONG_DOUBLE
	sscanf(s, "%d/%d/%Lf", &y, &d, &secs);
#else
        sscanf(s, "%d/%d/%lf", &y, &d, &secs);
#endif
	if (type == Ht_STR_Y2DS) if (y < 50) y += 2000;
	sprintf(eve, "%04d0101_000000", y);
	gps = Ht_string_to_GPS(eve, 0);
	gps += (d-1) * 86400.0;
	gps += secs;
        gps += Ht_leapsecs_from_year_start_to_gps(y,gps);
	return(gps);
}
/*
 * simple variation on this theme
 */
static GTime
file_plan_str2gps(char *s, int type)
{
	static char	eve[40];
	int		y, d, hr, mn, ss, ms;
	HTLdbl		gps;
	if (type == Ht_STR_FILE) {
		sscanf(s, "%d_%d_%d", &y, &d, &hr);
		mn = hr % 100;
		hr = (hr - mn) / 100;
		ms = ss = 0;
	} else if (type == Ht_STR_FILH) {
		sscanf(s, "%d_%d_%d_%d", &y, &d, &hr, &mn);
		ms = ss = 0;
	} else if (type == Ht_STR_YDOY) {
		sscanf(s, "%d/%d.%8d", &y, &d, &hr);
		ms = 17 - strlen(s);
		if ( ms <= 0 ) {
		    gps = ((HTLdbl)hr+0.5) / 100000000;
		} else if ( ms < 9 ) {
		    gps = ((HTLdbl)hr+0.5) / 100000000;
		    while (ms--) gps *= 10;
		} else {
		    gps = 0.5;
		}
		hr = floor(24 * gps);
		mn = floor(60 * (24*gps - hr));
		ss = floor(60 * (60*(24*gps - hr) - mn));
		ms = floor(1000*(60*(60*(24*gps - hr) - mn) - ss));
	} else if (type == Ht_STR_PLAN) {
		sscanf(s, "%d/%d.%d", &y, &d, &hr);
		ss = hr % 100;
		hr = (hr - ss) / 100;
		mn = hr % 100;
		hr = (hr - mn) / 100;
		ms = 0;
	} else {
		/* shut up the compiler */
		y = 1970; d = 1; hr = 1; mn = 0; ms = ss = 0;
	}
	sprintf(eve, "%04d0101_000000", y);
	gps = Ht_string_to_GPS(eve, 0);
	gps += (d-1) * 86400.0;
	gps += hr * 3600.0;
	gps += mn * 60.0;
	gps += ss * 1.0;
	gps += ms * 0.001;
        gps += Ht_leapsecs_from_year_start_to_gps(y,gps);
	return(gps);
}

/*
 * A variation for MAESTRO, similar to the above
 * gtla = maes_dc_str2ut(s, type);
 */
static GTime
maes_dc_str2ut(char *s, int type)
{
	static char	eve[40], now[40];
	int		y, d, hr, mn, ss;
	HTLdbl		gps;
	if (type == Ht_STR_MY4D || type == Ht_STR_MY2D) {
		sscanf(s, "%d.%d.%d.%d.%d", &y, &d, &hr, &mn, &ss);
	} else if (type == Ht_STR_MY4C || type == Ht_STR_MY2C) {
		sscanf(s, "%d:%d:%d:%d:%d", &y, &d, &hr, &mn, &ss);
	} else if (type == Ht_STR_MY0C) {
		sscanf(s, "%d:%d:%d:%d", &d, &hr, &mn, &ss);
	} else if (type == Ht_STR_MY0D) {
		sscanf(s, "%d.%d.%d.%d", &d, &hr, &mn, &ss);
		/* grab current year */
		gps = Ht_now_GPS(0);
		strcpy(now, Ht_GPS_to_string(gps, Ht_DS_Y2KP));
		now[4] = 0;
		y = atoi(now);
	} else {
		/* shut up the compiler */
		printf ("type is %04X for %s\n", type, s);
		y = 1970; d = 1; hr = 1; mn = 0; ss = 0;
	}
	sprintf(eve, "%04d0101_000000", y);
	gps = Ht_string_to_GPS(eve, 0);
	gps += (d-1) * 86400.0;
	gps += hr * 3600.0;
	gps += mn * 60.0;
	gps += ss * 1.0;
        gps += Ht_leapsecs_from_year_start_to_gps(y,gps);
	return(gps);
}

/*
 *  Convert a UNIX date string to UT
 */
static UTime
unix_str2ut(char *s)
{
	static UTime		ut;
	static long		usec;

	switch (sscanf(s, "%ld.%ld", &ut.tv_sec, &usec)) {
	case 2:
		ut.tv_usec = usec % 1000000;
		break;
	default:
	case 0:
		ut.tv_sec = 0;
	case 1:
		ut.tv_usec = 0;
		break;
	}
	return(ut);
}

/*
 *  Convert a PASS date string to UT
 *  This was once static, but it's a useful function,
 *  and is now needed for leapsecond support.
 */
UTime
pass_str2ut(char *s)
{
	static UTime		ut;
	static struct tm	tm;
	static char		b[3];
	int			century;

	b[2] = 0;

	b[0] = s[0]; b[1] = s[1]; s += 2;
	tm.tm_year = atoi(b);
	if (s[4] == '_') {	/* we've got the y2k choice */
		century = (tm.tm_year < 50) ? 100 : 0;
	} else {		/* handle the century */
		century = (tm.tm_year - 19) * 100;
		b[0] = s[0]; b[1] = s[1]; s += 2;
	}
	tm.tm_year = atoi(b);
	tm.tm_year += century;	/* tm.tm_year is now (year - 1900) */

	b[0] = s[0]; b[1] = s[1]; s += 2;
	tm.tm_mon  = atoi(b) - 1;
	b[0] = s[0]; b[1] = s[1]; s += 3;
	tm.tm_mday = atoi(b);
	b[0] = s[0]; b[1] = s[1]; s += 2;
	tm.tm_hour = atoi(b);
	b[0] = s[0]; b[1] = s[1]; s += 2;
	tm.tm_min  = atoi(b);
	b[0] = s[0]; b[1] = s[1]; s += 2;
	tm.tm_sec  = atoi(b);
	tm.tm_isdst = 0;
	ut.tv_sec  = mktime(&tm);
	ut.tv_usec = (*s == '.') ? (int)(1000000. * atof(s)) : 0;

	return(ut);
}
/*
 *  And a variant
 */
static UTime
svn_str2ut(char *s)
{
	static UTime		ut;
	static struct tm	tm;
	static char		temp[50];
	char			zp;

	strncpy(temp, s, sizeof(temp));
	s = temp;

	s[4] = 0;	tm.tm_year = atoi(s) - 1900;	s += 5;
	s[2] = 0;	tm.tm_mon = atoi(s) - 1;	s += 3;
	s[2] = 0;	tm.tm_mday = atoi(s);		s += 3;
	s[2] = 0;	tm.tm_hour = atoi(s);		s += 3;
	s[2] = 0;	tm.tm_min = atoi(s);		s += 3;
	zp = s[2];
	s[2] = 0;	tm.tm_sec = atoi(s);		s += 3;
	tm.tm_isdst = 0;
	ut.tv_sec  = mktime(&tm);
	*--s = '.';
	ut.tv_usec = (zp == '.') ? (int)(1000000.0 * atof(s)) : 0;

	return(ut);
}
/*
 *  A MAESTRO oriented version -- same idea as svn_str2ut()
 */
static UTime
maes_str2ut(char *s)
{
	static UTime		ut;
	static struct tm	tm;
	static char		temp[50];

	strncpy(temp, s, sizeof(temp));
	s = temp;

	s[4] = 0;	tm.tm_year = atoi(s) - 1900;	s += 5;
	s[2] = 0;	tm.tm_mon = atoi(s) - 1;	s += 3;
	s[2] = 0;	tm.tm_mday = atoi(s);		s += 3;
	s[2] = 0;	tm.tm_hour = atoi(s);		s += 3;
	s[2] = 0;	tm.tm_min = atoi(s);		s += 3;
	s[2] = 0;	tm.tm_sec = atoi(s);		s += 3;
	tm.tm_isdst = 0;
	ut.tv_sec  = mktime(&tm);
	ut.tv_usec = 0;
	return(ut);
}
/*
 *  And yet another variant
 */
static UTime
iso_str2ut(char *s)
{
	static char	temp[50];
	strncpy(temp, s, sizeof(temp));
	temp[10] = ' ';
	return(svn_str2ut(temp));
}
static UTime
plog_str2ut(char *s)
{
	static char	temp[50];
	strncpy(temp, s, sizeof(temp));
	temp[10] = ' ';
	temp[19] = '.';
	temp[23] = temp[24] = temp[25] = '0';
	temp[26] = 0;
	return(svn_str2ut(temp));
}
/*
 *  Can't have too many
 */
static UTime
pigmi_str2ut(char *s)
{
	static char	temp[50];
	strncpy(temp, s, sizeof(temp));
	temp[8] = ' ';
	strncpy(temp+9, s+8, sizeof(temp)-9);
	temp[15] = '.';
	strncpy(temp+16, s+14, sizeof(temp)-16);
	temp[19] = '0';
	temp[20] = '0';
	temp[21] = '0';
	return(pass_str2ut(temp));
}

/*
 *  Convert a DATE date string to UT
 *  sget_day() and sget_mon() advance s past what they have parsed.
 *  Original code expected:  Sat Dec 14 22:23:25[.000000] UTC 2019
 *  But we should now allow: Sat 14 Dec 2019 10:23:34[.000000] PM UTC
 *  ptyp is a pointer to an integer to hold Ht_STR_ADAT or Ht_STR_UTDT
 */
static UTime
date_str2ut(char *s, int *ptyp)
{
	static UTime		ut;
	static struct tm	tm;
        //fprintf(stderr, "START: %s\n", s);

	s = sget_day(s, &tm);	/* day of week is optional */
        //fprintf(stderr, "  DOW: %s\n", s);
        if (*s == ' ') s++;     /* space-padded day */
        if (isdigit(*s)) {
            //fprintf(stderr, "DIGIT: %s\n", s);
            tm.tm_mday = atoi(s);
            while (isdigit(*s) || *s == ' ') s++;
            s = sget_mon(s, &tm);
            tm.tm_year = atoi(s) - 1900;
            s += 4;
        } else {
            //fprintf(stderr, "MONTH: %s\n", s);
            s = sget_mon(s, &tm);
            tm.tm_mday = atoi(s);
            s += 3;
            tm.tm_year = -6900; /* flag for below */
        }
        //fprintf(stderr, " NEXT: %s\n", s);

	/* : required */
	while (*s && *s != ':') s++;
	s -= 2;

	tm.tm_hour = atoi(s);	s += 3;
	tm.tm_min  = atoi(s);	s += 3;
	tm.tm_sec  = atoi(s);	s += 2;
	ut.tv_usec = (*s == '.') ? (int)(1000000. * atof(s)) : 0;

	/* GMT or UTC is required; but watch for AM or PM */
	while (!(*s == 'G' || *s == 'U' || *s == 'A' || *s == 'P')) s++;
        if (*s == 'P') tm.tm_hour += 12;
        if (*s == 'P' || *s == 'A') s += 3;
        else                        s += 4;
        if (tm.tm_year == -6900) {
            tm.tm_year = atoi(s) - 1900;
            *ptyp = Ht_STR_ADAT;
        } else {
            *ptyp = Ht_STR_UTDT;
        }

	tm.tm_isdst = 0;
	ut.tv_sec = mktime(&tm);

	return(ut);
}

/*
 * VLBI EXperiment (VEX) Time
 */
static GTime
vex_str2gt(char *s)
{
	static char	eve[40];
	HTLdbl		gps, secs;
	int		y, xx, left = strlen(s);
	y = atoi(s);	s += 5; left -= 5;
	if (s[2] == 'y') {
	    if      (y < 50) y += 2000;
	    else if (y > 50) y += 1900;
	    else             y += 2000;   /* ambiguous */
	    s -= 2; left += 2;
	}
	sprintf(eve, "%04d0101_000000", y);
	gps = Ht_string_to_GPS(eve, 0);
	if (left <= 0) return(gps);
	xx = atoi(s);	s += 4; left -= 4;
	gps += (xx-1) * 86400.0;
	if (left <= 0) return(gps);
	xx = atoi(s);	s += 3; left -= 3;
	gps += xx     *  3600.0;
	if (left <= 0) return(gps);
	xx = atoi(s);	s += 3; left -= 3;
	gps += xx     *    60.0;
	secs = atof(s);
	gps += secs;
        gps += Ht_leapsecs_from_year_start_to_gps(y,gps);
	return(gps);
}
static GTime
rdbe_str2gt(char *s)
{
        static char     t[20];
        int             ii = 0, jj = 0;
        while (ii <  4) t[ii++] = s[jj++];
        t[ii++] = 'y';
        while (jj <  7) t[ii++] = s[jj++];
        t[ii++] = 'd';
        while (jj <  9) t[ii++] = s[jj++];
        t[ii++] = 'h';
        while (jj < 11) t[ii++] = s[jj++];
        t[ii++] = 'm';
        while (jj < 13) t[ii++] = s[jj++];
        t[ii++] = 's';
        return(vex_str2gt(t));
}
static GTime
vdif_str2gt(char *s)
{
	static char	eve[40];
	int	ep = 0;
	HTLdbl	se = 0.0, gps;
#ifdef USE_LONG_DOUBLE
	sscanf(s, "%d@%Lf", &ep, &se);
#else
        sscanf(s, "%d@%lf", &ep, &se);
#endif
	ep &= 0x3F;	/* only 6-bits are legal */
	if (ep & 0x1) sprintf(eve, "%04d0701_000000", ep / 2 + 2000);
	else          sprintf(eve, "%04d0101_000000", ep / 2 + 2000);
	gps = Ht_string_to_GPS(eve, 0);
	gps += se;
        /* no need for Ht_leapsecs_from_year_start_to_gps() */
	return(gps);
}

/*
 *  Convert a string to a time delta: sign is optional at this point.
 *  Then compute and return GPS time relative to Ht_gps_origin.
 */
static GTime
rltv_str2ut(char *s, HTLdbl *err)
{
	static long	sg, m_s, m_us;
	GTime		gps, dtee;
	int		ns, nc;

	gps = (Ht_gps_origin != 0.0)
	    ? Ht_gps_origin
	    : Ht_now_GPS(err);

	/* before or after now? */
	if      (*s == '-') { sg = -1; s++; }
	else if (*s == '+') { sg =  1; s++; }
	else { return(gps); }

	while (isspace(*s)) s++;
#ifdef USE_LONG_DOUBLE
	ns = sscanf(s, "%Lf%n", &dtee, &nc);
#else
	ns = sscanf(s, "%lf%n", &dtee, &nc);
#endif
	if (ns == 0) {
		dtee = atoi(s);		/* value */
		while (isdigit(*s) || isspace(*s)) s++;
	} else {
		s += nc;
	}

	switch (*s) {
	/* (s)i(deral) == 27.321661 days by fixed stars */
	case 'i':	m_s = 2360591;		m_us = 510400;	break;
	/* (sy)n(odic) == 29.530589 days by lunar phase */
	case 'n':	m_s = 2551442;		m_us = 889600;	break;
			/* a year == 3600*24*365.25 */
	case 'y':	m_s = 60*60*24*1461/4;	m_us = 0;	break;
	case 'd':	m_s = 60 * 60 * 24;	m_us = 0;	break;
	case 'h':	m_s = 60 * 60;		m_us = 0;	break;
	case 'u':	m_s = 0;		m_us = 1;	break;
	case 'm':
			if (s[1] == 's') {
				m_s = 0;	m_us = 1000;
			} else {
				m_s = 60;	m_us = 0;
			}
			break;
	default:
	case 's':	m_s = 1;		m_us = 0;	break;
	}

	gps += sg * dtee * m_s;
	gps += (sg * dtee * m_us) * 1e-6;

	return(gps);
}

/*
 * Setup for relative time output
 */
int Ht_delta_setup(char *type, char *base)
{
    switch (*type) {
    default:					return(1);
    case 's':   Ht_d_mult =     1.0;		break;
    case 'm':   Ht_d_mult = (type[1] == 's')
			  ?     0.001
			  :    60.0;		break;
    case 'h':   Ht_d_mult =  3600.0;		break;
    case 'd':   Ht_d_mult = 86400.0;		break;
    case 'u':   Ht_d_mult =     0.000001;	break;
    case 'i':	Ht_d_mult =    27.321661*3600;	break;
    case 'n':	Ht_d_mult =    29.530589*3600;	break;
    case 'y':   Ht_d_mult =    60*60*24*1461/4; break;
    }
    Ht_d_mult = 1.0 / Ht_d_mult;
    Ht_d_orig = Ht_string_to_GPS(base, 0);
    return(0);
}
/*
 * And compute it
 */
HTLdbl Ht_delta_value(char *s, HTLdbl *pterror)
{
    HTLdbl  gps_delta = Ht_string_to_GPS(s, pterror);
    gps_delta -= Ht_d_orig;
    gps_delta *= Ht_d_mult;
    if (pterror) *pterror *= Ht_d_mult;
    return(gps_delta);
}

#if HETE2_PECULIAR
/*
 *  Stuff timestamp into the correct bits; add epoch.
 */
static HTime
ts2ht(IPP_word *ts)
{
	static HTime	ht;

	if (Ht_hete_epoch == EPOCH_NULL) ht = Ht_now_HTC(0);
	assert(!THIS_IS_IBEX);

	ht.time[3] = Ht_hete_epoch;
	ht.time[2] = ts[1];
	ht.time[1] = ts[0];
	ht.time[0] = 0x8000;	/* halfway */
	return(ht);
}
#endif /* HETE2_PECULIAR */

/*
 *  Scan input string for HETE sc clock
 *  If only 3 words scanned, and the first doesn't look like
 *  an epoch, force it to the IBEX epoch.
 */
static HTime
hex_str2ht(char *s, char *fmt)
{
	static HTime	ht;
	int		n;

	ht.time[0] = ht.time[1] = ht.time[2] = ht.time[3] = 0;
	n = sscanf(s, fmt, ht.time+3, ht.time+2, ht.time+1, ht.time+0);
	if ((n < 4) && (ht.time[3] & 0xFF00)) {
		ht.time[0] = ht.time[1];
		ht.time[1] = ht.time[2];
		ht.time[2] = ht.time[3];
		ht.time[3] = EPOCH_IBEX(0);
	}

	return(ht);
}

/*
 *  Given ptr to UNIX time, cough up a GMT time string.
 */
static char *
gtime(UTime *ut, int alt)
{
	static char	string[26 + 8 + 4 + 100 + 2], gmt[40], hh[4];
        struct tm       *tmp = gmtime(&(ut->tv_sec));
	char		apm = 'A';
	long		usec;

        memset(string, 0, sizeof(string));  /* clean slate */
	strncpy(gmt, asctime(tmp), 40);     /* private copy */
	gmt[24] = '\0';			    /* toss newline */

        usec = ut->tv_usec;
        if (alt == 0) {
            strncpy(string, gmt, 19);
            sprintf(string+19, ".%06ld UTC%s", usec, gmt+19);
        } else {
            if (tmp->tm_hour > 11) {
                snprintf(hh, 4, "%02d", tmp->tm_hour - 12);
                gmt[11] = hh[0];
                gmt[12] = hh[1];
                apm = 'P';
            }
            strncpy(string, gmt, 4);        /* dow */
            strncpy(string+4, gmt+8, 2);    /* dom */
            string[6] = ' ';
            strncpy(string+7, gmt+4, 4);    /* month */
            strncpy(string+11, gmt+20, 4);  /* year */
            string[15] = ' ';
            strncpy(string+15, gmt+10, 9);  /* hh:mm:ss */
            sprintf(string+24, ".%06ld %cM UTC", usec, apm);
        }

	return(string);
}

/*
 * This prints out yr/doy/sod things
 */
static void
yr_doy_sod(UTime ut, char *date, int style)
{
	struct tm	*tm = gmtime(&ut.tv_sec);
	HTLdbl		sod = tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
	sod += 1e-6 * ut.tv_usec;
	if (style == Ht_DS_Y2DS) {
#ifdef USE_LONG_DOUBLE
		sprintf(date, "%02d/%03d/%.6Lf", 1900 + tm->tm_year % 100,
			tm->tm_yday + 1, sod);
#else
sprintf(date, "%02d/%03d/%.6f", 1900 + tm->tm_year % 100,
    tm->tm_yday + 1, sod);
#endif
	} else if (style == Ht_DS_Y4DS) {
#ifdef USE_LONG_DOUBLE
		sprintf(date, "%04d/%03d/%.6Lf", 1900 + tm->tm_year,
			tm->tm_yday + 1, sod);
#else
sprintf(date, "%04d/%03d/%.6f", 1900 + tm->tm_year,
    tm->tm_yday + 1, sod);
#endif
	} else if (style == Ht_DS_RDBE) {
		sprintf(date, "%04d%03d%02d%02d%02d",
		    1900 + tm->tm_year, tm->tm_yday + 1,
		    tm->tm_hour, tm->tm_min, tm->tm_sec);
	} else if (style == Ht_DS_VEXT) {
		sprintf(date, "%04dy%03dd%02dh%02dm%02d.%04ds",
		    1900 + tm->tm_year, tm->tm_yday + 1,
		    tm->tm_hour, tm->tm_min, tm->tm_sec,
		    (int)(1e-2 * (ut.tv_usec + 0.5)));
	} else if (style == Ht_DS_FILH) {
		sprintf(date, "%04d_%03d_%02d_%02d", 1900 + tm->tm_year,
			tm->tm_yday + 1, tm->tm_hour, tm->tm_min);
	} else if (style == Ht_DS_FILE) {
		sprintf(date, "%04d_%03d_%02d%02d", 1900 + tm->tm_year,
			tm->tm_yday + 1, tm->tm_hour, tm->tm_min);
	} else if (style == Ht_DS_PLAN) {
		sprintf(date, "%04d/%03d.%02d%02d%02d", 1900 + tm->tm_year,
			tm->tm_yday + 1, tm->tm_hour, tm->tm_min, tm->tm_sec);
	} else if (style == Ht_DS_YDOY) {
		sprintf(date, "%04d/%03d.%08d", 1900 + tm->tm_year,
			tm->tm_yday + 1,
			(int)floor(100000000*(sod/86400.0)+0.5));
	} else {
		sprintf(date, "what time is it, anyway?");
	}
}

/*
 * Generate the 6-mo epoch + seconds in epoch
 * It is possibly a sane thing to do to make sure
 * the seconds of epoch is positive....
 * Of course, you are screwed on pre J2000 dates, in
 * which case you end up with seconds before J2000.
 */
int Ht_vdif_epoch(void)
{
	static int  epo = -1;
	char	    *ve;
	if (epo >= 0) return(epo);
	ve = getenv("HOPS_VDIF_EPOCH");
	if (ve) epo = atoi(ve) & 0x3F;
	else	epo = 0;    /* J2000 */
	return(epo);
}
static void
vdif_time(UTime ut, char *date)
{
	static char eve[40];
	int	    ep = Ht_vdif_epoch();
	HTLdbl	    gps, gpe;
	do {
	    gps = Ht_UTC_to_GPS(ut, 0);
	    if (ep & 0x1) sprintf(eve, "%04d0701_000000", ep / 2 + 2000);
	    else          sprintf(eve, "%04d0101_000000", ep / 2 + 2000);
	    gpe = Ht_string_to_GPS(eve, 0);
	    gps -= gpe;
	} while (gps < 0 && --ep >= 0);
        if (ep < 0) ep = 0;
        /* no need for Ht_leapsecs_from_year_start_to_gps() */
#ifdef USE_LONG_DOUBLE
	sprintf(date, "%02d@%.4Lf", ep, gps);
#else
	sprintf(date, "%02d@%.4f", ep, gps);
#endif
    }

/*
 * This prints out yr?doy?hh?mm?ss things
 */
static void
yr_doy_hhmmss(UTime ut, char *date, int style)
{
	struct tm	*tm = gmtime(&ut.tv_sec);
	char		d = style == Ht_DS_MY4D ? '.' : ':';
	sprintf(date, "%04d%c%03d%c%02d%c%02d%c%02d",
		1900 + tm->tm_year, d, tm->tm_yday + 1, d,
		tm->tm_hour, d, tm->tm_min, d, tm->tm_sec);
}

/*
 *  New, HETE-2 externals follow
 */

HTLdbl	Ht_default_error = 0.0;

/*
 *  Ht_HTC_to_GPS, Ht_GPS_to_HTC are in lifetime.c.
 *  Ht_HTC_to_secs, Ht_secs_to_HTC are in lifetime.c.
 */

/*
 *  Ht_UTC_to_GPS and Ht_GPS_to_UTC are in leapsec.c
 */

/*
 *  Do these using Ht_UTC_to_GPS, Ht_GPS_to_UTC and
 *  Ht_HTC_to_GPS, Ht_GPS_to_HTC.  Errors propagate.
 */
HTime
Ht_UTC_to_HTC(UTime ut, HtErr *err)
{
	HtErr	terr;
	GTime	gps = Ht_UTC_to_GPS(ut, &terr);
	HTime	htc;

	if (!err) err = &Ht_default_error;
	htc = Ht_GPS_to_HTC(gps, err);
	*err += terr;
	return(htc);
}
UTime
Ht_HTC_to_UTC(HTime ht, HtErr *err)
{
	HtErr	terr;
	GTime	gps = Ht_HTC_to_GPS(ht, &terr);
	UTime	utc;

	if (!err) err = &Ht_default_error;
	utc = Ht_GPS_to_UTC(gps, err);
	*err += terr;
	return(utc);
}

/*
 *  Return the present time, based on UNIX internal clock.
 */
UTime
Ht_now_UTC(HtErr *err)
{
	UTime	utc;

	gettimeofday(&utc, 0);
	if (!err) err = &Ht_default_error;
	*err = 0.01;	/* assume 10 ms for now */
	return(utc);
}
GTime
Ht_now_GPS(HtErr *err)
{
	HtErr	terr;
	UTime	utc = Ht_now_UTC(&terr);
	GTime	gps;

	if (!err) err = &Ht_default_error;
	gps = Ht_UTC_to_GPS(utc, err);
	*err += terr;
	return(gps);
}
HTime
Ht_now_HTC(HtErr *err)
{
	HtErr	terr;
	GTime	gps = Ht_now_GPS(&terr);
	HTime	htc;

	if (!err) err = &Ht_default_error;
	htc = Ht_GPS_to_HTC(gps, err);
	*err += terr;
	return(htc);
}

#if HETE2_PECULIAR
/*
 *  Convert the timestamp to GPS time, based on message source:
 *	ground timestamps are based on bits from gettimeofday();
 *	flight timestamps are missing the epoch and low bits.
 *
 *  Note that time conversions on the ground are screwed if the data
 *  was taken in the last unix epoch and examined in the present.
 */
GTime
Ht_IPP_to_GPS(IPP_header *hdr, HtErr *err)
{
	GTime	gps;
	UTime	utc;
	HTime	htc;
	HTLdbl	x;
	int	ep;

	if (!err) err = &Ht_default_error;
	if (IPP_source(hdr) > LAST_SC_ADDRESS) {	/* ground */
		/* figure out the epoch: either 0x0023 or 0x0024 */
		gettimeofday(&utc, 0);
		ep = (utc.tv_sec*1E6 + utc.tv_usec) / 281474976710656.0;

		x  = 32768.0;				/* halfway */
		x += scalbn((HTLdbl)(IPP_time_lss(hdr)), 16);
		x += scalbn((HTLdbl)(IPP_time_mss(hdr)), 32);
		x += scalbn((HTLdbl)ep,                  48);

		utc.tv_usec = (int)fmod(x, 1.e+06);
		utc.tv_sec  = (int)(x/1.0e+06);

		gps = Ht_UTC_to_GPS(utc, err);
	} else {					/* flight */
		if (Ht_hete_epoch == EPOCH_NULL) htc = Ht_now_HTC(0);

		htc.time[0] = 0x8000;			/* halfway */
		htc.time[1] = IPP_time_lss(hdr);
		htc.time[2] = IPP_time_mss(hdr);
		htc.time[3] = Ht_hete_epoch;

		gps = Ht_HTC_to_GPS(htc, err);
	}

	*err += 0.033;
	return(gps);
}
#endif /* HETE2_PECULIAR */

/*
 * For debugging
 */
#define what_type(T,S)
#ifndef what_type
static void what_type(int T, char *S)
{
	if (getenv("testverb"))
		fprintf(stderr, "Input %s returned type %x\n", S, T);
}
#endif /* what_type */

/*
 * Estimate the length of a string of each type; the idea is that
 * after this length, a whitespace character is the end of it.
 */
static int
Ht_min_len(int type, int extra, char *orig)
{
	int	x = 0;
	if        ((type & 0x100) == 0x100) switch (type) {
	case Ht_STR_UTYP:	x = (10);		break;
	case Ht_STR_UTNX:	x = (10);		break;
	case Ht_STR_UTPS:	x = (15);		break;
	case Ht_STR_USVN:	x = (20);		break;
	case Ht_STR_UTDT:	x = (35);		break;
	case Ht_STR_ISOT:	x = (20);		break;
	case Ht_STR_ISOM:	x = (20);		break;
	case Ht_STR_MAES:	x = (19);		break;
	case Ht_STR_MY4D:	x = (20);		break;
	case Ht_STR_MY4C:	x = (20);		break;
	case Ht_STR_MY2D:	x = (18);		break;
	case Ht_STR_MY2C:	x = (18);		break;
	case Ht_STR_MY0D:	x = (15);		break;
	case Ht_STR_MY0C:	x = (15);		break;
	case Ht_STR_FILE:	x = (12);		break;
	case Ht_STR_FILH:	x = (13);		break;
	case Ht_STR_YDOY:	x = (16);		break;
	case Ht_STR_PLAN:	x = (14);		break;
	case Ht_STR_PLOG:	x = (24);		break;
	case Ht_STR_UTRL:	x = (0);		break;
	case Ht_STR_UGPS:	x = (9);		break;
	case Ht_STR_UWKS:	x = (11);		break;
	case Ht_STR_UGRB:	x = (12);		break;
	case Ht_STR_JDAY:	x = (7);		break;
	case Ht_STR_MODJ:	x = (5);		break;
	case Ht_STR_Y2DS:	x = (12);		break;
	case Ht_STR_Y4DS:	x = (14);		break;
	case Ht_STR_PGMI:	x = (17);		break;
	case Ht_STR_VEXT:	x = (23);		break;
	case Ht_STR_VDIF:	x = (16);		break;
	case Ht_STR_RDBE:	x = (14);		break;
	case Ht_STR_NULL:	x = (0);		break;
	} else if ((type & 0x200) == 0x200) switch (type) {
	case Ht_STR_HTYP:	x = (0);		break;
	case Ht_STR_HT0x:	x = (6);		break;
	case Ht_STR_HT0X:	x = (6);		break;
	case Ht_STR_HT00:	x = (9);		break;
	case Ht_STR_HT03:	x = (4);		break;
	case Ht_STR_HOPS:	x = (6);		break;
	case Ht_STR_NULL:	x = (0);		break;
	}
	what_type(type, orig);
	return(x + extra);
}

/*
 *  Which type:  Ht_STR_.... ?
 *
 *  This is where all the heuristics are buried.  Leading whitespace
 *  is skipped, then the various special characters are used to pick
 *  the type.  If we aren't sure Ht_STR_NTYP is a safe return.
 *
 *  Ht_asserted_string_type can be externally set to bypass the heuristics.
 */
#define RETURN(X)	return(Ht_string_min_len=Ht_min_len(X,s-so+xx,so),X)
int
Ht_string_type(char *s)
{
	static struct tm	ignore;
	register int		t, xx = 0;
	char			*so = s, *xs;

	if (Ht_asserted_string_type != Ht_STR_NTYP)
	    return(Ht_asserted_string_type);

	while (isspace(*s)) s++;	/* skip leading space */

	if (!*s)
		RETURN(Ht_STR_NULL);			/* null */
	if ('+' == *s || '-' == *s)
		RETURN(Ht_STR_UTRL);			/* relative */

        if (strlen(s) == 6) {                           /* HOPS rootcode */
            if (isalnum(s[0]) && isalnum(s[1]) && isalnum(s[2]) &&
                isalnum(s[3]) && isalnum(s[4]) && isalnum(s[5]))
                    return(Ht_STR_HOPS);
        }

	/* all the other cases that begin with a digit */
	if ('0' <= *s && *s <= '9') {
		if (s[6] == '_') RETURN(Ht_STR_UTPS);	/* pass 99...  */
		if (s[6] == '+') RETURN(Ht_STR_UGRB);	/* grb utc */
		if (s[2] == '/') {
		    if (s[6] == '/')
			RETURN(Ht_STR_Y2DS);	/* YY/DOY/SOD */
		    else if (s[6] == ':')
			RETURN(Ht_STR_MY2C);	/* YY:DOY:HH:MM:SS */
		    else if (s[6] == '.')
			RETURN(Ht_STR_MY2D);	/* YY.DOY.HH.MM.SS */
		}
		if (s[3] == ':' && s[6] == ':')
			RETURN(Ht_STR_MY0C);    /* DOY:HH:MM:SS */
		if (s[3] == '.' && s[6] == '.')
			RETURN(Ht_STR_MY0D);    /* DOY.HH.MM.SS */
		if (s[4] == ':' && s[8] == ':' && s[11] == ':') {
			RETURN(Ht_STR_MY4C);	/* YYYY:DOY:HH:MM:SS */
		}
		if (s[4] == '.' && s[8] == '.' && s[11] == '.') {
			RETURN(Ht_STR_MY4D);	/* YYYY.DOY.HH.MM.SS */
		}
		if (s[4] == 'y' || s[2] == 'y') {
			RETURN(Ht_STR_VEXT);	/* [YY]YYyDOYdHHhMMmSS.ffffs */
		}
		if (s[2] == '@' || s[1] == '@')
			RETURN(Ht_STR_VDIF);	/* EE@ssssssss.ffff (EE<64) */
		if (s[4] == '/') {
		    if (s[7] == ':')
			RETURN(Ht_STR_MAES);	/* YYYY/MM:DD:HH:MM:SS */
		    else if (s[8] == '/')
			RETURN(Ht_STR_Y4DS);	/* YYYY/DOY/SOD */
		    else if (s[8] == '.')
		      { int ll = strlen(s);
			RETURN( (isdigit(s[15]) || ll != 15)
			    ? Ht_STR_YDOY	/* YYYY/DOY.fr-day */
			    : Ht_STR_PLAN);	/* YYYY/DOY.HHMMSS */
		      }
		    else
		      { int ll = strlen(s);
			RETURN( (ll == 8)
			    ? Ht_STR_YDOY	/* YYYY/DOY */
			    : Ht_STR_NTYP);	/* unknown type */
		      }
		}
		if (s[4] == '_' && s[8] == '_') {
		    if (s[11] == '_')
			RETURN(Ht_STR_FILH);	/* YYYY_DOY_HH_MM */
		    else
			RETURN(Ht_STR_FILE);	/* YYYY_DOY_HHMM */
		}
		if (s[8] == '_') RETURN(Ht_STR_UTPS);	/* pass 1999...*/
		if (s[4] == '-') {
			if (s[19] == ',') RETURN(Ht_STR_PLOG);	/* PythonLog */
			if (s[10] == ' ') RETURN(Ht_STR_USVN);	/* SVN date */
			if (s[10] == 'T') {
			    t = strlen(s);
			    if (t < 18)
				RETURN(Ht_STR_ISON);	/* ISO time (s)  */
			    else if (t < 24)
				RETURN(Ht_STR_ISOM);	/* ISO time (us) */
			    else
				RETURN(Ht_STR_ISOT);	/* ISO time (us) */
			}
			/* short-cut to the start of each year */
			RETURN(Ht_STR_USVN);		/* SVN Y-M-D ? */
		}
		if (s[1] == 'x' || s[1] == 'X') {
			xx = 0;		/* more heuristics needed */
		}
		if (s[1] == 'x') RETURN(Ht_STR_HT0x);	/* clck */
		if (s[1] == 'X') RETURN(Ht_STR_HT0X);	/* clck */
		xx = 0;		/* more heuristics needed */
#if HETE2_PECULIAR
		if (s[4] == ' ') {
			RETURN( (s[9] == ' ' || s[9] == '\n')
				? Ht_STR_HT00	/* xxxx xxxx [xxxx xxxx] */
				: Ht_STR_HT0E);	/* xxxx xxxxxxxxxxxx */
		}
#endif /* HETE2_PECULIAR */
		if (s[4] == ' ') {
		    if (s[0] == '0' && s[1] == '0') {
			xx = 5;		/* more heuristics needed */
			RETURN(Ht_STR_HT00); /* 00xx xxxx [xxxx [xxxx]] */
		    } else {
			xx = 10;		/* more heuristics needed */
			RETURN(Ht_STR_HT03); /* NNxx [xxxx [xxxx]] */
		    }
		}
		xx = 0;

		if ((t = atoi(s)) == 0)
			RETURN(Ht_STR_NULL);		/* not a num */

		if (600 < t && t < MAX_WEEKS && '0' <= s[5] && s[5] <= '9')
			RETURN(Ht_STR_UWKS);		/* week?secs */
#if HETE2_PECULIAR
		if (strchr(s, '.') == NULL)
			RETURN(Ht_STR_HTTS);		/* timestamp */
#endif /* HETE2_PECULIAR */

                /* no fraction, no . and exactly 13 digits */
                if (strchr(s, '.') == NULL && strlen(s) == 13)
                    RETURN(Ht_STR_RDBE);

		/* secs without fraction is UNIX, unless it is PIGMI */
		if ((xs = strchr(s, '.')) == NULL || xs - s > 19) {

		    /* PIGMI is a very long string, no nondigits */
		    if (//s[16] >= '0' && s[16] <= '9' &&
			s[ 4] >= '0' && s[ 4] <= '9' &&
			s[10] >= '0' && s[10] <= '9' &&
			s[ 6] >= '0' && s[ 6] <= '9' &&
			s[12] >= '0' && s[12] <= '9' &&
			s[ 8] >= '0' && s[ 8] <= '9' &&
			//s[14] >= '0' && s[14] <= '9')
			s[13] >= '0' && s[13] <= '9')
			    RETURN(Ht_STR_PGMI);
		    else
			    RETURN(Ht_STR_UTNX);
		}

		/* secs.fraction is GPS, Julian or MJD */
		if (t >   50000 && t <   70000) RETURN(Ht_STR_MODJ);
		if (t > 2440000 && t < 2470000) RETURN(Ht_STR_JDAY);

		RETURN(Ht_STR_UGPS);	 	    	/* GPS  secs */
	}

        //no longer needed as it is handled above
	//if (strlen(s) == 6 && *s >= 'a' && *s <= 'z')
	//	return(Ht_STR_HOPS);		/* HOPS rootcode (orig) */

	//if (s != sget_day(s, &ignore))
	//	RETURN(Ht_STR_UTDT);		/* date starts with weekday */
	//if (s != sget_mon(s, &ignore))
	//	RETURN(Ht_STR_UTDT);		/* date starts with month */

        /* could be Ht_STR_UTDT or Ht_STR_ADAT at this point */
        /* the date may optionally start with weekday */
        if (s != sget_day(s, &ignore) || s != sget_mon(s, &ignore)) {
            UTime ignore;
            int   stype;
            ignore = date_str2ut(s, &stype);
            return(stype);
        }

	RETURN(Ht_STR_NTYP);			/* unknown type */
}
#undef RETURN

/*
 *  Common workhorse for all string conversions
 */
static GTime	gtla;
static HTime	htla;
static UTime	utla;
static HTLdbl	etla;

#define Ht_String_GPS	0x001
#define Ht_String_UTC	0x010
#define Ht_String_HTC	0x100

static void
string_to_TLA(char *s)
{
	int		type = Ht_string_type(s);
#if HETE2_PECULIAR
	int		stamp;
	static IPP_word	ts[2];
	static char	hold[20];
#endif /* HETE2_PECULIAR */
	HTLdbl		terr;

	if (Ht_tz_unset) {
		putenv("TZ=GMT");
		Ht_tz_unset = 0;
	}

	while (isspace(*s)) s++;

	switch (type) {
	case Ht_STR_UTNX:
		type = Ht_STR_UTYP;
	case Ht_STR_UTYP:
		utla = unix_str2ut(s);
		break;
	case Ht_STR_UTPS:
		type = Ht_STR_UTYP;
		utla = pass_str2ut(s);
		break;
	case Ht_STR_ADAT:
	case Ht_STR_UTDT:
		//type = Ht_STR_UTYP;
		utla = date_str2ut(s, &type);
		break;
	case Ht_STR_PGMI:
		type = Ht_STR_UTYP;
		utla = pigmi_str2ut(s);
		break;
	case Ht_STR_VEXT:
		type = Ht_STR_UGPS;
		gtla = vex_str2gt(s);
		break;
	case Ht_STR_VDIF:
		type = Ht_STR_UGPS;
		gtla = vdif_str2gt(s);
		break;
	case Ht_STR_RDBE:
		type = Ht_STR_UGPS;
		gtla = rdbe_str2gt(s);
		break;
	case Ht_STR_MAES:
		type = Ht_STR_UTYP;
		utla = maes_str2ut(s);
		break;
	case Ht_STR_MY4D: case Ht_STR_MY4C:
	case Ht_STR_MY2D: case Ht_STR_MY2C:
	case Ht_STR_MY0D: case Ht_STR_MY0C:
		gtla = maes_dc_str2ut(s, type);
		type = Ht_STR_UGPS;
		break;
	case Ht_STR_ISOM:
	case Ht_STR_ISOT:
		type = Ht_STR_UTYP;
		utla = iso_str2ut(s);
		break;
	case Ht_STR_PLOG:
		type = Ht_STR_UTYP;
		utla = plog_str2ut(s);
		break;
	case Ht_STR_USVN:
		type = Ht_STR_UTYP;
		utla = svn_str2ut(s);
		break;
	case Ht_STR_UGPS:
		type = Ht_STR_UGPS;
		gtla = sec_str2gps(s);
		break;
	case Ht_STR_UWKS:
		type = Ht_STR_UGPS;
		gtla = wks_str2gps(s);
		break;
	case Ht_STR_UGRB:
		type = Ht_STR_UGPS;
		gtla = grb_str2gps(s);
		break;
	case Ht_STR_HTYP:
		break;
#if HETE2_PECULIAR
	case Ht_STR_HTTS:
		type = Ht_STR_HTYP;
		stamp = (atoi(s) & 0xffffffff);
		ts[0] = stamp & 0xffff;
		ts[1] = (stamp>>16) & 0xffff;
		htla = ts2ht(ts);
		break;
#endif /* HETE2_PECULIAR */
	case Ht_STR_HT0x:
		type = Ht_STR_HTYP;
		htla = hex_str2ht(s, "0x%hx 0x%hx 0x%hx 0x%hx");
		break;
	case Ht_STR_HT0X:
		type = Ht_STR_HTYP;
		htla = hex_str2ht(s, "0X%hx 0X%hx 0X%hx 0X%hx");
		break;
	case Ht_STR_HT03:
	case Ht_STR_HT00:
		type = Ht_STR_HTYP;
		htla = hex_str2ht(s, "%hx %hx %hx %hx");
		break;
#if HETE2_PECULIAR
	case Ht_STR_HT0E:
		type = Ht_STR_HTYP;
		strncpy(hold, s, 9);
		hold[9] = ' ';
		strncpy(hold+10, s+9, 4);
		hold[14] = ' ';
		strncpy(hold+15, s+13, 4);
		hold[19] = ' ';
		htla = hex_str2ht(hold, "%hx %hx %hx %hx");
		break;
#endif /* HETE2_PECULIAR */
	default:
	case Ht_STR_NULL:
	case Ht_STR_UTRL:
		type = Ht_STR_UGPS;
		gtla = rltv_str2ut(s, &terr);
		etla += terr;
		break;
	case Ht_STR_JDAY:
	case Ht_STR_MODJ:
		type = Ht_STR_UGPS;
		gtla = jul_str2gps(s);
		break;
	case Ht_STR_YDOY:
	case Ht_STR_PLAN:
	case Ht_STR_FILH:
	case Ht_STR_FILE:
		gtla = file_plan_str2gps(s, type);
		type = Ht_STR_UGPS;
		break;
	case Ht_STR_Y2DS:
	case Ht_STR_Y4DS:
		gtla = sod_str2gps(s, type);
		type = Ht_STR_UGPS;
		break;
	case Ht_STR_HOPS:
		utla = hopsrc_str2ut(s);        /* HOPS rootcode */
		type = Ht_STR_UTYP;
		break;
	}

	/* create the other two values */
	switch (type) {
	case Ht_STR_HTYP:
		gtla = Ht_HTC_to_GPS(htla, &terr);
		etla += terr;
		utla = Ht_HTC_to_UTC(htla, &terr);
		etla += terr;
		break;
	case Ht_STR_UGPS:
		htla = Ht_GPS_to_HTC(gtla, &terr);
		etla += terr;
		utla = Ht_GPS_to_UTC(gtla, &terr);
		etla += terr;
		break;
	case Ht_STR_UTYP:
	default:
		gtla = Ht_UTC_to_GPS(utla, &terr);
		etla += terr;
		htla = Ht_UTC_to_HTC(utla, &terr);
		etla += terr;
		break;
	}
}

/*
 *  Front end wrappers.
 */
HTime
Ht_string_to_HTC(char *s, HtErr *err)
{
	if (!err) err = &Ht_default_error;
	string_to_TLA(s);
	*err = etla;
	return(htla);
}
GTime
Ht_string_to_GPS(char *s, HtErr *err)
{
	if (!err) err = &Ht_default_error;
	string_to_TLA(s);
	*err = etla;
	return(gtla);
}
UTime
Ht_string_to_UTC(char *s, HtErr *err)
{
	if (!err) err = &Ht_default_error;
	string_to_TLA(s);
	*err = etla;
	return(utla);
}

/*
 *  A temporary holder for time conversion errors.
 *  In practice, we'll ignore it for all but Ht_DS_SECS.
 */
static HTLdbl	temp_error = 0.0;

/*
 *  Dates are just easier to handle as UTC
 */
char *
Ht_UTC_to_string(UTime ut, int style)
{
	static char	date[80];
	struct tm	*tm;
	GTime		gps;
	HTime		htc;
	HTLdbl		terr;
	char		iso_svn;
	long		usec;
	int		msec;

	switch (style) {
	case Ht_DS_SECS:
		gps = Ht_UTC_to_GPS(ut, &terr);
		terr += temp_error;
		//sprintf(date, "%.6lf [%.1le]", gps, terr);
#ifdef USE_LONG_DOUBLE
		sprintf(date, "%.6Lf", gps);
#else
		sprintf(date, "%.6lf", gps);
#endif
		break;
	case Ht_DS_UNIX:
		sprintf(date, "%ld", ut.tv_sec + ((ut.tv_usec > 500000)?1:0 ));
		break;
	case Ht_DS_PASS:
		tm = gmtime(&ut.tv_sec);
		if (tm->tm_year > 99) tm->tm_year -= 100;
		usec = ut.tv_usec;
		sprintf(date, "%02d%02d%02d_%02d%02d%02d.%06ld",
			tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min,     tm->tm_sec, usec);
		break;
	case Ht_DS_Y2KP:
		tm = gmtime(&ut.tv_sec);
		usec = ut.tv_usec;
		sprintf(date, "%04d%02d%02d_%02d%02d%02d.%06ld",
			tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour,      tm->tm_min,     tm->tm_sec,
			usec);
		break;
	case Ht_DS_DATE:
		return(gtime(&ut, 0));
		break;
	case Ht_DS_ADAT:
		return(gtime(&ut, 1));
		break;
	case Ht_DS_HT00:
		htc = Ht_UTC_to_HTC(ut, &terr);
		sprintf(date, "%04X %04X %04X %04X",
			htc.time[3], htc.time[2],
			htc.time[1], htc.time[0]);
		break;
#ifdef HETE2_PECULIAR
	case Ht_DS_I_TS:
		htc = Ht_UTC_to_HTC(ut, &terr);
		sprintf(date, "%u", htc.time[1] | (htc.time[2] << 16));
		break;
#endif /* HETE2_PECULIAR */
	case Ht_DS_GRBS:
		tm = gmtime(&ut.tv_sec);
		if (tm->tm_year > 99) tm->tm_year -= 100;
#ifdef USE_LONG_DOUBLE
		sprintf(date, "%02d%02d%02d+%.6lf",
			tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour * 3600.0 +
			tm->tm_min * 60.0 +
			tm->tm_sec + ut.tv_usec * 1e-06);
#else
sprintf(date, "%02d%02d%02d+%.6Lf",
    tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
    tm->tm_hour * 3600.0 +
    tm->tm_min * 60.0 +
    tm->tm_sec + ut.tv_usec * 1e-06);
#endif
		break;
	case Ht_DS_JULD:
		gps = Ht_UTC_to_GPS(ut, &terr);
#ifdef USE_LONG_DOUBLE
		sprintf(date, "%.12Lf", Ht_GPS_to_julian(gps));
#else
		sprintf(date, "%.12lf", Ht_GPS_to_julian(gps));
#endif
		break;
	case Ht_DS_MJUL:
		gps = Ht_UTC_to_GPS(ut, &terr);
#ifdef USE_LONG_DOUBLE
		sprintf(date, "%.10Lf", Ht_GPS_to_modjul(gps));
#else
		sprintf(date, "%.10lf", Ht_GPS_to_modjul(gps));
#endif
		break;
	case Ht_DS_YDOY:
	case Ht_DS_PLAN:
	case Ht_DS_FILH:
	case Ht_DS_FILE:
	case Ht_DS_Y2DS:
	case Ht_DS_Y4DS:
	case Ht_DS_VEXT:
	case Ht_DS_RDBE:
		yr_doy_sod(ut, date, style);
		break;
	case Ht_DS_VDIF:
		vdif_time(ut, date);
		break;
	case Ht_DS_MY4D: case Ht_DS_MY4C:
		yr_doy_hhmmss(ut, date, style);
		break;
	case Ht_DS_USVZ:
	case Ht_DS_ISON:
		iso_svn = (style == Ht_DS_ISON) ? 'T' : ' ';
		if (ut.tv_usec > 500000) ut.tv_sec++;
		ut.tv_usec = 0;
		tm = gmtime(&ut.tv_sec);
		sprintf(date, "%04d-%02d-%02d%c%02d:%02d:%02d%s",
			tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday,
			iso_svn,
			tm->tm_hour,      tm->tm_min,     tm->tm_sec,
			Ht_suppress_Z ? "" : "Z");
		break;
	case Ht_DS_PGMI:
		tm = gmtime(&ut.tv_sec);
		msec = ut.tv_usec/1000;
		sprintf(date, "%04d%02d%02d%02d%02d%02d%03d",
			tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour,      tm->tm_min,     tm->tm_sec,
			msec);
		break;
	case Ht_DS_MAES:
		if (ut.tv_usec > 500000) ut.tv_sec++;
		ut.tv_usec = 0;
		tm = gmtime(&ut.tv_sec);
		sprintf(date, "%04d/%02d:%02d:%02d:%02d:%02d",
			tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour,      tm->tm_min,     tm->tm_sec);
		break;
	case Ht_DS_USVN:
	case Ht_DS_ISOT:
	case Ht_DS_ISOM:
		iso_svn = (style == Ht_DS_ISOT ||
		           style == Ht_DS_ISOM) ? 'T' : ' ';
		tm = gmtime(&ut.tv_sec);
		usec = ut.tv_usec;
		sprintf(date, "%04d-%02d-%02d%c%02d:%02d:%02d.%06ld%s",
			tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday,
			iso_svn,
			tm->tm_hour,      tm->tm_min,     tm->tm_sec,
			usec, Ht_suppress_Z ? "" : "Z");
		if (style == Ht_DS_ISOM) {
			date[23] = Ht_suppress_Z ? 0 : 'Z';
			date[24] = 0;
		}
		break;
	case Ht_DS_PLOG:
		tm = gmtime(&ut.tv_sec);
		usec = ut.tv_usec;
		sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d,%03ld",
			tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour,      tm->tm_min,     tm->tm_sec,
			usec / 1000);
		break;
	case Ht_DS_UWKS:
		gps = Ht_UTC_to_GPS(ut, &terr);
		terr = floor(gps / (7*86400.));
#ifdef USE_LONG_DOUBLE
		sprintf(date, "%.0Lf#%Lf%s", terr, ( gps - terr*(7*86400.) ) ,
			"             ");
#else
        sprintf(date, "%.0f#%lf%s", terr, ( gps - terr*(7*86400.) ) ,
            "             ");
#endif
		date[18] = 0;
		break;
	case Ht_DS_HOPS:
		ut2hopsrc_str(ut, date);        /* HOPS rootcode */
		break;
	case Ht_DS_UTRL:
		gps = Ht_UTC_to_GPS(ut, &terr);
		gps -= Ht_d_orig;
		gps *= Ht_d_mult;
#ifdef USE_LONG_DOUBLE
		sprintf(date, "%+Lf", gps);
#else
		sprintf(date, "%+f", gps);
#endif
		break;
	default:
		sprintf(date, "Undefined date style %d", style);
		break;
	}
	return(date);
}

/*
 *  Wrappers which do their work via Ht_UTC_to_string().
 */
char *
Ht_HTC_to_string(HTime ht, int style)
{
	HTLdbl	terr;
	UTime	ut = Ht_HTC_to_UTC(ht, &terr);
	char	*date;

	if (style == Ht_DS_HT00) {
		static char	cdate[40];
		sprintf(cdate, "%04X %04X %04X %04X",
			ht.time[3], ht.time[2],
			ht.time[1], ht.time[0]);
		return(cdate);
	} else if (style == Ht_DS_HT03) {
		static char	cdate[40];
		sprintf(cdate, "%04X %04X %04X",
			ht.time[2], ht.time[1], ht.time[0]);
		return(cdate);
	}

	temp_error = terr;
	date = Ht_UTC_to_string(ut, style);
	temp_error = 0.0;
	return(date);
}
char *
Ht_GPS_to_string(GTime gt, int style)
{
	HTLdbl	terr;
	UTime	ut = Ht_GPS_to_UTC(gt, &terr);
	char	*date;

	temp_error = terr;
	date = Ht_UTC_to_string(ut, style);
	temp_error = 0.0;
	return(date);
}

/*
 * eof
 */
