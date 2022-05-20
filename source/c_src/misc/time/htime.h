/*
 *  $Id: htime.h 3802 2008-12-21 01:12:04Z gbc $
 *
 *  Hete time utilities, hacked for Ibex, hacked for Hops.
 *
 *  Copyright (C) 1998-2010 by MIT & gbc.
 */

#ifndef HTIME_H
#define HTIME_H

/* set this nonzero for HETE */
#define HETE_PECULIAR 0

#include "hetetimecode.h"
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//now needs hops_config in order to determine if long HTLdbl is supported
#include "time_float_type.h"

//upper limit on number of weeks count, this is ~157 years
#define MAX_WEEKS 8192


/*
 * various environment variables are littered about, based on history:
 *  HETE_OPS          - root of ops tree for HETE
 *  HETE_TIME_EPOCH   - epoch of HETE digital watch
 *  IBEX_DATACALSC    - directory for s/c calibration
 *  IBEX_OPS          - root of ops tree for IBEX
 *  IBEX_TIME_EPOCH   - epoch of IBEX
 *  HOPS_VDIF_EPOCH   - epoch of VDIF times
 *  HOPS_JULIAN_EPOCH - epoch of julian dates/times
 * since the IBEX clock was never really calculated, most of this
 * machinery wasn't really used for IBEX--the IBEX clock is essentially
 * synonymous with GPS.
 *
 * and all this needs to become more general....
 */

/*
 * Basic calibration %s is HETE_OPS and %1x is the digital watch id.
#define TAI_UTC_DATA	"%s/data/calibration/sc/tai-utc.dat"
#define Ht_LIFETIMES	"%s/data/calibration/sc/gps-dw%1x.dat"
 */
#define TAI_UTC_DATA	"%s/%s/tai-utc.dat"
#define Ht_LIFETIMES	"%s/%s/gps-dw%1x.dat"
#define Ht_IBEXTIMES	"%s/%s/gps-sc%1x.dat"
#define Ht_DATACALSC	"data/calibration/sc"

/*
 * Time in the pizza boxes and on HETE is maintained as a
 * count of microseconds.  It is initialized at boot to the
 * value maintained by a digital watch.
 *
 * CF: hetetimecode.h
 */
typedef time_code	HTime;	/* S/C Clock: epoch + 3words */

/*
 * The epoch will switch between these cases:
 * HETE: 3 words are count of us on flight clock
 * IBEX: 2 words are 32-bit count of MET (~GPS) seconds, 1 word of MET ms.
 */

/*
 * Atomic time is encoded as HTLdbl precision floating point, representing
 * seconds relative to the GPS epoch of 0000.0 6 January 1980 UTC.
 * The resolution of this encoding is 1/8 microsecond for the time of
 * the HETE mission.
 *
 * CF: http://tycho.usno.navy.mil
 */

typedef HTLdbl		GTime;	/* Time in s since GPS epoch */
typedef HTLdbl		HtErr;	/* approximate error estimate */

/*
 * For operational convenience, the unix timeval structure is
 * useful for many timekeeping operations.  It currently differs
 * from the GPS standard only by some number of leap seconds:
 *
 *	As of 1 March 1999,
 *		 TAI is ahead of UTC   by 32 seconds.
 *		 TAI is ahead of GPS   by 19 seconds.
 *		 GPS is ahead of UTC   by 13 seconds.
 *	As of 19 Feb 2008 we have another ls (2006) so
 *		 TAI is ahead of UTC   by 33 seconds.
 *		 TAI is ahead of GPS   by 19 seconds.
 *		 GPS is ahead of UTC   by 14 seconds.
 *	As of Aug 2008 we'll have a new one in 2009 so
 *		 TAI is ahead of UTC   by 34 seconds.
 *		 TAI is ahead of GPS   by 19 seconds.
 *		 GPS is ahead of UTC   by 15 seconds.
 *
 * Expect the next leapsecond in January or July 2009, maybe....
 *
 * CF: sys/time.h
 */
