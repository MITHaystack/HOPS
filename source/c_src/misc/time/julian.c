/* Cribbed from seesat5 10/1/99 jpd */
/* Modified to avoid segfaults on unreasonable dates */

/*
UTIL.C
Paul S. Hirose, 1991 Dec 31
User interface support functions for SEESAT satellite tracking program.

This file is in the public domain.

100 - 199

Library functions and #defines used in this file:

atof, atoi, isalpha, NULL, printf, strcmp, strcpy, strlen, toupper
*/

#include "julian.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define ITOA(s, i) sprintf(s, "%d", i)

#if ECOC
extern void printf();
extern char *strcpy();
extern HTLdbl atof();
#endif


/* Use 0 for normal operation.  Non-zero will make static functions & data
externally visible for testing */
#define NSTAT 0

#if NSTAT		/* test mode */
#define STATIC		/* precedes static definitions of functions & data */
#define SC extern	/* precedes declarations of static functions */

#else			/* normal mode */
#define STATIC static
#define SC static
#endif

/*############################## DATA ##############################*/

struct month {
	char *mname;	/* month name */
	int difm;	/* days in full months */
};

/* calendar */
STATIC struct month calndr[] = {
	{"JAN",   0}, {"FEB",  31}, {"MAR",  59},
	{"APR",  90}, {"MAY", 120}, {"JUN", 151},
	{"JUL", 181}, {"AUG", 212}, {"SEP", 243},
	{"OCT", 273}, {"NOV", 304}, {"DEC", 334}, {(char *) 0, 365}
};

/*################ FUNCTIONS LOCAL TO THIS FILE ################*/

SC void cpy0p();	/* strncpy() with leading 0 padding */
SC int leapyr();	/* returns 1 if arg is leap year */

/*############################## CODE ##############################*/

HTLdbl
atomin(string)
char *string;
/* Returns value (in minutes) of string of form "hhmm:ss.s...".  Leading
minus sign permitted. */
{
	HTLdbl time;
	char *ptr, c, buf[5];
	int sign;

	if (*string == '-') {
		sign = -1;
		++string;
	} else
		sign = 1;
	ptr = string;

	/* point to the colon (or terminating null if no colon) */
	while ((c = *ptr)  &&  c != ':')
		++ptr;

	/* process seconds */
	if (c) {		/* string had a colon */
		time = atof(ptr + 1) / 60.;
		*ptr = '\0';		/* turn ':' to '\0' */
	} else
		time = 0.;
	cpy0p(buf, string, 5);		/* buf[] = "hhmm" */
	time += atof(buf + 2);		/* add minutes */
	buf[2] = '\0';
	/* add hours, restore sign */
	return (time + atof(buf) * 60.) * sign;
}


STATIC void
cpy0p(dest, src, n)
char *dest, *src;
int n;
/* Copy n-1 chars from src[] to dest[].  If src[] is too short, pad dest[]
with leading '0's.  If src[] is too long, not all of src[] will be copied. 
dest[] will always receive n-1 chars followed by null terminator. */
{
	int i, len;

	len = strlen(src);
	for (i = n; --i; )
		*dest++ = (i > len) ? '0' : *src++;
	*dest = '\0';
}


char **
degdms(pre, x)
int pre;		/* precision */
HTLdbl x;
/* Converts x degrees to 3 strings holding degrees, minutes, seconds.  "pre"
specifies rounding and may be 0 - 4:  0 = nearest deg, 1 = nearest 10 min, 2
= 1 min, 3 = 10 sec, 4 = 1 sec.  Domain of x (after rounding):  -1000 < x <
10000.  Returns pointer to dms[].  If pre was 1 or 2, s[] is garbage.  Both
s[] & m[] are garbage if pre was 0. */
{
	long int lx;
	int sign;
	static int mult[] = {1, 6, 60, 360, 3600};
	static char
		d[5], m[3], sec[3],
		*dms[] = {d, m, sec};

	if (x >= 0.)
		sign = 1;
	else {
		sign = -1;
		x = -x;
	}

	lx = (long) (x * mult[pre] + .5) * (((unsigned) pre & 1) ? 10 : 1);
	switch (pre) {
	case 4:
	case 3:
		ITOA(sec, (int) (lx % 60));	/* seconds */
		lx /= 60;
	case 2:
	case 1:
		ITOA(m, (int) (lx % 60));	/* minutes */
		lx /= 60;
	}
	ITOA(d, (int) lx * sign);	/* degrees */
	return dms;
}

