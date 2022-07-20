/*
 *  $Id: hetetimecode.h 80 2007-05-11 16:10:54Z gbc $
 *
 *  HETE clock globals.
 *
 *  Copyright (C) 1998 by MIT.          All rights reserved.
 */

#ifndef	_HETETIMECODE_H
#define	_HETETIMECODE_H

typedef unsigned short IPP_word;

typedef struct
{
	IPP_word	time[4];	
}	time_code;

/* Words are in little-endian order: 
 *	time[0] is microseconds.
 *	time[1] units are about 66 ms.
 *	time[2] units are about 1.2 hours
 *	time[3] units are about 9 years.
 *
 * time[3] codes the "epoch" of the time. On the spacecraft, the epoch
 * is determined by the digital watch's jumpers. In other environments,
 * it depends on the clock source:
 */
 
#define EPOCH_NULL	0	/* no clock available */
#define EPOCH_UNSYNC	0x1	/* raw unsynchronized processor time */
#define EPOCH_HUBSYNC	0x2	/* HUB synchronized time, no watch */
#define EPOCH_WATCH(N)	(0x10|(N))	/* synchronized to watch 0<=N<=15 */
#define EPOCH_UNIX(N)	(0x20|(N))	/* Unix sys time, N is 4 MSB's */

/* note that HETE-II started in EPOCH_UNIX(3). EPOCH_UNIX(4) will begin
 * January 10, 2004 at 13:37:04 UT
 */

/*
 *  Epoch 003X will be used for IBEX.
 */
#define THIS_IS_IBEX	((Ht_hete_epoch & 0x30) == 0x30)
#define EPOCH_IBEX(N)	(0x30|(N))
 
/* The HETE-I interface: returns microseconds as an unsigned as well as the
 * code. The pointer may be null if you don't want the code.
 */
 
extern unsigned get_sc_time(time_code *tc);

/* A slightly lower level interface, for your convenience in constructing
 * messages as arrays:
 */
 
extern void get_sc_time_array(IPP_word time[4]);

/* Ordering functions */
extern int time_cmp(time_code *time1, time_code *time2);
extern int has_this_time_passed(time_code *time);

#endif /* _HETETIMECODE_H */

/*
 * eof
 */
