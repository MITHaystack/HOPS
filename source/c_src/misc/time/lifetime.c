/*
 *  $Source: /usr/hete/repository/infra/ops/lifetime.c,v $
 *
 *  HETE time library lifetime module
 *
 *  Copyright (C) 1999 by MIT.			All rights reserved.
 */

#include <assert.h>

#include "htime.h"
#include "isoclog.h"
#include <sys/stat.h>
#include <string.h>

/* linked list of s/c lifetimes */
HLife		*Ht_lifetimes = (HLife *)0;
int		Ht_update_interval = 0;

/* the deduced s/c epoch */
IPP_word	Ht_hete_epoch = EPOCH_NULL;
int		Ht_tz_unset = 1;

/* an origin for relative time calculations */
GTime		Ht_gps_origin = 0;
int		Ht_string_min_len = 0;
 
int		Ht_suppress_Z = 0;

int		Ht_asserted_string_type = Ht_STR_NTYP;

HTLdbl		Ht_d_mult = 1.0, Ht_d_orig = 0.0;

/* adjustments to the dw made at boot */
char		*Ht_life_file;
int		Ht_dwab_dwdelta = 0;
HTLdbl		Ht_dwab_dw_mult = 1.0;
static char	dwd_comment[80] = "", dwm_comment[80] = "";
static char	gps_dwx[1024];
static int	last_update = 0;

/*
 *  This is a pointer to the last HLife used so that consecutive
 *  time lookups are reasonably efficient.  It will eventually be
 *  periodically reset to NULL to force a reload from the database.
 */
HLife		*Ht_last_life = (HLife *)0;

/*
 *  These are pretty raw calibration numbers whose only purpose is to
 *  allow the program to continue if a real calibration can't be found.
 */
static HLife	default_life[16] = {
#if HETE_PECULIAR
	/* watch none    0x0 1999/01/01 00:00:00 UTC */
	{ 599184013.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x0)}},
	  0.3000,1.00010000,1.00e-6, /* uncalibrated */
	  0,0, "DW-0 Reset: Jan  1 00:00:00 1999 UTC" } ,
#else /* HETE_PECULIAR */
	/* watch none    0x0 1980/06/01 00:00:00 UTC */
	{ 0.0, 0.0, {{0, 0, 0, EPOCH_IBEX(0x0)}},
	  0.0000,1.00000000,1.00e-12, /* GPS Based */
	  0,0, "DW-0 Reset: Jan  6 00:00:00 1980 UTC" } ,
