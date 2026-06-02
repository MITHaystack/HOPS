/*
 * This file holds templates used to construct the
 * eventual gnuplot splot command; the idea is similar
 * to what is done in cohfit, but it is a simpler plot.
 * Each string defined has a BASE size which is updated
 * for the various %-items expanded.
 */
#if BIGGER
#ifndef __splotemp_h__
#define __splotemp_h__

/* pdfcairo terminal setup */
#define GNUPLOT_PDFILE "\
set term pdfcairo size %lf,%lf font '%s,%d'\n\
set output '%s'\n\
"
#define GNUPLOT_PDFILE_BASE 200

/* pre-command configuration */
#define GNUPLOT_CONFIG "\
#\n\
# %s\n\
#\n\
"
#define GNUPLOT_CONFIG_BASE 200

/* splot command */
#define GNUPLOT_SPLOT "\
splot '%s'\n\
"
#define GNUPLOT_SPLOT_BASE 200

/* so the human knows this is all there is */
#define GNUPLOT_CODA "\
set output\n\
#\n\
# %s\n\
#\n\
# eof\n\
#\n\
"
#define GNUPLOT_CODA_BASE 200

#endif /* __splotemp_h__ */
#endif /* BIGGER */
/*
 * end vim:nospell
 */