#define TAI_GPS_DELTA   19/*seconds*/
#define JD_GPS_EPOCH    (2444239.5 + 5.0)/*days*/
#define JD_UNIX_EPOCH   (2440587.5)/*days*/
/*
 * To see ticking clocks:
 * http://www.leapsecond.com/java/gpsclock.htm
 * which expresses 4 time systems in a civilian time representation.
 *
 * http://tycho.usno.navy.mil/systime.html
 * is a good reference on time systems.  As is
 * http://en.wikipedia.org/wiki/Ephemeris_time
 * and other wiki articles.  For more general work TDT (terrestrial
 * dynamical time aka TT) is useful, and
 *
 * 1977 Jan 1.0003725 TT == 1977 Jan 1.0000000 TAI == JD 2443144.5
 *
 * so that TT  - TAI == 32.184 s  (0.0003725 day)
 * and     TAI - GPS == 19.000 s  (always)
 * and     TAI - UTC == 33.000 s  (as I type this in Feb 2008) abd
 * and     GPS - UTC == 14.000 s  (as I type this in Feb 2008), so
 *         TT  - UTC == 65.184 s  (")
 *         TT  - GPS == 51.184 s  (always)
 *
 * http://maia.usno.navy.mil/
 * has the current data.
 * https://cdf.gsfc.nasa.gov/html/CDFLeapSeconds.txt
 * is a copy
 */
#define TDT_TAI_DELTA	32.184/*seconds*/
#define TDT_GPS_DELTA	51.184/*seconds*/

/* this is a reference time where the above are correct */
#define J2000_EPOCH	630763200.000000
/* the SPICE ephemeris time origin lies at this time in gps epoch: */
#define SPICE_ET_ORIGIN	630763148.816000
/* i.e. 51.184 second earlier */

/* for breaking symmetry on similar large numbers */
#define SPICE_ET_TYPE_GPS   0
#define SPICE_ET_TYPE_TAI   1
#define SPICE_ET_TYPE_TDT   2
#define SPICE_ET_TYPE_TDB   3
#define SPICE_ET_TYPE_UTC   4

/*
 * HOPS rootcode is number of 4-sec periods since midnight Jan1 1979.
 *  -q K 19790101_000000
 *  -q K 2018-02-26T15:45:04
 */
#define HOPS_ROOT_EPOCH	 283996800
#define HOPS_ROOT_BREAK	1519659904

/*
 * Support routine for ephemeris time, returning leapseconds since
 * the J2000 ephemeris, and a convenience function pinning down
 * leapseconds within a year.  (Note, Jan 1 leap seconds are part
 * of the previous year--the calendar year starts a second later.)
 */
GTime Ht_leapsecs_since_J2000(GTime gt);
HTLdbl Ht_leapsecs_from_year_start_to_gps(int year, GTime gps);

/*
 * in the spice leap second kernel
 *   DELTET/DELTA_T_A =  32.184
 *   DELTET/DELTA_AT  = ( ... 32, 33, 34 ... )
 */

typedef struct timeval	UTime;	/* UNIX time: tv_sec,tv_usec */

