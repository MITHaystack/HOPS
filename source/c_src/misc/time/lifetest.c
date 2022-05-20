/*
 *  $Source: /usr/hete/repository/infra/ops/lifetest.c,v $
 *
 *  HETE time library test file
 *
 *  Copyright (C) 2001 by MIT.			All rights reserved.
 */

#include "htime.h"
#include <unistd.h>
#include <assert.h>
#include <math.h>

#define	ROE	1e-8

static int	verb = 0;

int
main(int argc, char **argv)
{
	HTime	ht;
	HTLdbl	error, gps, dt, go;
	char	*epoch = getenv("IBEX_TIME_EPOCH");
	int	i, ibex, errs = 0;

	if (argc > 1) verb++;

	/* set via TESTS_ENVIRONMENT from Makefile */
	if (getenv("srcdir")) epoch = "30";

	if (!epoch) {
	    epoch = getenv("HETE_TIME_EPOCH");
	    if (!epoch) {
		fprintf(stderr, "HETE_TIME_EPOCH must be defined\n");
		return(1);
	    }
	    ibex = 0;
	} else {
	    ibex = 1;
	}
	/*
	if (atoi(epoch) > 0) {
		fprintf(stderr, "Sorry, need to use file epoch here.\n");
		return(1);
	}
	*/

	Ht_initialize();
	if (ibex) assert( THIS_IS_IBEX);
	else      assert(!THIS_IS_IBEX);

	/*
	 * don't care about this part
	Ht_update_interval = 5;
	fprintf(stdout, "Type 6666666 %04x\n", Ht_string_type("6666666"));
	 */

	for (i = 0; i < 3; i++) {
	/*
		ht = Ht_GPS_to_HTC(Ht_lifetimes->start, &error);
		fprintf(stdout, "%f -> %04x %04x %04x %04x [Err = %.2e] -> ",
			Ht_lifetimes->start,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		fprintf(stdout, "%f [Err = %.2e]\n", gps, error);
	 */
#ifdef USE_LONG_DOUBLE
	    if (THIS_IS_IBEX) {
		dt = 65536.000;
		ht = Ht_GPS_to_HTC(go = Ht_lifetimes->start + dt*i, &error);
		if (verb) fprintf(stdout,
			"%14.3Lf -> %04x %04x %04x %04x [%.0Le] -> ",
			Ht_lifetimes->start + dt*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		if (fabs(go - gps) > ROE) errs++;
		if (verb) fprintf(stdout,
			"%14.3Lf [%.0Le]%c\n", gps, error, errs?'*':' ');

		dt = 65538.001;
		ht = Ht_GPS_to_HTC(go = Ht_lifetimes->start + dt*i, &error);
		if (verb) fprintf(stdout,
			"%14.3Lf -> %04x %04x %04x %04x [%.0Le] -> ",
			Ht_lifetimes->start + dt*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		if (fabs(go - gps) > ROE) errs++;
		if (verb) fprintf(stdout,
			"%14.3Lf [%.0Le]%c\n", gps, error, errs?'*':' ');

		dt = 65538.999;
		ht = Ht_GPS_to_HTC(go = Ht_lifetimes->start + dt*i, &error);
		if (verb) fprintf(stdout,
			"%14.3Lf -> %04x %04x %04x %04x [%.0Le] -> ",
			Ht_lifetimes->start + dt*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		if (fabs(go - gps) > ROE) errs++;
		if (verb) fprintf(stdout,
			"%14.3Lf [%.0Le]%c\n", gps, error, errs?'*':' ');

		dt = 65539.000;
		ht = Ht_GPS_to_HTC(go = Ht_lifetimes->start + dt*i, &error);
		if (verb) fprintf(stdout,
			"%14.3Lf -> %04x %04x %04x %04x [%.0Le] -> ",
			Ht_lifetimes->start + dt*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		if (fabs(go - gps) > ROE) errs++;
		if (verb) fprintf(stdout,
			"%14.3Lf [%.0Le]%c\n", gps, error, errs?'*':' ');

	    } else {
		ht = Ht_GPS_to_HTC(Ht_lifetimes->start + 16.777216*i, &error);
		fprintf(stdout, "%.6Lf -> %04x %04x %04x %04x [Err = %.2Le] -> ",
			Ht_lifetimes->start + 16.777216*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		fprintf(stdout, "%.6Lf [Err = %.2Le]\n", gps, error);
	    }
#else 
		if (THIS_IS_IBEX) {
		dt = 65536.000;
		ht = Ht_GPS_to_HTC(go = Ht_lifetimes->start + dt*i, &error);
		if (verb) fprintf(stdout,
			"%14.3f -> %04x %04x %04x %04x [%.0e] -> ",
			Ht_lifetimes->start + dt*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		if (fabs(go - gps) > ROE) errs++;
		if (verb) fprintf(stdout,
			"%14.3f [%.0e]%c\n", gps, error, errs?'*':' ');

		dt = 65538.001;
		ht = Ht_GPS_to_HTC(go = Ht_lifetimes->start + dt*i, &error);
		if (verb) fprintf(stdout,
			"%14.3f -> %04x %04x %04x %04x [%.0e] -> ",
			Ht_lifetimes->start + dt*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		if (fabs(go - gps) > ROE) errs++;
		if (verb) fprintf(stdout,
			"%14.3f [%.0e]%c\n", gps, error, errs?'*':' ');

		dt = 65538.999;
		ht = Ht_GPS_to_HTC(go = Ht_lifetimes->start + dt*i, &error);
		if (verb) fprintf(stdout,
			"%14.3f -> %04x %04x %04x %04x [%.0e] -> ",
			Ht_lifetimes->start + dt*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		if (fabs(go - gps) > ROE) errs++;
		if (verb) fprintf(stdout,
			"%14.3f [%.0e]%c\n", gps, error, errs?'*':' ');

		dt = 65539.000;
		ht = Ht_GPS_to_HTC(go = Ht_lifetimes->start + dt*i, &error);
		if (verb) fprintf(stdout,
			"%14.3f -> %04x %04x %04x %04x [%.0e] -> ",
			Ht_lifetimes->start + dt*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		if (fabs(go - gps) > ROE) errs++;
		if (verb) fprintf(stdout,
			"%14.3f [%.0e]%c\n", gps, error, errs?'*':' ');

		} else {
		ht = Ht_GPS_to_HTC(Ht_lifetimes->start + 16.777216*i, &error);
		fprintf(stdout, "%.6f -> %04x %04x %04x %04x [Err = %.2e] -> ",
			Ht_lifetimes->start + 16.777216*i,
			ht.time[3], ht.time[2], ht.time[1], ht.time[0], error);
		gps = Ht_HTC_to_GPS(ht, &error);
		fprintf(stdout, "%.6f [Err = %.2e]\n", gps, error);
		}
#endif

		sleep(0/*0*/);
		/* Ht_reset(); */
	}

	Ht_dump_lives("Debug.txt");
	return(0);
}

/*
 * $Log: lifetest.c,v $
 * Revision 1.1  2001/01/11 21:26:41  gbc
 * recreated the testing program.
 *
 */