#endif /* HETE_PECULIAR */
	/* watch geoff   0x1 1998/06/01 18:18:40 UTC */
	{ 580760332.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x1)}},
	  0.8000,1.00004526,1.00e-6, /* approximate */
	  0,0, "DW-1 Reset: Jun  1 18:18:40 1998 UTC",0 } ,
	/* watch jeanluc 0x2 1999/01/20 18:24:00 UTC */
	{ 600891853.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x2)}},
	  0.0215,1.00003254,6.53e-07 /* 0x2 : 1.04857600 / 1.04854188 */,
	  0,0, "DW-2 Reset: Jan 20 18:24:00 1999 UTC",0 } ,
	/* watch rosalia 0x3 1999/01/20 16:25:15 UTC */
	{ 600884728.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x3)}},
	  0.3000,1.00010000,1.00e-6, /* uncalibrated */
	  0,0, "DW-3 Reset: Jan 20 16:25:15 1999 UTC",0 } ,
	/* watch mike    0x4 1999/01/20 20:46:30 UTC */
	{ 600900403.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x4)}},
	  0.0216,1.00009136,5.95e-07 /* 0x4 : 1.04857600 / 1.04848022 */,
	  0,0, "DW-4 Reset: Jan 20 20:46:30 1999 UTC",0 } ,
	/* watch roland  0x5 1999/01/14 19:31:10 UTC */
	{ 600377483.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x5)}},
	  0.3000,1.00010000,1.00e-6, /* uncalibrated */
	  0,0, "DW-5 Reset: Jan 14 19:31:10 1999 UTC",0 } ,
	/* watch arnaud  0x6 1999/01/20 18:52:40 UTC */
	{ 600893573.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x6)}},
	  0.0215,1.00003509,1.12e-07 /* 0x6 : 1.04857600 / 1.04853920 */,
	  0,0, "DW-6 Reset: Jan 20 18:52:40 1999 UTC",0 } ,
	/* watch greg    0x7 1999/01/21 18:23:15 UTC */
	{ 600978208.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x7)}},
	  0.0215,1.00004223,4.64e-07 /* 0x7 : 1.04857600 / 1.04853172 */,
	  0,0, "DW-7 Reset: Jan 21 18:23:15 1999 UTC",0 } ,
	/* watch none    0x8 1999/01/01 00:00:00 UTC */
	{ 599184013.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x8)}},
	  0.3000,1.00010000,1.00e-6, /* uncalibrated */
	  0,0, "DW-8 Reset: Jan  1 00:00:00 1999 UTC",0 } ,
	/* watch bob     0x9 1999/01/26 21:06:15 UTC */
	{ 601419988.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0x9)}},
	  0.0210,1.00004424,6.18e-07 /* 0x9 : 1.04857600 / 1.04852962 */,
	  0,0, "DW-9 Reset: Jan 26 21:06:15 1999 UTC",0 } ,
	/* watch fred    0xa 1999/02/23 15:15:10 UTC */
	{ 603818123.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0xa)}},
	  0.0214,1.00017375,5.51e-07 /* 0xa : 1.04857600 / 1.04839384 */,
	  0,0, "DW-A Reset: Feb 23 15:15:10 1999 UTC",0 } ,
	/* watch none    0xb 1999/01/01 00:00:00 UTC */
	{ 599184013.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0xb)}},
	  0.3000,1.00010000,1.00e-6, /* uncalibrated */
	  0,0, "DW-B Reset: Jan  1 00:00:00 1999 UTC",0 } ,
	/* watch none    0xc 1999/01/01 00:00:00 UTC */
	{ 599184013.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0xc)}},
	  0.3000,1.00010000,1.00e-6, /* uncalibrated */
	  0,0, "DW-C Reset: Jan  1 00:00:00 1999 UTC",0 } ,
	/* watch none    0xd 1999/01/01 00:00:00 UTC */
	{ 599184013.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0xd)}},
	  0.3000,1.00010000,1.00e-6, /* uncalibrated */
	  0,0, "DW-D Reset: Jan  1 00:00:00 1999 UTC",0 } ,
	/* watch none    0xe 1999/01/01 00:00:00 UTC */
	{ 599184013.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0xe)}},
	  0.3000,1.00010000,1.00e-6, /* uncalibrated */
	  0,0, "DW-E Reset: Jan  1 00:00:00 1999 UTC",0 } ,
	/* watch none    0xf 1999/01/01 00:00:00 UTC */
	{ 599184013.0, 0.0, {{0, 0, 0, EPOCH_WATCH(0xf)}},
	  0.3000,1.00010000,1.00e-6, /* uncalibrated */
	  0,0, "DW-F Reset: Jan  1 00:00:00 1999 UTC",0 }
};

/*
 *  Little scrap to grab the comment at the end
 *  of the line and copy it to the HLife record.
 */
static void
load_comment(HLife *l, char *entry)
{
	while (*entry != '#' && *entry != '\n') entry++;
	if (*entry == '#')
		strncpy(l->comment, entry+2, sizeof(l->comment));
	else
		strcpy(l->comment, "No Comment");
	for (entry = l->comment; *entry && *entry != '\n'; entry++);
	if (*entry == '\n') *entry = '\0';
	l->old = l->last = l->next = 0;
}

/*
 *  Scan all the number in, with useful defaults
 */
static void
scan_numbers(HLife **pl, char *entry)
{
	HLife	*l = *pl;

#ifdef USE_LONG_DOUBLE
        switch ( sscanf(entry, "%Lf %Lf %hx %hx %hx %hx %Lf %Lf %Lf",
                &(l->start), &(l->span),
                &(l->birth.time[3]), &(l->birth.time[2]),
                &(l->birth.time[1]), &(l->birth.time[0]),
                &(l->error), &(l->dsc_dt), &(l->esc_dt)
        )) {
        case 2: l->birth.time[3] = 0x0010;
        case 3: l->birth.time[2] = 0x0000;
        case 4: l->birth.time[1] = 0x8000;
        case 5: l->birth.time[0] = 0x8000;
        case 6: l->error = 10.0;
        case 7: l->dsc_dt = 1.0;
        case 8: l->esc_dt = 1.0;
        case 9: break;
        default:
                free(l);
                *pl = 0;
                break;
        }
#else
	switch ( sscanf(entry, "%lf %lf %hx %hx %hx %hx %lf %lf %lf",
		&(l->start), &(l->span),
		&(l->birth.time[3]), &(l->birth.time[2]),
		&(l->birth.time[1]), &(l->birth.time[0]),
		&(l->error), &(l->dsc_dt), &(l->esc_dt)
	)) {
	case 2: l->birth.time[3] = 0x0010;
	case 3: l->birth.time[2] = 0x0000;
	case 4: l->birth.time[1] = 0x8000;
	case 5: l->birth.time[0] = 0x8000;
	case 6: l->error = 10.0;
	case 7: l->dsc_dt = 1.0;
	case 8: l->esc_dt = 1.0;
	case 9: break;
	default:
		free(l);
		*pl = 0;
		break;
	}
#endif
}