char *jdstr(jd)
long int jd;	/* Julian Date, unit = days */
/* Converts jd to Gregorian calendar date.  Good for both BC & AD for
practically any date one might want.  More precisely, domain of jd is limited
by largest/smallest year an int can hold, as well as size of buffer[].
Returns pointer to year/month/day string, e.g., "1990 JAN 14". */
{
	static char buffer[50];		/* enough to convert 128 bit day # */
	struct month *calp;		/* pointer into calendar */
	int d, m, n;
	int leap;		/* flag; 1 = leap yr */
	char *cp;

	const signed char calc[12] = {0, 1, -1, 0, 0, 1, 1, 2, 3, 3, 4, 4};
	const char *daystr[8] = 
		{"SUN","MON","TUE","WED","THU","FRI","SAT","???"};
	int dow_year, dow_month, dow_day;

	/* 1721425 = Julian Date @ 1 BC Dec 31 (Gregorian calendar), 12h UT.
	365.2425 = days per year (over the 400 yr Gregorian cycle). */
	n = (jd - 1721425) / 365.2425;
	if (n >= 0)
		++n;

	/*  n = estimated year.  Correct the estimate to actual year.  The
	preceding formula can mistakenly yield an AD year from a BC jd, but
	the reverse will never happen.  Therefore, we need only guard against
	n being DECREMENTED to 0 (which is an undefined year). */

	while (1) {
		d = jd - julday(n, 0, 0);
		leap = leapyr(n);	/* true if leap year */
		if (d <= 365 + leap  &&  d >= 1)
			break;
		if (d < 1) {
			--n;
			if (n == 0)
				--n;	/* skip year 0 */
		} else
			++n;
	}

	dow_year = n;           /* Save the year */

	ITOA(buffer, n);	/* write the year */
	cp = buffer + strlen(buffer);
	*cp++ = ' ';

	/* Determine month.  Begin by moving forward thru the calendar a
	month at a time (starting with Feb) till we overshoot */

	calp = calndr;		/* point to Jan */
	m = 0;			/* month counter; 0 = Jan */
	do {
		++calp;		/* point to next month in calendar */
		++m;		/* month counter */
		n = calp->difm;
		if (m >= 2 && leap)	/* March or later, & leap yr */
			++n;
	} while (d > n);

	/* We've overshot by one month.  Decrement calendar pointer "calp".
	Month counter "m" has overshot too, so the "m >= 3" test actually
	means "March or later".  After subtracting days in full months, d =
	day of month. */
	d -= (--calp)->difm + (m >= 3 && leap);

	dow_month = m;        /* Save the month */
	dow_day = d;          /* Save the day */

	/* write month & day */
	strcpy(cp, calp->mname);
	cp += 3;
	*cp++ = ' ';
	ITOA(cp, d);
	
	/* The following doesn't work for BC. I don't care, and
	 * don't understand backward extrapolation of Gregorian
	 * dates well enough to fix it. If the DOW is undetermined,
	 * use "???" (the original code just crashed).
	 * 10/8/99 jpd
	 */

	n =    (   (  ((dow_year - 1) * 365L)
		    + ((dow_year  - 1) / 4)
		    - ( ((dow_year - 1) / 100)
		    -   ((dow_year - 1) / 400)
		      )
		    + (calc[dow_month - 1] + ((dow_month - 1) * 30))
		    + ( (   ((dow_year % 4 == 0) && (dow_year % 100 != 0))
			 || (dow_year % 400 == 0)
			)
			&& (dow_month > 2)
		      ) + dow_day
		   )
		 % 7);
	if( n < 0 || n > 7 ) n = 7;
	
	cp = buffer + strlen(buffer);
	*cp++ = ' ';
	*cp++ = '(';
	strcpy(cp, daystr[n]);
	cp += 3;
	*cp++ = ')';
	*cp++ = '\000';

	return buffer;
}

int year_of_jd(jd)
long int jd;
{
	int d, n;
	int leap;
	n = (jd - 1721425) / 365.2425;
	if (n >= 0)
		++n;
	while (1) {
		d = jd - julday(n, 0, 0);
		leap = leapyr(n);	/* true if leap year */
		if (d <= 365 + leap  &&  d >= 1)
			break;
		if (d < 1) {
			--n;
			if (n == 0)
				--n;	/* skip year 0 */
		} else
			++n;
	}
	return n;
}

int month_of_jd(jd)
long int jd;
{
	struct month *calp;		/* pointer into calendar */
	int d, m, n;
	int leap;		/* flag; 1 = leap yr */

	/* 1721425 = Julian Date @ 1 BC Dec 31 (Gregorian calendar), 12h UT.
	365.2425 = days per year (over the 400 yr Gregorian cycle). */
	n = (jd - 1721425) / 365.2425;
	if (n >= 0)
		++n;

	/*  n = estimated year.  Correct the estimate to actual year.  The
	preceding formula can mistakenly yield an AD year from a BC jd, but
	the reverse will never happen.  Therefore, we need only guard against
	n being DECREMENTED to 0 (which is an undefined year). */

	while (1) {
		d = jd - julday(n, 0, 0);
		leap = leapyr(n);	/* true if leap year */
		if (d <= 365 + leap  &&  d >= 1)
			break;
		if (d < 1) {
			--n;
			if (n == 0)
				--n;	/* skip year 0 */
		} else
			++n;
	}

	/* Determine month.  Begin by moving forward thru the calendar a
	month at a time (starting with Feb) till we overshoot */

	calp = calndr;		/* point to Jan */
	m = 0;			/* month counter; 0 = Jan */
	do {
		++calp;		/* point to next month in calendar */
		++m;		/* month counter */
		n = calp->difm;
		if (m >= 2 && leap)	/* March or later, & leap yr */
			++n;
	} while (d > n);

	/* We've overshot by one month.  Decrement calendar pointer "calp".
	Month counter "m" has overshot too, so the "m >= 3" test actually
	means "March or later".  After subtracting days in full months, d =
	day of month. */
	d -= (--calp)->difm + (m >= 3 && leap);
	return m;
}