/*
 *  Note that the Unix system time may be provided in HTime format:
 *
 *	0020 0000 0000 0000 = Thu Jan  1 00:00:00 1970
 *	0020 4000 0000 0000 = Sat Mar 25 10:52:24 1972
 *	0020 8000 0000 0000 = Mon Jun 17 21:44:48 1974
 *	0020 c000 0000 0000 = Thu Sep  9 08:37:12 1976
 *	0021 0000 0000 0000 = Sat Dec  2 19:29:36 1978
 *	0021 4000 0000 0000 = Tue Feb 24 06:22:00 1981
 *	0021 8000 0000 0000 = Thu May 19 17:14:25 1983
 *	0021 c000 0000 0000 = Sun Aug 11 04:06:49 1985
 *	0022 0000 0000 0000 = Tue Nov  3 14:59:13 1987
 *	0022 4000 0000 0000 = Fri Jan 26 01:51:37 1990
 *	0022 8000 0000 0000 = Sun Apr 19 12:44:01 1992
 *	0022 c000 0000 0000 = Tue Jul 12 23:36:25 1994
 *	0023 0000 0000 0000 = Fri Oct  4 10:28:50 1996	EPOCH_UNIX(3)
 *	0023 4000 0000 0000 = Sun Dec 27 21:21:14 1998	EPOCH_UNIX(3)
 *	0023 8000 0000 0000 = Wed Mar 21 08:13:38 2001	EPOCH_UNIX(3)
 *	0023 c000 0000 0000 = Fri Jun 13 19:06:02 2003	EPOCH_UNIX(3)
 *	0024 0000 0000 0000 = Mon Sep  5 05:58:26 2005	EPOCH_UNIX(4)
 *	0024 4000 0000 0000 = Wed Nov 28 16:50:51 2007	EPOCH_UNIX(4)
 *	0024 8000 0000 0000 = Sat Feb 20 03:43:15 2010	EPOCH_UNIX(4)
 *	0024 c000 0000 0000 = Mon May 14 14:35:39 2012	EPOCH_UNIX(4)
 *
 *  And since it's easy to get confused (2^20 us ~= 10^6 us)...
 *
 *    ctime(...)         lowest epoch byte:	0X	1X	2X
 *    0x00000000 = Thu Jan  1 00:00:00 1970	NULL	DW0	X
 *    0x10000000 = Tue Jul  4 21:24:16 1978	UNSYNC	DW1	X
 *    0x20000000 = Mon Jan  5 18:48:32 1987	HUBSYNC DW2	X
 *    0x30000000 = Sun Jul  9 16:12:48 1995	X	DW3	~UNIX(3)
 *    0x40000000 = Sat Jan 10 13:37:04 2004	X	DW4	~UNIX(4)
 *    0x50000000 = Fri Jul 13 11:01:20 2012	X	DW5	~UNIX(5)
 *    0x60000000 = Thu Jan 14 08:25:36 2021	X	DW6	~UNIX(6)
 *    0x70000000 = Wed Jul 18 05:49:52 2029	X	DW7	~UNIX(7)
 *    0x7FFFFFFF = Tue Jan 19 03:14:08 2038	X	X	X
 */

/*
 *  Epoch 003X will be used for IBEX--see hetetimecode.h
 */
#define THIS_IS_IBEX	((Ht_hete_epoch & 0x30) == 0x30)
#define EPOCH_IBEX(N)	(0x30|(N))

/*
 *  Processor lifetime management structure.
 *
 *  This is used to create/maintain a linked list of lifetimes.
 *  Each lifetime represents a piecewise linear mapping among the
 *  sc clock, UTC, and GPS time.  New lifetimes are begun when
 *  either the s/c has rebooted or a significant event occurs.
 */
typedef struct sc_life	HLife;	/* a particular HETE lifetime */
struct sc_life {
	GTime	start;		/* time of birth in GPS epoch */
	HTLdbl	span;		/* effective lifetime in secs */
	HTime	birth;		/* effective sc time of birth */
	HTLdbl	error;		/* start--birth error in secs */
	HTLdbl	dsc_dt;		/* effective d(sctime) / d(t) */
	HTLdbl	esc_dt;		/* error in  d(sctime) / d(t) */
	HLife	*last;		/* pointer to a previous life */
	HLife	*next;		/* pointer to subsequent life */
	char	comment[40];	/* describing  this  lifetime */
	HLife	*old;		/* points to subsumed records */
};

/* pointer to linked life list, or null if not available */
extern HLife	*Ht_lifetimes;
extern IPP_word	Ht_hete_epoch;
extern int	Ht_tz_unset;

/* how often (secs) to check calibration file; zero disables updates */
extern int	Ht_update_interval;