/*
 *  Utility to load a lifetime calibration entry.
 */
static HLife *
load_lifetime(FILE *s)
{
	static char	buf[1024];
	char		*entry;
	HLife		*l = (HLife *)malloc(sizeof(HLife));
	HLife		*o = Ht_last_life;
	int		dwd;
	HTLdbl		dwm;

	if (!l) return(0);		/* out of memory */

	/* find a non-comment/non-blank entry */
	do {
		if ( !fgets(entry = buf, sizeof(buf), s) ) {
			free(l);	/* end of file */
			return(0);
		}
		if (1 == sscanf(buf, "# DWAB dwdelta = %d", &dwd)) {
			/* load most recent dwdelta eeprom parameter */
			Ht_dwab_dwdelta = dwd;
			strncpy(dwd_comment, buf, sizeof(dwd_comment));
		}
		if (1 == sscanf(buf, "# DWAB dw_mult = %lf", &dwm)) {
			/* load most recent dw_mult eeprom parameter */
			Ht_dwab_dw_mult = dwm;
			strncpy(dwm_comment, buf, sizeof(dwm_comment));
		}
		if (buf[0] == '#' && o) {
			scan_numbers(&l, entry+1);
			if (l) {
				/* it is a valid old record */
				load_comment(l, entry+1);
				o->old = l;
				o = o->old;
			}
			l = (HLife *)malloc(sizeof(HLife));
		}
	} while (buf[0] == '#' || buf[0] == '\n');

	/* scan in the line, using defaults for least interesting stuff */
	/* l will be zeroed if the line has too muchor too little on it */
	scan_numbers(&l, entry);

	/* finish it off */
	if (l) load_comment(l, entry);

	return(l);
}

/*
 *  Utility to dump a lifetime calibration entry.
 */
static void
dump_lifetime(FILE *s, HLife *l, char *prefix)
{
#ifdef USE_LONG_DOUBLE
	fprintf(s,
		"%s%16.6Lf %10.1Lf "
		"%04x %04x %04x %04x "
		"%12.8Lf %.10Lf %.4Le # %s\n",
		prefix,
		l->start, l->span,
		l->birth.time[3], l->birth.time[2],
		l->birth.time[1], l->birth.time[0],
		l->error, l->dsc_dt, l->esc_dt, l->comment
	);
#else 
    fprintf(s,
        "%s%16.6f %10.1f "
        "%04x %04x %04x %04x "
        "%12.8f %.10f %.4e # %s\n",
        prefix,
        l->start, l->span,
        l->birth.time[3], l->birth.time[2],
        l->birth.time[1], l->birth.time[0],
        l->error, l->dsc_dt, l->esc_dt, l->comment
    );
#endif
	/* and flush out subsumed lives */
	if (*prefix == ' ') {
		l = l->old;
		while (l) {
			dump_lifetime(s, l, "#");
			l = l->old;
		}
	}
}

/*
 *  Utility to dump lifetime list
 */
void
Ht_dump_lives(char *file)
{
	FILE	*fs;
	HLife	*entry = Ht_lifetimes;
	UTime	now;

	if (!(fs = fopen(file, "w"))) return;	/* no dice */

	gettimeofday(&now, 0);
	fprintf(fs, "#\n"
		    "# Lifetime for epoch %04x made %s"
		    "#\n",
		Ht_hete_epoch, asctime(gmtime(&now.tv_sec)));

	fprintf(fs, "# dwab dwdelta = %d\n", Ht_dwab_dwdelta);
	if (dwd_comment[0]) fputs(dwd_comment, fs);	/* this one counts */
#ifdef USE_LONG_DOUBLE
	fprintf(fs, "# dwab dw_mult = %.16Lf\n", Ht_dwab_dw_mult);
#else 
	fprintf(fs, "# dwab dw_mult = %.16f\n", Ht_dwab_dw_mult);
#endif
	if (dwm_comment[0]) fputs(dwm_comment, fs);	/* this one counts */
	fputs("#\n", fs);

	while (entry) {
		dump_lifetime(fs, entry, " ");
		entry = entry->next;
	}

	fclose(fs);
}