int day_of_jd(jd)
long int jd;
{
	struct month *calp;		/* pointer into calendar */
	int d, m, n;
	int leap;		/* flag; 1 = leap yr */

	/* 1721425 = Julian Date @ 1 BC Dec 31 (Gregorian calendar), 12h UT.
	365.2425 = days per year (over the 400 yr Gregorian cycle). */
	n = (jd - 1721425) / 365.2425;
	if (n >= 0)
		++n;

	/*  n = estimated year.  Correct the estimate to actual year.  The
	preceding formula can mistakenly yield an AD year from a BC jd, but
	the reverse will never happen.  Therefore, we need only guard against
	n being DECREMENTED to 0 (which is an undefined year). */

	while (1) {
		d = jd - julday(n, 0, 0);
		leap = leapyr(n);	/* true if leap year */
		if (d <= 365 + leap  &&  d >= 1)
			break;
		if (d < 1) {
			--n;
			if (n == 0)
				--n;	/* skip year 0 */
		} else
			++n;
	}

	/* Determine month.  Begin by moving forward thru the calendar a
	month at a time (starting with Feb) till we overshoot */

	calp = calndr;		/* point to Jan */
	m = 0;			/* month counter; 0 = Jan */
	do {
		++calp;		/* point to next month in calendar */
		++m;		/* month counter */
		n = calp->difm;
		if (m >= 2 && leap)	/* March or later, & leap yr */
			++n;
	} while (d > n);

	/* We've overshot by one month.  Decrement calendar pointer "calp".
	Month counter "m" has overshot too, so the "m >= 3" test actually
	means "March or later".  After subtracting days in full months, d =
	day of month. */
	d -= (--calp)->difm + (m >= 3 && leap);
	return d;
}

long int
julday(int y, int m, int d)
/* Returns Julian Date (unit = days) corresponding to 12h UT on given date,
Gregorian calendar.  Jan = month 0.  Use -1 for 1 BC, etc.  Any integer
is legal for d. */
{
	int bc;		/* flag; 1 = BC */

	if (m > 1)	/* after Feb */
		d += leapyr(y);		/* add 1 if leap yr */

	if (y >= 0) {
		--y;
		bc = 0;
	} else {
		y = -(y + 1);
		bc = 1;
	}
	return ((bc ? 366 : 0) + y*365L + y/4 - y/100 + y/400) *
	  (bc ? -1 : 1) + calndr[m].difm + d + 1721425;
	/* 1721425 = Julian Date for 1 BC Dec 31 12h, Gregorian calendar */
}


STATIC int
leapyr(y)
int y;
/* Returns 1 if y is leap year in Gregorian calendar, 0 otherwise.  To
signify BC year use a negative number, e.g., -1 for 1 BC. */
{
	/* For leap year determination purposes, 1 BC = 0, 2 BC = -1, etc.
	Therefore, we must make an adjustment to y if it's BC. */
	if (y < 0)
		++y;
	return (!(y%4) && y%100)  ||  !(y%400);
}


char *
stoup(string)
char *string;
/* Converts "string" to all upper case, returns "string". */
{
	char *cp;
	int i;

	for (cp = string; (i = *cp); ++cp)
		if (isalpha(i))
			*cp = toupper(i);
	return string;
}


char *
timstr(m)
HTLdbl m;	/* minutes */
/* Turns m into a string of format "hhmm:ss" (or "-hhmm:ss" if m is
negative), rounded to nearest second.  Be sure that hours will not require
more than two digits.  String will be padded with zeros to fill out all six
digits.  Returns the string. */
{
	static char buf[9];
	char *cp, **cpp;
	int minus;		/* flag; 1 = negative */

	cp = buf;
	cpp = degdms(4, m / 60.);

	if (m < 0.) {
		*cp++ = '-';
		minus = 1;
	} else
		minus = 0;
	cpy0p(cp, cpp[0] + minus, 3);
	cpy0p(cp + 2, cpp[1], 3);
	cp[4] = ':';
	cpy0p(cp + 5, cpp[2], 3);

	return buf;
}
