/*
 *  $Id: sc_time.c 80 2007-05-11 16:10:54Z gbc $
 *
 *  This file contains xptr system time functions.
 *
 *  Copyright (C) 1998 by MIT.			All rights reserved.
 */

#include <assert.h>
#include <sys/time.h>
#include <math.h>
#include "hetetimecode.h"
#include "time_float_type.h"

extern unsigned short Ht_hete_epoch;

/*
 *  The HETE-2 time primitive.  Time is computed using the UNIX
 *  gettimeofday(2) primitive and the UNIX epoch.
 */
#define TVP2USEC(t) ((HTLdbl)((t)->tv_sec)*(1E6)+(HTLdbl)((t)->tv_usec))
void
get_sc_time_array(IPP_word time[4])
{
	struct timeval	now;
	HTLdbl		us;

	/* only here if testing with HETE data */
	assert(!THIS_IS_IBEX);

	gettimeofday(&now, 0);
	us = TVP2USEC(&now);

	time[0] = fmod(( us                    ), 65536.0 );
	time[1] = fmod(( us / 65536.0          ), 65536.0 );
	time[2] = fmod(( us / 4294967296.0     ), 65536.0 );
	time[3] = fmod(( us / 281474976710656.0), 65536.0 );
	time[3] |= EPOCH_UNIX(0);
}

/*
 *  The HETE-1 interface.  Returns microseconds as an unsigned
 *  and it sets the time_code if the pointer is not NULL.
 */
unsigned int
get_sc_time(time_code *tc)
{
	time_code	t;

	/* only here if testing with HETE data */
	assert(!THIS_IS_IBEX);

	get_sc_time_array(t.time);
	if (tc) *tc = t;
	return( (t.time[0]) | (t.time[1]<<16) );
}

/*
 *  Returns 	-N	if time1 <  time2 (time1 is before time2)
 *		 0	if time1 == time2 (time1  equals   time2)
 *		+N	if time1 >  time2 (time1 is  after time2)
 *
 *  where the times are the same through word N.
 *  Note: ordering between EPOCHS isn't well defined.
 */
int
time_cmp(time_code *time1, time_code *time2)
{
	if (time1->time[3] >  time2->time[3]) return( 4);
	if (time1->time[3] <  time2->time[3]) return(-4);
	/* (time1->time[3] == time2->time[3]) */

	if (time1->time[2] >  time2->time[2]) return( 3);
	if (time1->time[2] <  time2->time[2]) return(-3);
	/* (time1->time[2] == time2->time[2]) */

	if (time1->time[1] >  time2->time[1]) return( 2);
	if (time1->time[1] <  time2->time[1]) return(-2);
	/* (time1->time[1] == time2->time[1]) */

	if (time1->time[0] >  time2->time[0]) return( 1);
	if (time1->time[0] <  time2->time[0]) return(-1);
	/* (time1->time[0] == time2->time[0]) */

	return(0);
}

/*
 *  Hand off to more general subroutine.
 */
int
has_this_time_passed(time_code *time)
{
	time_code	t;

	/* only here if testing with HETE data */
	assert(!THIS_IS_IBEX);

	get_sc_time_array(t.time);
	return( (time_cmp(time, &t) < 0) ? 1: 0);
}

/*
 * eof
 */
