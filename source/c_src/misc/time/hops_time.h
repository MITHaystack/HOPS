/*
 * $Id: hops_time.h 2911 2019-12-23 17:24:02Z gbc $
 *
 * Declarations for hops_time
 */

#ifndef hops_time_h
#define hops_time_h

#include <ctype.h>
#include <stdio.h>
#include "htime.h"

/* hops_time.c */
extern FILE	*ofp;
extern int	verb, echo, replace, statf, rtype, qtype, gibber;
extern char	*joiner, *query_arg, *report_arg;
extern HTLdbl	d_mult, d_orig;

/* ht_parser.c */
extern int dump_formats(char *oa);
extern int set_origin(char *oa);
extern int delta_setup(char *oa);
extern int input_setup(char *oa);
extern int query_setup(char *oa);

/* ht_utils.c */
extern int time_argument(char *oa);
extern int all_arguments(char *oa);

#endif /* hops_time_h */

/*
 *
 */
