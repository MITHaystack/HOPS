/*
 * $Id: julian.h 47 2007-03-31 00:49:45Z gbc $
 */
/* Cribbed from seesat5 10/1/99 jpd */

#include "time_float_type.h"

#ifndef JULIAN_H
#define JULIAN_H

extern HTLdbl atomin(char *string);
	/* convert string to minutes */

extern char **degdms(int pre, HTLdbl x);
	/* convert HTLdbl to deg, minutes, seconds strings */

extern char *jdstr(long int jd);
	/* converts jd to year, month, day in string form */

extern long int julday(int y, int m, int d);
	/* returns JD (unit = days) of given year, month, day @ 12h */

extern char *stoup(char *string);
	/* converts sting to all upper case */

extern char *timstr(HTLdbl m);
	/* converts minutes to string of "hhmm:ss" format */
	
extern int year_of_jd(long int jd);

#endif /* ndef JULIAN_H */

/*
 * eof
 */
