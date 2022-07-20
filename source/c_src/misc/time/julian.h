/*
 * $Id: julian.h 47 2007-03-31 00:49:45Z gbc $
 */
/* Cribbed from seesat5 10/1/99 jpd */

#include "time_float_type.h"

#ifndef JULIAN_H
#define JULIAN_H

extern HTLdbl atomin();
	/* convert string to minutes */

extern char **degdms();
	/* convert HTLdbl to deg, minutes, seconds strings */

extern char *jdstr();
	/* converts jd to year, month, day in string form */

extern long int julday(int y, int m, int d);
	/* returns JD (unit = days) of given year, month, day @ 12h */

extern char *stoup();
	/* converts sting to all upper case */

extern char *timstr();
	/* converts minutes to string of "hhmm:ss" format */
	
int year_of_jd( long int jd);

#endif /* ndef JULIAN_H */

/*
 * eof
 */