/*
 *  Initialize the internal information to the extent possible.
 *
 *  This version relies on environment variables (e.g.)
 *	HETE_OPS=/usr/hete		the hete ops directory
 *	IBEX_OPS=(wherever)		the ibex ops directory
 *	IBEX_TIME_EPOCH=3[01]		the ibex epoch, or
 *			filename	to load for lifetimes.
 *	HETE_TIME_EPOCH=1[0-9A-Fa-f]	the hete epoch, or
 *			filename	to load for lifetimes.
 *
 *  Preference is given to IBEX.
 */
static void
load_lives(void)
{
	char	*ops = getenv("IBEX_OPS"),
		*epoch = getenv("IBEX_TIME_EPOCH"),
		*caldir = getenv("IBEX_DATACALSC");
	FILE	*htl_file;
	HLife	*new = 0;
	int	dwe = 0, ibex;

	/* be insistent */
	if (Ht_tz_unset) {
		putenv("TZ=GMT");
		Ht_tz_unset = 0;
	}

	if (!epoch) {
		epoch = getenv("HETE_TIME_EPOCH");
		if (!epoch) epoch = "30";	/* IBEX default */
	}
	if (!ops) {
		ops = getenv("HETE_OPS");
	}

	if (!epoch || !ops) {
		/*
		fprintf(stderr, "Undefined IBEX_OPS or IBEX_TIME_EPOCH\n");
		 */
		errlog("Undefined IBEX_OPS or IBEX_TIME_EPOCH");
	} else {
		ibex = (*epoch == '3');
		if (*epoch == '1' || *epoch == '3') {
			epoch ++;
			if      (*epoch >= '0' && *epoch <= '9')
				dwe = *epoch - '0';
			else if (*epoch >= 'A' && *epoch <= 'F')
				dwe = *epoch - 'A' + 10;
			else if (*epoch >= 'a' && *epoch <= 'f')
				dwe = *epoch - 'a' + 10;
			else
				dwe = 0;
			if (!caldir) caldir = Ht_DATACALSC;
			sprintf(gps_dwx,
				ibex?Ht_IBEXTIMES:Ht_LIFETIMES,
				ops, caldir, dwe);
			/*
			sprintf(gps_dwx, Ht_LIFETIMES, ops, caldir, dwe);
			fprintf(stderr, "%s\n", gps_dwx);
			*/
			++epoch;
			if (*epoch >= 'A' && *epoch <= 'Z') {
				/* special calibration cases */
				gps_dwx[strlen(gps_dwx)-6] = *epoch;
				gps_dwx[strlen(gps_dwx)-7] = 'D';
			}
		} else {
			/* --epoch; */
			//strncpy(gps_dwx, epoch, strlen(epoch) + 1);
			strncpy(gps_dwx, epoch, sizeof(gps_dwx));
                        gps_dwx[sizeof(gps_dwx)-1] = 0;     /*force term*/
		}
	}

	if ((htl_file = fopen(gps_dwx, "r"))) {
		Ht_life_file = gps_dwx;		/* make name available */
		while ((new = load_lifetime(htl_file))) {
			if (!Ht_lifetimes) Ht_lifetimes = new;
			if (Ht_last_life) {
				Ht_last_life->next = new;
				new->last = Ht_last_life;
			}
			Ht_last_life = new;
		}
		fclose(htl_file);
	} else {
		Ht_life_file = 0;
		/*
		errlog("Using default DW calibration");
		fprintf(stderr, "Using default DW calibration\n");
		 */
	}

	if (!Ht_lifetimes)
		Ht_last_life = Ht_lifetimes = &default_life[dwe];

	Ht_hete_epoch = Ht_lifetimes->birth.time[3];
}

/*
 *  Flush out the old, then bring in the new...
 */