/* these are used by euridice */
extern int	Ht_dwab_dwdelta;
extern HTLdbl	Ht_dwab_dw_mult;
extern void	Ht_dump_lives(char *file);
extern HLife	*Ht_last_life;
extern char	*Ht_life_file;
extern GTime	Ht_gps_origin;		/* origin for relative times */
extern int	Ht_string_min_len;	/* the string is at least this long */

/* essential conversion primitives */
extern UTime	Ht_HTC_to_UTC(HTime ht, HtErr *err);
extern GTime	Ht_HTC_to_GPS(HTime ht, HtErr *err);

extern HTime	Ht_UTC_to_HTC(UTime ut, HtErr *err);
extern HTime	Ht_GPS_to_HTC(GTime gt, HtErr *err);

extern GTime	Ht_UTC_to_GPS(UTime ut, HtErr *err);
extern UTime	Ht_GPS_to_UTC(GTime gt, HtErr *err);

/* where the result gets stored if err is a null ptr */
extern HTLdbl	Ht_default_error;

/* current time in each format */
extern HTime	Ht_now_HTC(HtErr *err);
extern UTime	Ht_now_UTC(HtErr *err);
extern GTime	Ht_now_GPS(HtErr *err);

#if HETE2_PECULIAR
/* estimate the current time based on a message header */
extern GTime	Ht_IPP_to_GPS(IPP_header *hdr, HtErr *err);
#endif /* HETE2_PECULIAR */

/* this forces an initialization which normally occurs implicitly */
extern void	Ht_initialize(void);
extern void	Ht_reset(void);

/*
 * Adding a new time format requires:
 *
 * Ht_STR_XXXX  to define the string input format       htime.h 1 line
 * Ht_DS_XXXX   to define the output format             htime.h 1 line
 * Xxxx         some nickname for queries           ht_timeis.c 1 line
 * Xxxx         nickname and strcmp() for it            rtime.c 4 lines
 * str()        functions to encode/decode              htime.c varies
 * length       an Ht_min_len() entry                   htime.c 1 line
 */

/* types of date-strings we can parse */
#define Ht_STR_NTYP	0x000	/* unknown typ */
#define Ht_STR_UTYP	0x100	/* a UT string */
#define Ht_STR_UTNX	0x101	/* a UNIX time */
#define Ht_STR_UTPS	0x102	/* a PASS date */
#define Ht_STR_USVN	0x103	/* a SVN  date */
#define Ht_STR_UTDT	0x104	/* a DATE date */
#define Ht_STR_ISOT	0x105	/* an ISO time */
#define Ht_STR_ISON	0x106	/* an ISO (s)  */
#define Ht_STR_ISOM	0x107	/* an ISO (ms) */
#define Ht_STR_UTRL	0x108	/* relative UT */
#define Ht_STR_HOPS	0x109	/* UNIX baseX6 */
#define Ht_STR_PLOG	0x10A	/* Python  Log */
#define Ht_STR_ADAT	0x10B	/* Alt.DateFmt */
#define Ht_STR_UGPS	0x110	/* GPS in secs */
#define Ht_STR_UWKS	0x120	/* GPS wks+sec */
/* maestro formats */
#define Ht_STR_MAES	0x139	/* YYYY/MM:DD: */
#define Ht_STR_MY4D	0x13A	/* M. YYYY.HH. */
#define Ht_STR_MY2D	0x13B	/* Mae. YY.HH. */
#define Ht_STR_MY0D	0x13C	/* Maestr. HH. */
#define Ht_STR_MY4C	0x13D	/* M. YYYY:HH: */
#define Ht_STR_MY2C	0x13E	/* Mae. YY:HH: */
#define Ht_STR_MY0C	0x13F	/* Maestr. HH: */
#define Ht_STR_UGRB	0x140	/* GRB secs UT */
#define Ht_STR_JDAY	0x160	/* Julian Date */
#define Ht_STR_MODJ	0x161	/* Mod JulianD */
#define Ht_STR_PLAN	0x170	/* yyyydoy.hms */
#define Ht_STR_FILE	0x171	/* yyyy_doy_hm */
#define Ht_STR_FILH	0x172	/*  _doy_hh_mm */
#define Ht_STR_Y2DS	0x180	/*  yy/doy/sod */
#define Ht_STR_Y4DS	0x181	/*yyyy/doy/sod */
#define Ht_STR_YDOY	0x182	/*yyyy/doy.frd */
#define Ht_STR_PGMI	0x190	/* pygmi  time */
#define Ht_STR_VEXT	0x1A0	/* vlbi  (vex) */
#define Ht_STR_VDIF	0x1A1	/* vlbi (vdif) */
#define Ht_STR_RDBE	0x1A2	/* vlbi (rdbe) */
#define Ht_STR_HTYP	0x200	/* a HT string */
#if HETE2_PECULIAR
#define Ht_STR_HTTS	0x201	/* a timestamp */
#endif /* HETE2_PECULIAR */
#define Ht_STR_HT0x	0x202	/* sc_clock 0x */
#define Ht_STR_HT0X	0x204	/* sc_clock 0X */
#define Ht_STR_HT00	0x208	/* without 0xX */
#define Ht_STR_HT03	0x210	/* 3wds no 0xX */
#if HETE2_PECULIAR
#define Ht_STR_HT0E	0x210	/* epoch+clock */
#endif /* HETE2_PECULIAR */
#define Ht_STR_NULL	0x300	/* null string */