static void
reload_lives(void)
{
	HLife	*old, *next, *sub;

	old = Ht_lifetimes;
	while (old) {
		next = old->next;
		sub = old->old;
		free(old);
		while (sub) {
			old = sub;
			free(sub);
			sub = old->old;
		}
		old = next;
	}
	Ht_lifetimes = 0;
	Ht_life_file = 0;
	Ht_last_life = 0;
	Ht_dwab_dwdelta = 0;
	Ht_dwab_dw_mult = 1.0;
	Ht_hete_epoch = EPOCH_NULL;

	load_lives();
}

/*
 *  Simple utility to see if file changed since we last looked.
 */
static int
file_was_updated(void)
{
	struct stat	st;
	int		err = stat(gps_dwx, &st);
	if (err) {
		errlog("file_was_updated: Unable to stat file: %m");
		return(0);
	}
	return(((st.st_mtime - last_update) > 0) ? 1 : 0);
}

/*
 *  Check whether we need to initialize or reload lives.
 */
static void
reload_check(void)
{
	/* initialization case */
	if (!Ht_last_life) {
		load_lives();
		last_update = time(0);
		return;
	}

	/* bail if no reason for update */
	if (Ht_update_interval == 0 ||
	    (time(0) - last_update) < Ht_update_interval) return;

	/* update if file was modified */
	if (file_was_updated()) reload_lives();
	last_update = time(0);
}

/*
 *  Rather than call load_lives(), just call reload_check()
 *  which only calls load_lives() if it really makes sense.
 */
void
Ht_initialize(void)
{
	reload_check();
}

/*
 *  Force a reload.
 */
void
Ht_reset(void)
{
	reload_lives();
}

/*
 *  Figure out which life to lead based on hete clock value.
 */
static HLife *
get_ht_life(HTime ht)
{
	reload_check();

	/* find including lifetime */
	while (time_cmp(&Ht_last_life->birth, &ht) < 0 && Ht_last_life->next)
		Ht_last_life = Ht_last_life->next;

	while (time_cmp(&Ht_last_life->birth, &ht) > 0 && Ht_last_life->last) 
		Ht_last_life = Ht_last_life->last;

	return(Ht_last_life);
}

/*
 *  Figure out which life to lead based on GPS time.
 */
static HLife *
get_gt_life(GTime gt)
{
	reload_check();

	/* find including lifetime */
	while (Ht_last_life->start < gt && Ht_last_life->next)
		Ht_last_life = Ht_last_life->next;

	while (Ht_last_life->start > gt && Ht_last_life->last)
		Ht_last_life = Ht_last_life->last;

	return(Ht_last_life);
}

/*
 *  Turn HTime into seconds
 */
HTLdbl
Ht_HTC_to_secs(HTime ht)
{
	HTLdbl	x;
    if (THIS_IS_IBEX) {
	x  = scalbn((HTLdbl)(ht.time[0]),  0);
	x += scalbn((HTLdbl)(ht.time[1]),  0) * 1000.0;
	x += scalbn((HTLdbl)(ht.time[2]), 16) * 1000.0;

	x *= 1.0e-3;	/* msecs -> secs */
    } else {
	x  = scalbn((HTLdbl)(ht.time[0]),  0);
	x += scalbn((HTLdbl)(ht.time[1]), 16);
	x += scalbn((HTLdbl)(ht.time[2]), 32);
	x += scalbn((HTLdbl)(ht.time[3]), 48);

	x *= 1.0e-6;	/* usecs -> secs */
    }
	return(x);
}

/*
 *  Turn seconds into HTime
 */
HTime
Ht_secs_to_HTC(HTLdbl s)
{
	static HTime	ht;
    if (THIS_IS_IBEX) {
	s *= 1.0e+3;	/* secs -> msecs */
	s += 0.5;	/* we're about to truncate: */

	ht.time[0] = fmod(( s                    ),  1000.0 );
	ht.time[1] = fmod(( s / 1000.0           ), 65536.0 );
	ht.time[2] = fmod(( s / 65536000.0       ), 65536.0 );
	ht.time[3] = EPOCH_IBEX(0);

    } else {
	s *= 1.0e+6;	/* secs -> usecs */
	s += 0.5;	/* we're about to truncate: */

	ht.time[0] = fmod(( s                    ), 65536.0 );
	ht.time[1] = fmod(( s / 65536.0          ), 65536.0 );
	ht.time[2] = fmod(( s / 4294967296.0     ), 65536.0 );
	ht.time[3] = fmod(( s / 281474976710656.0), 65536.0 );
    }
	return(ht);
}