#if HETE2_PECULIAR
#define Ht_HETE_TIME_HELP "\n\
	...\n\
	time_code:	(epoch  time[2] time[1] time[0]:)\n\
			0x0000 [0x0000 [0x0000 [0x0000]]]\n\
			0X0000 [0X0000 [0X0000 [0X0000]]]\n\
			0000 [0000 [0000 [0000]]]\n\
			0000 000000000000\n\
	timestamp:	ticks\n\
			(the epoch is implicit)\n\
"
#endif /* HETE2_PECULIAR */

#define Ht_IBEX_TIME_HELP "\n\
	Parse-able IBEX time formats:\n\
\n\
	relative:	+|- integer [us|ms|s|m|h|d|y]\n\
			+|- float [us|ms|s|m|h|d|y]\n\
			+|- integer [(s)i(dereal)|(sy)n(odic)] (lunar months)\n\
\n\
	UNIX clock:	secs\n\
	GPS seconds:	secs.[usecs]\n\
	GPS time:	weeks#secs[.usecs]\n\
			(# == any non-space)\n\
	PASS time:	[19]990613_054700[.000000]\n\
	DATE string:	[Thu] Jun 13 05:47:00[.000000] UTC 1996\n\
			(GMT may replace UTC) or the variant\n\
                        [Thu] 13 Jun 1996 05:47:00 AM UTC\n\
	GRB time:	YYMMDD+SOD (UTC)\n\
	YDS time:	YYYY/DOY/SOD (UTC)\n\
			YY/DOY/SOD (UTC)\n\
	PLAN time:	YYYY/DOY.HHMMSS\n\
	FILE time:	YYYY_DOY_HHMM\n\
	or              YYYY_DOY_HH_MM\n\
	MAESTRO time:	YYYY/MM:DD:HH:MM:SS\n\
	or              [[YY]YY:]DOY:HH:MM:SS\n\
	or              [[YY]YY.]DOY.HH.MM.SS\n\
	SVN time:	YYYY-MM-DD HH:MM:SS[.uuuuuu][Z]\n\
	ISO time:	YYYY-MM-DDTHH:MM:SS[.uuuuuu][Z]\n\
	MPS time:	YYYY-MM-DDTHH:MM:SS[.mmm][Z]\n\
	PIGMI time:     YYYYMMDDHHMMSSmmm\n\
	VLBI vex:       [YY]YYyDOYdHHhMMmSS.ffffs\n\
	VLBI s/e:	EE@ssssssss.ffff (EE<64)\n\
\n\
	time_code:	(epoch  time[2] time[1] time[0]:)\n\
			0x0030 [0xXXXX [0xXXXX [0xXXXX]]] (w/epoch)\n\
			0xXXXX [0xXXXX [0xXXXX]] (no epoch)\n\
\n\
	Julian day:	days.[fday]    (both of these are restricted to\n\
	Modified JD:	days.[fday]     the epoch 50000 < MJD < 70000)\n\
\n\
	HOPS rootcode:	unix clock base 26\n\
\n\
"

/* in case you don't want to see the Z on output */
extern int		Ht_suppress_Z;

/* this returns one of these date-string defines */
extern int		Ht_string_type(char *s);

/* this forces the issue if you need it */
extern int		Ht_asserted_string_type;

extern HTime		Ht_string_to_HTC(char *s, HtErr *err);
extern GTime		Ht_string_to_GPS(char *s, HtErr *err);
extern UTime		Ht_string_to_UTC(char *s, HtErr *err);

/* more efficient for a special case */
extern UTime            pass_str2ut(char *s);

/* types of date-strings we can print... */
#define Ht_DS_UNIX	0	/* UNIX time secs */
#define Ht_DS_PASS	1	/* PASS datestamp */
#define Ht_DS_DATE	2	/* DATE datestamp */
#define Ht_DS_SECS	3	/* GPS  time secs */
#define Ht_DS_Y2KP	4	/* PASS w/century */
#define Ht_DS_HT00	5	/* s/c clock no0x */
#if HETE2_PECULIAR
#define Ht_DS_I_TS	6	/* IPP  timestamp */
#else /* HETE2_PECULIAR */
#define Ht_DS_YDOY	6	/* YEAR/DOY.f-day */
#endif /* HETE2_PECULIAR */
#define Ht_DS_GRBS	7	/* GRB seconds UT */
#define Ht_DS_HT03	8	/* s/c clock 3wds */
#define Ht_DS_JULD	9	/* the julian day */
#define Ht_DS_MJUL	10	/* mod julian day */
#define Ht_DS_Y2DS	11	/*  yy/doy/sec.ms */
#define Ht_DS_Y4DS	12	/*year/doy/sec.ms */
#define Ht_DS_UWKS      13	/* GPS weeks/sow  */
#define Ht_DS_USVN	14	/* SVN  datestamp */
#define Ht_DS_USVZ	15	/* SVN w/no usecs */
#define Ht_DS_ISON	16	/* ISO time no us */
#define Ht_DS_ISOT	17	/* ISO time w/ us */
#define Ht_DS_PGMI	18	/* pigmi   format */
#define Ht_DS_ISOM	19	/* ISO time w/ ms */
#define Ht_DS_MAES	20	/* for ze MAESTRO */
#define Ht_DS_PLAN	21	/* planning times */
#define Ht_DS_FILE	22	/* for file names */
#define Ht_DS_MY4D	23	/* MAESTRO  YYYY. */
#define Ht_DS_MY4C	24	/* MAESTRO  YYYY: */
#define Ht_DS_FILH	25	/* alt file names */
#define Ht_DS_HOPS	26	/* HOPS  rootcode */
#define Ht_DS_UTRL	27	/* relative UTime */
#define Ht_DS_VEXT	28	/* vlbi vex  time */
#define Ht_DS_VDIF	29	/* vlbi vdif time */
#define Ht_DS_RDBE	30	/* vlbi rdbe time */
#define Ht_DS_PLOG	31	/* Python logtime */
#define Ht_DS_ADAT	32	/* alternate date */
#define Ht_DS_NUMB	33	/* count of these */

/* supplied as style argument to one of these */
extern char		*Ht_HTC_to_string(HTime ht, int style);
extern char		*Ht_UTC_to_string(UTime ut, int style);
extern char		*Ht_GPS_to_string(GTime gt, int style);

/* these are useful: straightforward conversions */
extern HTLdbl		Ht_HTC_to_secs(HTime ht);
extern HTime		Ht_secs_to_HTC(HTLdbl s);

#if HETE_PECULIAR
/* MET was the 3 lower words, with epoch 0019 implicit */
extern HTLdbl		Ht_HTC_to_MET(HTime ht);
extern HTime		Ht_MET_to_HTC(HTLdbl s);
#endif /* HETE_PECULIAR */

/*
 *  Ht_julday returns the Julian Day Number which begins at Noon of the
 *  calendar date specified by month mm, day id, and year iyyy, all
 *  being integer variables.  The year after 1 BC (-1) is 1 AD (1).
 */

extern int		Ht_julday(int mm, int id, int iyyy);
extern GTime		Ht_julday_to_GPS(HTLdbl julian);

extern GTime		Ht_julian_to_GPS(HTLdbl julian);
extern HTLdbl		Ht_GPS_to_julian(GTime gps);
extern GTime		Ht_modjul_to_GPS(HTLdbl modjul);
extern HTLdbl		Ht_GPS_to_modjul(GTime gps);

/*
 *  The float point julian day here is really in the GPS epoch,
 *  unless you've explicity shifted it via HOPS_JULIAN_EPOCH.
 *  (This time can be specified in any format and serves only
 *   to specify an alternate leap second offset GTime<->UTime.)
 *  This function then returns the number of leap-seconds of offset.
 *  (Same as Ht_leapsecs_since_J2000("HOPS_JULIAN_EPOCH"), but the
 *   evaluation takes place once--afterwards the previous value is used.)
 *  You can set HOPS_JULIAN_EPOCH to "GPS" to explicitly get the default.
 *  You can set HOPS_JULIAN_EPOCH to "UTC" to put julian days into UTC.
 *  (I.e. the julian days track the calendar.)  If you do that, this
 *  function returns Ht_JULIAN_EPOCH_IS_UTC which flags nonstandard handling.
 */
extern int		Ht_julian_epoch(void);
#define Ht_JULIAN_EPOCH_IS_UTC	(-666)

/*  Other potentially interesting epochs are
 *	~1958 when TAI starts and leap seconds = 0.
 *	 1972 TAI-UTC goes to integral leap seconds
 *       and some way to indicate Julian days of UTC
 */

/*
 *  The VDIF epochs are defined for 0 (J2000) through 63 (July 2031)
 *  and HOPS_VDIF_EPOCH should have one of those values.
 */
extern int		Ht_vdif_epoch(void);

/*
 *  Julian UTC is not yet implemented...
 */

/*
 *  For working with SPICE and solar system precision timing
 */
extern HTLdbl		Ht_GPS_to_TDT(HTLdbl gps);
extern HTLdbl		Ht_GPS_to_TAI(HTLdbl gps);
extern HTLdbl		Ht_TDT_to_GPS(HTLdbl tdt);
extern HTLdbl		Ht_TAI_to_GPS(HTLdbl tai);
extern HTLdbl		Ht_TDT_to_TDB(HTLdbl tdt);
extern HTLdbl		Ht_TDB_to_TDT(HTLdbl tdb);

extern int		Ht_ephem_type(char *);
extern char		*Ht_convert_et_to_gps(char *arg, int eph_type);
extern void		Ht_spice_sprintf(char *b, char *tla, HTLdbl t);

/*
 * And to generate relative timing;
 * the first character is s, m, h, d, ...
 * and the string is the origin.
 * Ht_d_mult, Ht_d_orig are visible as diagnostics only.
 */
extern int		Ht_delta_setup(char *type, char *base);
extern HTLdbl		Ht_delta_value(char *s, HTLdbl *pterror);
extern HTLdbl		Ht_d_mult, Ht_d_orig;

/*
 * For going from A-Z to _DS_ and _STR_ types.
 */
extern int		Ht_rtype_to_style(char *rtype);
extern int		Ht_qtype_to_style(char *rtype);
extern void		Ht_format_help(void);

#endif /* HTIME_H */

/*
 * eof
 */