#if HETE_PECULIAR
/*
 *  These are convenient variants.  They differ from the above
 *  in that the epoch is implicit in the calculation.
 */
HTLdbl
Ht_HTC_to_MET(HTime ht)
{
	HTLdbl	x;

	x  = scalbn((HTLdbl)(ht.time[0]),  0);
	x += scalbn((HTLdbl)(ht.time[1]), 16);
	x += scalbn((HTLdbl)(ht.time[2]), 32);

	x *= 1.0e-6;    /* usecs -> secs */

	return(x);
}
HTime
Ht_MET_to_HTC(HTLdbl s)
{
	static HTime	ht;

	s *= 1.0e+6;	/* secs -> usecs */
	s += 0.5;	/* we're about to truncate: */

	ht.time[0] = fmod(( s                    ), 65536.0 );
	ht.time[1] = fmod(( s / 65536.0          ), 65536.0 );
	ht.time[2] = fmod(( s / 4294967296.0     ), 65536.0 );

	/* just in case we're not initialized */
	if (Ht_hete_epoch == EPOCH_NULL) reload_check();
	ht.time[3] = Ht_hete_epoch;

	return(ht);
}
#endif /* HETE_PECULIAR */

/*
 *  The primary time conversion utilities
 */
GTime
Ht_HTC_to_GPS(HTime ht, HtErr *err)
{
	GTime	gps;
	HLife	*hl = get_ht_life(ht);
	HTLdbl	s = Ht_HTC_to_secs(hl->birth), t = Ht_HTC_to_secs(ht);

	if (!err) err = &Ht_default_error;

	t -= s;			/* elapsed HTC seconds */
	gps = hl->start + t / hl->dsc_dt;

	*err = hl->error;
	*err += ((t > 0) ? t : -t) * hl->esc_dt;

	return(gps);
}

HTime
Ht_GPS_to_HTC(GTime gt, HtErr *err)
{
	HTime	htc;
	HLife	*hl = get_gt_life(gt);
	HTLdbl	s = Ht_HTC_to_secs(hl->birth), t;

	if (!err) err = &Ht_default_error;

	gt -= hl->start;	/* elapsed GPS seconds */
	t = s + gt * hl->dsc_dt;

	*err = hl->error;

	if (t < 0) {	/* times before reset are undefined */
		*err -= t;	/* 100 % uncertainty :-) */
		t = 0;
	}

	htc = Ht_secs_to_HTC(t);
	*err += ((gt > 0) ? gt : -gt ) * hl->esc_dt;

	return(htc);
}

/*
 * $Log: lifetime.c,v $
 * Revision 1.11  2001/02/20 19:43:42  gbc
 * added Ht_HTC_to_MET() and Ht_MET_to_HTC()
 *
 * Revision 1.10  2001/02/01 23:14:48  gbc
 * Fixed loading bug for non-compliant comments.
 * Added extra character handling for non-standard epochs.
 *
 * Revision 1.9  2001/01/29 15:53:00  gbc
 * Fixed Ht_initialize() to do a reload_check() rather than load_lives().
 *
 * Revision 1.8  2001/01/11 21:30:10  gbc
 * Cleaned up code to eliminate warning messages.
 * Revised lifetime load and dump methods to handle lifetimes relegated
 * to comment status.  Removed TEST_LIFETIME main() routine -> lifetest.c
 *
 * Revision 1.7  2001/01/11 02:50:58  gbc
 * modifications for monitoring lifetimes
 *
 * Revision 1.6  1999/09/24 15:26:25  gbc
 * Added Ht_DS_I_TS and Ht_initialize().
 *
 * Revision 1.5  1999/06/07 19:00:16  gbc
 * Added externals Ht_life_file and Ht_last_life for euridice.
 *
 * Revision 1.4  1999/06/07 12:07:35  gbc
 * Hooked up Ht_dwab_dwdelta & Ht_dwab_dw_mult; fixed commentary on load.
 *
 * Revision 1.3  1999/06/01 11:56:44  gbc
 * The '#' and the following space isn't part of the comment.
 *
 * Revision 1.2  1999/05/27 15:04:15  gbc
 * Cleaned up lifetime.c -- accepts a test epoch file, and
 * it now scans in the calibration database files correctly.
 *
 * Revision 1.1  1999/05/27 01:37:53  gbc
 * Broke the lifetime management stuff off to a separate file.
 *
 */
